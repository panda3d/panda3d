// Filename: mesherTempl.I
// Created by:  drose (15Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "builderMisc.h"
#include "mesherStrip.h"
#include "mesherFanMaker.h"
#include "config_builder.h"

#include <algorithm>

template <class PrimType>
MesherTempl<PrimType>::
MesherTempl(BuilderBucket *bucket) {
  _bucket = bucket;
  _stripIndex = 0;
  _next_strip = _done.end();
}

template <class PrimType>
int MesherTempl<PrimType>::
add_prim(const Prim &prim, MesherStripOrigin origin) {
  if (!prim.is_valid()) {
    return false;
  }

  // Define an initial strip (probably of length 1) for the prim.
  Strip temp_strip(prim, _stripIndex++, *_bucket);
  Strips &list = choose_strip_list(temp_strip);
  list.push_back(temp_strip);
  Strip &strip = list.back();
  strip._origin = origin;

  int i;
  int num_verts = prim.get_num_verts();

  const Vertex **vptrs = (const Vertex **)alloca(num_verts * sizeof(Vertex *));
  EdgePtrs **eptrs = (EdgePtrs **)alloca(num_verts * sizeof(EdgePtrs *));

  // Get the common vertex pointers for the primitive's vertices.
  for (i = 0; i < num_verts; i++) {
    TYPENAME Verts::iterator n =
      _verts.insert(Verts::value_type(prim.get_vertex(i), EdgePtrs())).first;
    vptrs[i] = &(*n).first;
    eptrs[i] = &(*n).second;

    strip._verts.push_back(vptrs[i]);
  }

  // Now identify the common edges.
  if (prim.get_type() == BPT_tri || prim.get_type() == BPT_quad) {
    // Polygons of arbitrary size don't get meshed, and so therefore
    // don't have any edges in common with anything else.  Only tris
    // and quads can be meshed.  (The builder normally breaks up
    // larger polygons into tris, though the user can choose to defeat
    // this.)

    for (i = 0; i < num_verts; i++) {
      // Define an inner and outer edge.  A polygon shares an edge with a
      // neighbor only when one of its inner edges matches a neighbor's
      // outer edge (and vice-versa).
      Edge inner(vptrs[i], vptrs[(i+1) % num_verts]);
      Edge outer(vptrs[(i+1) % num_verts], vptrs[i]);

      // Add it to the list and get its common pointer.
      Edge &inner_ref = (Edge &)*_edges.insert(inner).first;
      Edge &outer_ref = (Edge &)*_edges.insert(outer).first;

      // Tell the edges about each other.
      inner_ref._opposite = &outer_ref;
      outer_ref._opposite = &inner_ref;

      // Associate the common edge to the strip.
      strip._edges.push_back(&inner_ref);

      // Associate the strip, as well as the original prim, to the edge.
      outer_ref._strips.push_back(&strip);

      // Associate the common edge with the vertices that share it.
      //      Edge *edge_ptr = inner_ref.common_ptr();
      eptrs[i]->insert(&outer_ref);
      eptrs[(i+1) % num_verts]->insert(&outer_ref);
    }
  }
  return true;
}


template <class PrimType>
void MesherTempl<PrimType>::
mesh() {
  if (_bucket->_consider_fans) {
    find_fans();
  }

  // First, we try to make all the best quads we can.
  if (_bucket->_retesselate_coplanar) {
    make_quads();
  }

  // Then, we do the rest of the tris.
  meshList(_tris);

  if (_bucket->_show_quads) {
    // If we're showing quads, we shouldn't do any more meshing.
    TYPENAME Strips::iterator si;
    for (si = _quads.begin(); si != _quads.end(); ++si) {
      if ((*si)._status == MS_alive) {
        (*si)._status = MS_done;
      }
    }
    for (si = _strips.begin(); si != _strips.end(); ++si) {
      if ((*si)._status == MS_alive) {
        (*si)._status = MS_done;
      }
    }
  }

  // Then, build quads into sheets where possible.
  build_sheets();

  // Pick up any quads that might have been left behind.
  meshList(_quads);

  // Finally, do the longer strips.
  meshList(_strips);

  // Get ready to walk through the results.
  _next_strip = _done.begin();
}


template <class PrimType>
PrimType MesherTempl<PrimType>::
getPrim() {
  if (_next_strip == _done.end()) {
    // End of the list, return a primitive with no vertices.
    finalize();
    return Prim();
  }

  Strip &strip = (*_next_strip++);
  BuilderPrimType orig_type = strip._type;
  Prim prim = strip.make_prim(*_bucket);

  if (_bucket->_show_tstrips) {
    // If we have _show_tstrips enabled, it means we need to color
    // every primitive according to which, if any, tristrip it is in.

    // We use the _colors array--and later make a copy in shared
    // memory for the bucket to point to--in case the primitives are
    // indexed.  If the primitives are nonindexed, we'll still
    // allocate the _colors array and its copy in shared memory, but
    // it will be deleted when the bucket destructs.

    if (_colors.empty()) {
      // We need one entry for not-a-tristrip, indicated by white.
      _colors.push_back(Colorf(0.85, 0.85, 0.85, 1.0));
    }

    ushort i1, i2;
    Colorf color1, color2;;

    switch (prim.get_type()) {
    case BPT_tristrip:
    case BPT_trifan:
      make_random_color(color2);
      color1 = (color2 * 0.8);   // somewhat darker.
      i1 = _colors.size();
      i2 = i1+1;
      _colors.push_back(color1);
      _colors.push_back(color2);
      break;

    default:
      // not-a-tristrip.
      i1 = i2 = 0;
    }

    // Now i1 and i2 index into the colors array to indicate the color
    // for the first triangle and the rest of the primitive,
    // respectively.
    int num_components = prim.get_num_components();
    if (num_components > 0) {
      prim.get_component(0).set_color_value(&_colors[0], i1);
      for (int i = 1; i < num_components; i++) {
        prim.get_component(i).set_color_value(&_colors[0], i2);
      }
    } else {
      prim.set_color_value(&_colors[0], i1);
    }

  } else if (_bucket->_show_qsheets) {
    // _show_qsheets means to color every primitive according to
    // which, if any, quadsheet it is in.  This is a bit easier,
    // because the entire primitive gets the same color.

    if (_colors.empty()) {
      // We need one entry for not-a-qsheet, indicated by white.
      _colors.push_back(Colorf(0.85, 0.85, 0.85, 1.0));
    }

    // Is this a quadsheet?
    ushort i1 = 0;
    if (strip._row_id < 0) {
      // Yep!  Assign a new color, if it doesn't already have one.
      ColorSheetMap::iterator ci = _color_sheets.find(strip._row_id);

      if (ci == _color_sheets.end()) {
        Colorf color1;
        make_random_color(color1);
        i1 = _colors.size();
        _colors.push_back(color1);
        _color_sheets[strip._row_id] = i1;
      } else {
        i1 = (*ci).second;
      }
    }

    // Now i1 is the color we want to assign to the whole primitive.
    // Just set all vertices to the same color.
    int num_verts = prim.get_num_verts();
    for (int i = 0; i < num_verts; i++) {
      prim.get_vertex(i).set_color_value(&_colors[0], i1);
    }

  } else if (_bucket->_show_quads) {
    // _show_quads means to show the assembling of tris into quads and fans.

    // We use the following color convention:

    // white: unchanged; as supplied by user.
    // dark blue: quads made in the initial pass.  These are more certain.
    // light blue: quads made in the second pass.  These are less certain.
    // very light blue: quadstrips.  These are unlikely to appear.
    // random shades of red: triangles and tristrips.
    // green: fans and retesselated fan polygons.

    if (_colors.empty()) {
      // We need a handful of entries.
      _colors.push_back(Colorf(0.85, 0.85, 0.85, 1.0));  // default: white
      _colors.push_back(Colorf(0.0, 0.0, 0.75, 1.0));    // dark blue
      _colors.push_back(Colorf(0.4, 0.4, 0.8, 1.0));     // light blue
      _colors.push_back(Colorf(0.6, 0.6, 1.0, 1.0));     // very light blue
      _colors.push_back(Colorf(0.2, 0.8, 0.2, 1.0));     // green
    }

    ushort i1;
    Colorf color1;
    if (strip._origin == MO_user) {
      i1 = 0;
    } else if (strip._origin == MO_firstquad) {
      i1 = 1;
    } else if (strip._origin == MO_fanpoly) {
      i1 = 4;
    } else {
      switch (orig_type) {
      case BPT_quad:
        i1 = 2;
        break;

      case BPT_quadstrip:
        i1 = 3;
        break;

      case BPT_tristrip:
        make_random_color(color1);
        // Make it a shade of red.
        if (color1[0] < color1[1]) {
          float t = color1[0];
          color1[0] = color1[1];
          color1[1] = t;
        }
        color1[2] = color1[1];
        i1 = _colors.size();
        _colors.push_back(color1);
        break;

      case BPT_trifan:
        make_random_color(color1);
        // Make it a shade of green.
        if (color1[0] > color1[1]) {
          float t = color1[0];
          color1[0] = color1[1];
          color1[1] = t;
        }
        color1[2] = color1[0];
        i1 = _colors.size();
        _colors.push_back(color1);
        break;

      default:
        i1 = 0;
      }
    }

    // Now i1 is the color we want to assign to the whole primitive.
    // Just set all vertices to the same color.
    int num_verts = prim.get_num_verts();
    for (int i = 0; i < num_verts; i++) {
      prim.get_vertex(i).set_color_value(&_colors[0], i1);
    }
  }

  return prim;
}


template <class PrimType>
void MesherTempl<PrimType>::
finalize() {
  if (!_colors.empty()) {
    // Create an array in the bucket we might use to add to geoms.
    PTA_Colorf colors=PTA_Colorf::empty_array(_colors.size());
    for (int i = 0; i < (int)_colors.size(); i++) {
      colors[i] = _colors[i];
    }
    _bucket->set_colors(colors);

    _colors.clear();
    _color_sheets.clear();
  }
}



template <class PrimType>
void MesherTempl<PrimType>::
show(ostream &out) {
  /*
  out << _edges.size() << " edges:\n";
  copy(_edges.begin(), _edges.end(), ostream_iterator<Edge>(out, "\n"));
  */

  out << _verts.size() << " verts:\n";
  TYPENAME Verts::const_iterator vi;

  for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
    const Vertex &v = (*vi).first;
    const EdgePtrs &edges = (*vi).second;
    out << v << " shares " << count_vert_edges(edges) << " edges:\n";
    TYPENAME EdgePtrs::const_iterator ei;
    for (ei = edges.begin(); ei != edges.end(); ++ei) {
      if (!(*ei)->_strips.empty() || !(*ei)->_opposite->_strips.empty()) {
        out << "  " << **ei << "\n";
      }
    }
  }

  TYPENAME Strips::const_iterator si;
  out << _tris.size() << " tris:\n";
  for (si = _tris.begin(); si != _tris.end(); ++si) {
    out << (*si) << "\n";
  }

  out << _quads.size() << " quads:\n";
  for (si = _quads.begin(); si != _quads.end(); ++si) {
    out << (*si) << "\n";
  }

  out << _strips.size() << " strips:\n";
  for (si = _strips.begin(); si != _strips.end(); ++si) {
    out << (*si) << "\n";
  }
}


template <class PrimType>
int MesherTempl<PrimType>::
count_vert_edges(const EdgePtrs &edges) const {
  int count = 0;
  TYPENAME EdgePtrs::const_iterator ei;
  for (ei = edges.begin(); ei != edges.end(); ++ei) {
    count += (!(*ei)->_strips.empty() || !(*ei)->_opposite->_strips.empty());
  }
  return count;
}

template <class PrimType>
plist<TYPENAME MesherTempl<PrimType>::Strip> &MesherTempl<PrimType>::
choose_strip_list(const Strip &strip) {
  switch (strip._status) {
  case MS_done:
    return _done;

  case MS_dead:
    return _dead;

  case MS_alive:
    switch (strip._type) {
    case BPT_tri:
      return _tris;

    case BPT_quad:
      return _quads;

    default:
      return _strips;
    }

  default:
    builder_cat.fatal() << "Invalid strip status!\n";
    abort();
  }

  return _strips; // Unreachable; this is just to make the compiler happy.
}


template <class PrimType>
void MesherTempl<PrimType>::
build_sheets() {
  int first_row_id = 1;

  // First, move all the quads to our own internal list.
  Strips pre_sheeted;
  pre_sheeted.splice(pre_sheeted.end(), _quads);

  while (!pre_sheeted.empty()) {
    // Pick the first quad on the list.

    TYPENAME Strips::iterator best = pre_sheeted.begin();

    // If the row_id is negative, we've already built a sheet out of
    // this quad.  Leave it alone.  We also need to leave it be if it
    // has no available edges.
    if ((*best)._row_id >= 0 &&
        (*best)._status == MS_alive &&
        !(*best)._edges.empty()) {
      // There are two possible sheets we could make from this quad,
      // in two different orientations.  Measure them both and figure
      // out which one is best.

      const Edge *edge_a = (*best)._edges.front();
      const Edge *edge_b = (*best).find_adjacent_edge(edge_a);

      int num_prims_a = 0;
      int num_rows_a = 0;
      int first_row_id_a = first_row_id;
      (*best).measure_sheet(edge_a, true, num_prims_a, num_rows_a,
                           first_row_id_a, 0, 0);
      first_row_id += num_rows_a;
      double avg_length_a = (double)num_prims_a / (double)num_rows_a;

      int num_prims_b = 0;
      int num_rows_b = 0;
      int first_row_id_b = first_row_id;
      double avg_length_b;
      if (edge_b != NULL) {
        (*best).measure_sheet(edge_b, true, num_prims_b, num_rows_b,
                             first_row_id_b, 0, 0);
        first_row_id += num_rows_b;
        avg_length_b = (double)num_prims_b / (double)num_rows_b;
      }

      // Which sheet is better?
      if (edge_b != NULL && avg_length_b >= avg_length_a) {
        // Sheet b.  That's easy.
        (*best).cut_sheet(first_row_id_b, true, *_bucket);

      } else {
        // Nope, sheet a is better.  This is a bit of a nuisance
        // because we've unfortunately wiped out the information we
        // stored when we measured sheet a.  We'll have to do it
        // again.

        num_prims_a = 0;
        num_rows_a = 0;
        first_row_id_a = first_row_id;
        (*best).measure_sheet(edge_a, true, num_prims_a, num_rows_a,
                             first_row_id_a, 0, 0);
        first_row_id += num_rows_a;

        // Now we can cut it.
        (*best).cut_sheet(first_row_id_a, true, *_bucket);
      }
    }

    // Now put it somewhere.  We'll never see this quad again in
    // build_sheets().
    Strips &list = choose_strip_list(*best);
    list.splice(list.end(), pre_sheeted, best);
  }
}


template <class PrimType>
void MesherTempl<PrimType>::
find_fans() {
#ifdef SUPPORT_FANS
  pvector<Prim> unrolled_tris;

  // Consider all vertices.  Any vertex with over a certain number of
  // edges connected to it is eligible to become a fan.

  TYPENAME Verts::iterator vi;

  for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
    EdgePtrs &edges = (*vi).second;

    // 14 is the magic number of edges.  12 edges or fewer are likely
    // to be found on nearly every vertex in a quadsheet (six edges
    // times two, one each way).  We don't want to waste time fanning
    // out each vertex of a quadsheet, and we don't want to break up
    // the quadsheets anyway.  We bump this up to 14 because some
    // quadsheets are defined with triangles flipped here and there.
    if (edges.size() > 6) {
      const Vertex &v = (*vi).first;

      // Build up a list of far fan edges.
      typedef pvector<FanMaker> FanMakers;

      FanMakers fans;

      TYPENAME EdgePtrs::iterator ei;
      TYPENAME Edge::Strips::iterator si;
      for (ei = edges.begin(); ei != edges.end(); ++ei) {
        for (si = (*ei)->_strips.begin();
             si != (*ei)->_strips.end();
             ++si) {
          Strip *strip = *si;
          if (strip->_type == BPT_tri) {
            fans.push_back(FanMaker(&v, strip, this));
          }
        }
      }

      // Sort the fans list by edge pointers, and remove duplicates.
      sort(fans.begin(), fans.end());
      fans.erase(unique(fans.begin(), fans.end()),
                 fans.end());

      TYPENAME FanMakers::iterator fi, fi2;

      // Now pull out connected edges.
      int joined_any;
      do {
        joined_any = false;
        for (fi = fans.begin(); fi != fans.end(); ++fi) {
          if (!(*fi).is_empty()) {
            fi2 = fi;
            for (++fi2; fi2 != fans.end(); ++fi2) {
              if (!(*fi2).is_empty()) {
                joined_any = (*fi).join(*fi2);
              }
            }
          }
        }
      } while (joined_any);

      for (fi = fans.begin(); fi != fans.end(); ++fi) {
        if ((*fi).is_valid()) {
          (*fi).build(unrolled_tris);
        }
      }
    }
  }

  // Finally, add back in the triangles we might have produced by
  // unrolling some of the fans.  We can't add these back in safely
  // until we're done traversing all the vertices and primitives we
  // had in the first place (since adding them will affect the edge
  // lists).
  TYPENAME pvector<Prim>::iterator ti;
  for (ti = unrolled_tris.begin(); ti != unrolled_tris.end(); ++ti) {
    add_prim(*ti);
  }
#endif
}



////////////////////////////////////////////////////////////////////
//     Function: MesherTempl::make_quads
//       Access: Public
//  Description: Attempts to join up all the single tris to its
//               neighbor and reconstruct a pattern of quads, suitable
//               for making into quadsheets or at least quadstrips.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherTempl<PrimType>::
make_quads() {
  // Ideally, we want to match tris across their hypotenuse to make a
  // pattern of quads.  (This assumes that we are working with a
  // triangulated mesh pattern, of course.  If we have some other
  // pattern of tris, all bets are off and it doesn't really matter
  // anyway.)

  // First, we'll find all the tris that have no doubt about their
  // ideal mate, and pair them up right away.  The others we'll get to
  // later.  This way, the uncertain matches won't pollute the quad
  // alignment for everyone else.

  typedef pair<Strip *, Strip *> Pair;
  typedef pair<Pair, Edge *> Matched;
  typedef pvector<Matched> SoulMates;

  SoulMates soulmates;

  Strip *tri, *mate, *mate2;
  Edge *common_edge, *common_edge2;

  TYPENAME Strips::iterator si;
  for (si = _tris.begin(); si != _tris.end(); ++si) {
    tri = &(*si);

    if (tri->_status == MS_alive) {
      if (tri->find_ideal_mate(mate, common_edge, *_bucket)) {
        // Does our chosen mate want us too?
        if (mate->_type == BPT_tri && mate->_status == MS_alive &&
            mate->find_ideal_mate(mate2, common_edge2, *_bucket) &&
            mate2 == tri) {
          // Hooray!
          soulmates.push_back(Matched(Pair(tri, mate), common_edge));
          // We'll temporarily mark the two tris as paired.
          tri->_status = MS_paired;
          mate->_status = MS_paired;
        }
      }
    }
  }

  // Now that we've found all the tris that are sure about each other,
  // mate them.
  TYPENAME SoulMates::iterator mi;
  for (mi = soulmates.begin(); mi != soulmates.end(); ++mi) {
    tri = (*mi).first.first;
    mate = (*mi).first.second;
    common_edge = (*mi).second;

    nassertv(tri->_status == MS_paired);
    nassertv(mate->_status == MS_paired);
    tri->_status = MS_alive;
    mate->_status = MS_alive;

    Strip::mate_pieces(common_edge, *tri, *mate, *_bucket);
    tri->_origin = MO_firstquad;
  }

  // Now move all the strips off the tri list that no longer belong.
  TYPENAME Strips::iterator next;
  si = _tris.begin();
  while (si != _tris.end()) {
    next = si;
    ++next;

    Strips &list = choose_strip_list(*si);
    if (&list != &_tris) {
      list.splice(list.end(), _tris, si);
    }

    si = next;
  }
}

template <class PrimType>
void MesherTempl<PrimType>::
meshList(Strips &strips) {
  while (!strips.empty()) {
    // Pick the first strip on the list.

    TYPENAME Strips::iterator best = strips.begin();

    if ((*best)._status == MS_alive) {
      (*best).mate(*_bucket);
    }

    // Put the strip back on the end of whichever list it wants.  This
    // might be the same list, if the strip is still alive, or it
    // might be _done or _dead.
    Strips &list = choose_strip_list(*best);
    list.splice(list.end(), strips, best);
  }
}
