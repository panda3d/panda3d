/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesher.cxx
 * @author drose
 * @date 2005-03-13
 */

#include "eggMesher.h"
#include "eggMesherFanMaker.h"
#include "eggPolygon.h"
#include "eggCompositePrimitive.h"
#include "eggTriangleStrip.h"
#include "eggTriangleFan.h"
#include "eggGroup.h"
#include "config_egg.h"
#include "eggGroupNode.h"
#include "dcast.h"
#include "thread.h"

#include <stdlib.h>

/**
 *
 */
EggMesher::
EggMesher() {
  _vertex_pool = nullptr;
  _strip_index = 0;
}

/**
 * Accepts an EggGroupNode, which contains a set of EggPrimitives--typically,
 * triangles and quads--as children.  Removes these primitives and replaces
 * them with (mostly) equivalent EggTriangleStrips and EggTriangleFans where
 * possible.
 *
 * If flat_shaded is true, then odd-length triangle strips, and triangle fans
 * of any length, are not permitted (because these can't be rotated when
 * required to move the colored vertex of each triangle to the first or last
 * position).
 */
void EggMesher::
mesh(EggGroupNode *group, bool flat_shaded) {
  _flat_shaded = flat_shaded;

  // Create a temporary node to hold the children of group that aren't
  // involved in the meshing, as well as the newly-generate triangle strips.
  PT(EggGroupNode) output_children = new EggGroupNode;

  // And another to hold the children that will be processed next time.
  PT(EggGroupNode) next_children = new EggGroupNode;
  PT(EggGroupNode) this_children = group;

  // Only primitives that share a common vertex pool can be meshed together.
  // Thus, pull out the primitives with the same vertex pool in groups.
  while (this_children->size() != 0) {
    clear();

    // Add each polygon in the group to the mesh pool.
    while (!this_children->empty()) {
      PT(EggNode) child = this_children->get_first_child();
      this_children->remove_child(child);

      if (child->is_of_type(EggPolygon::get_class_type())) {
        EggPolygon *poly = DCAST(EggPolygon, child);

        if (_vertex_pool == nullptr) {
          _vertex_pool = poly->get_pool();
          add_polygon(poly, EggMesherStrip::MO_user);

        } else if (_vertex_pool == poly->get_pool()) {
          add_polygon(poly, EggMesherStrip::MO_user);

        } else {
          // A different vertex pool; save this one for the next pass.
          next_children->add_child(poly);
        }

      } else {
        // If it's not a polygon of any kind, just output it unchanged.
        output_children->add_child(child);
      }
    }

    do_mesh();

    Strips::iterator si;
    for (si = _done.begin(); si != _done.end(); ++si) {
      PT(EggPrimitive) egg_prim = get_prim(*si);
      if (egg_prim != nullptr) {
        output_children->add_child(egg_prim);
      }
    }

    this_children = next_children;
    next_children = new EggGroupNode;
  }

  // Now copy the newly-meshed primitives back to the group.
  group->clear();
  group->steal_children(*output_children);

  clear();
}

/**
 *
 */
void EggMesher::
write(std::ostream &out) const {
  /*
  out << _edges.size() << " edges:\n";
  copy(_edges.begin(), _edges.end(), ostream_iterator<Edge>(out, "\n"));
  */

  out << _verts.size() << " verts:\n";
  Verts::const_iterator vi;

  for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
    int v = (*vi).first;
    const EdgePtrs &edges = (*vi).second;
    out << v << " shares " << count_vert_edges(edges) << " edges:\n";
    EdgePtrs::const_iterator ei;
    for (ei = edges.begin(); ei != edges.end(); ++ei) {
      if (!(*ei)->_strips.empty() || !(*ei)->_opposite->_strips.empty()) {
        out << "  " << **ei << "\n";
      }
    }
  }

  Strips::const_iterator si;
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

/**
 * Empties the pool of meshable primitives and resets to an initial state.
 */
void EggMesher::
clear() {
  _tris.clear();
  _quads.clear();
  _strips.clear();
  _dead.clear();
  _done.clear();
  _verts.clear();
  _edges.clear();
  _strip_index = 0;
  _vertex_pool = nullptr;
  _color_sheets.clear();
}

/**
 * Adds a single polygon into the pool of available primitives for meshing.
 */
bool EggMesher::
add_polygon(const EggPolygon *egg_poly, EggMesherStrip::MesherOrigin origin) {
  CPT(EggPolygon) this_poly = egg_poly;
  if (this_poly->size() != 3) {
    // If we have a higher-order or concave polygon, triangulate it
    // automatically.

    // We'll keep quads, unless they're concave.
    bool convex_also = (this_poly->size() != 4);

    PT(EggGroupNode) temp_group = new EggGroupNode;
    bool result = this_poly->triangulate_into(temp_group, convex_also);
    EggGroupNode::iterator ci;
    if (temp_group->size() != 1) {
      for (ci = temp_group->begin(); ci != temp_group->end(); ++ci) {
        add_polygon(DCAST(EggPolygon, *ci), EggMesherStrip::MO_user);
      }
      return result;
    }

    // Convert just the one polygon we got out of the group.  Don't recurse,
    // since it might be the same polygon we sent in.
    ci = temp_group->begin();
    this_poly = DCAST(EggPolygon, *ci);
  }

  if (_vertex_pool == nullptr) {
    _vertex_pool = this_poly->get_pool();
  } else {
    nassertr(_vertex_pool == this_poly->get_pool(), false);
  }

  // Define an initial strip (probably of length 1) for the prim.
  EggMesherStrip temp_strip(this_poly, _strip_index++, _vertex_pool,
                            _flat_shaded);
  Strips &list = choose_strip_list(temp_strip);
  list.push_back(temp_strip);
  EggMesherStrip &strip = list.back();
  strip._origin = origin;

  int i;
  int num_verts = this_poly->size();

  int *vptrs = (int *)alloca(num_verts * sizeof(int));
  EdgePtrs **eptrs = (EdgePtrs **)alloca(num_verts * sizeof(EdgePtrs *));

  // Get the common vertex pointers for the primitive's vertices.
  for (i = 0; i < num_verts; i++) {
    Verts::value_type v(this_poly->get_vertex(i)->get_index(), EdgePtrs());
    Verts::iterator n = _verts.insert(v).first;

    vptrs[i] = (*n).first;
    eptrs[i] = &(*n).second;

    strip._verts.push_back(vptrs[i]);
  }

  // Now identify the common edges.
  for (i = 0; i < num_verts; i++) {
    // Define an inner and outer edge.  A polygon shares an edge with a
    // neighbor only when one of its inner edges matches a neighbor's outer
    // edge (and vice-versa).
    EggMesherEdge inner(vptrs[i], vptrs[(i+1) % num_verts]);
    EggMesherEdge outer(vptrs[(i+1) % num_verts], vptrs[i]);

    // Add it to the list and get its common pointer.
    EggMesherEdge &inner_ref = (EggMesherEdge &)*_edges.insert(inner).first;
    EggMesherEdge &outer_ref = (EggMesherEdge &)*_edges.insert(outer).first;

    // Tell the edges about each other.
    inner_ref._opposite = &outer_ref;
    outer_ref._opposite = &inner_ref;

    // Associate the common edge to the strip.
    strip._edges.push_back(&inner_ref);

    // Associate the strip, as well as the original prim, to the edge.
    outer_ref._strips.push_back(&strip);

    // Associate the common edge with the vertices that share it.
    // EggMesherEdge *edge_ptr = inner_ref.common_ptr();
    eptrs[i]->insert(&outer_ref);
    eptrs[(i+1) % num_verts]->insert(&outer_ref);
  }

  return true;
}

/**
 * Performs the meshing process on the set of primitives that have been added
 * via add_prim(), leaving the result in _done.
 */
void EggMesher::
do_mesh() {
  if (egg_consider_fans && !_flat_shaded) {
    find_fans();
  }

  // First, we try to make all the best quads we can.
  if (egg_retesselate_coplanar) {
    make_quads();
  }

  // Then, we do the rest of the tris.
  mesh_list(_tris);

  if (egg_show_quads) {
    // If we're showing quads, we shouldn't do any more meshing.
    Strips::iterator si;
    for (si = _quads.begin(); si != _quads.end(); ++si) {
      if ((*si)._status == EggMesherStrip::MS_alive) {
        (*si)._status = EggMesherStrip::MS_done;
      }
    }
    for (si = _strips.begin(); si != _strips.end(); ++si) {
      if ((*si)._status == EggMesherStrip::MS_alive) {
        (*si)._status = EggMesherStrip::MS_done;
      }
    }
  }

  // Then, build quads into sheets where possible.
  build_sheets();

  // Pick up any quads that might have been left behind.
  mesh_list(_quads);

  // Finally, do the longer strips.
  mesh_list(_strips);

  Thread::consider_yield();
}

/**
 * Creates an EggPrimitive that represents the result of the meshed
 * EggMesherStrip object.
 */
PT(EggPrimitive) EggMesher::
get_prim(EggMesherStrip &strip) {
  EggMesherStrip::PrimType orig_type = strip._type;
  PT(EggPrimitive) egg_prim = strip.make_prim(_vertex_pool);

  if (egg_show_tstrips) {
    // If we have egg_show_tstrips on, it means we need to color every
    // primitive according to which, if any, tristrip it is in.

    LColor color1, color2;

    if (egg_prim->is_of_type(EggTriangleStrip::get_class_type()) ||
        egg_prim->is_of_type(EggTriangleFan::get_class_type())) {
      make_random_color(color2);
      color1 = (color2 * 0.8);   // somewhat darker.

    } else {
      // not-a-tristrip.
      color1.set(0.85, 0.85, 0.85, 1.0);
      color2.set(0.85, 0.85, 0.85, 1.0);
    }

    // Now color1 and color2 indicate the color for the first triangle and the
    // rest of the primitive, respectively.
    if (egg_prim->is_of_type(EggCompositePrimitive::get_class_type())) {
      EggCompositePrimitive *egg_comp = DCAST(EggCompositePrimitive, egg_prim);
      int num_components = egg_comp->get_num_components();
      if (num_components > 0) {
        egg_comp->get_component(0)->set_color(color1);
        for (int i = 1; i < num_components; i++) {
          egg_comp->get_component(i)->set_color(color2);
        }
      }
    } else {
      egg_prim->set_color(color1);
    }

    int num_verts = egg_prim->size();
    for (int i = 0; i < num_verts; i++) {
      egg_prim->get_vertex(i)->clear_color();
    }

  } else if (egg_show_qsheets) {
    // egg_show_qsheets means to color every primitive according to which, if
    // any, quadsheet it is in.  This is a bit easier, because the entire
    // primitive gets the same color.

    // Is this a quadsheet?
    LColor color1;
    if (strip._row_id < 0) {
      // Yep!  Assign a new color, if it doesn't already have one.
      ColorSheetMap::iterator ci = _color_sheets.find(strip._row_id);

      if (ci == _color_sheets.end()) {
        make_random_color(color1);
        _color_sheets[strip._row_id] = color1;
      } else {
        color1 = (*ci).second;
      }
    }

    // Now color1 is the color we want to assign to the whole primitive.
    egg_prim->set_color(color1);
    if (egg_prim->is_of_type(EggCompositePrimitive::get_class_type())) {
      EggCompositePrimitive *egg_comp = DCAST(EggCompositePrimitive, egg_prim);
      int num_components = egg_comp->get_num_components();
      for (int i = 0; i < num_components; i++) {
        egg_comp->get_component(i)->clear_color();
      }
    }
    int num_verts = egg_prim->size();
    for (int i = 0; i < num_verts; i++) {
      egg_prim->get_vertex(i)->clear_color();
    }

  } else if (egg_show_quads) {
    // egg_show_quads means to show the assembling of tris into quads and
    // fans.

    // We use the following color convention:

    // white: unchanged; as supplied by user.  dark blue: quads made in the
    // initial pass.  These are more certain.  light blue: quads made in the
    // second pass.  These are less certain.  very light blue: quadstrips.
    // These are unlikely to appear.  random shades of red: triangles and
    // tristrips.  green: fans and retesselated fan polygons.

    // We need a handful of entries.
    LColor white(0.85, 0.85, 0.85, 1.0);
    LColor dark_blue(0.0, 0.0, 0.75, 1.0);
    LColor light_blue(0.4, 0.4, 0.8, 1.0);
    LColor very_light_blue(0.6, 0.6, 1.0, 1.0);
    LColor green(0.2, 0.8, 0.2, 1.0);

    LColor color1;
    if (strip._origin == EggMesherStrip::MO_user) {
      color1 = white;
    } else if (strip._origin == EggMesherStrip::MO_firstquad) {
      color1 = dark_blue;
    } else if (strip._origin == EggMesherStrip::MO_fanpoly) {
      color1 = green;
    } else {
      switch (orig_type) {
      case EggMesherStrip::PT_quad:
        color1 = light_blue;
        break;

      case EggMesherStrip::PT_quadstrip:
        color1 = very_light_blue;
        break;

      case EggMesherStrip::PT_tristrip:
        make_random_color(color1);
        // Make it a shade of red.
        if (color1[0] < color1[1]) {
          PN_stdfloat t = color1[0];
          color1[0] = color1[1];
          color1[1] = t;
        }
        color1[2] = color1[1];
        break;

      case EggMesherStrip::PT_trifan:
        make_random_color(color1);
        // Make it a shade of green.
        if (color1[0] > color1[1]) {
          PN_stdfloat t = color1[0];
          color1[0] = color1[1];
          color1[1] = t;
        }
        color1[2] = color1[0];
        break;

      default:
        color1 = white;
      }
    }

    // Now color1 is the color we want to assign to the whole primitive.
    egg_prim->set_color(color1);
    if (egg_prim->is_of_type(EggCompositePrimitive::get_class_type())) {
      EggCompositePrimitive *egg_comp = DCAST(EggCompositePrimitive, egg_prim);
      int num_components = egg_comp->get_num_components();
      for (int i = 0; i < num_components; i++) {
        egg_comp->get_component(i)->clear_color();
      }
    }
    int num_verts = egg_prim->size();
    for (int i = 0; i < num_verts; i++) {
      egg_prim->get_vertex(i)->clear_color();
    }
  }

  return egg_prim;
}

/**
 * Returns the number of edges in the list that are used by at least one
 * EggMesherStrip object.
 */
int EggMesher::
count_vert_edges(const EdgePtrs &edges) const {
  int count = 0;
  EdgePtrs::const_iterator ei;
  for (ei = edges.begin(); ei != edges.end(); ++ei) {
    count += (!(*ei)->_strips.empty() || !(*ei)->_opposite->_strips.empty());
  }
  return count;
}

/**
 * Selects which of several strip lists on the EggMesher class the indicated
 * EggMesherStrip should be added to.
 */
plist<EggMesherStrip> &EggMesher::
choose_strip_list(const EggMesherStrip &strip) {
  switch (strip._status) {
  case EggMesherStrip::MS_done:
    return _done;

  case EggMesherStrip::MS_dead:
    return _dead;

  case EggMesherStrip::MS_alive:
    switch (strip._type) {
    case EggMesherStrip::PT_tri:
      return _tris;

    case EggMesherStrip::PT_quad:
      return _quads;

    default:
      return _strips;
    }

  default:
    egg_cat.fatal() << "Invalid strip status!\n";
    abort();
  }

  return _strips; // Unreachable; this is just to make the compiler happy.
}

/**
 * Attempts to locate large quadsheets in the polygon soup.  A quadsheet is
 * defined as a uniform rectangular mesh of quads joined at the corners.
 *
 * Sheets like this are commonly output by modeling packages, especially
 * uniform tesselators, and they are trivially converted into a row of
 * triangle strips.
 */
void EggMesher::
build_sheets() {
  int first_row_id = 1;

  // First, move all the quads to our own internal list.
  Strips pre_sheeted;
  pre_sheeted.splice(pre_sheeted.end(), _quads);

  while (!pre_sheeted.empty()) {
    // Pick the first quad on the list.

    Strips::iterator best = pre_sheeted.begin();

    // If the row_id is negative, we've already built a sheet out of this
    // quad.  Leave it alone.  We also need to leave it be if it has no
    // available edges.
    if ((*best)._row_id >= 0 &&
        (*best)._status == EggMesherStrip::MS_alive &&
        !(*best)._edges.empty()) {
      // There are two possible sheets we could make from this quad, in two
      // different orientations.  Measure them both and figure out which one
      // is best.

      const EggMesherEdge *edge_a = (*best)._edges.front();
      const EggMesherEdge *edge_b = (*best).find_adjacent_edge(edge_a);

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
      if (edge_b != nullptr) {
        (*best).measure_sheet(edge_b, true, num_prims_b, num_rows_b,
                             first_row_id_b, 0, 0);
        first_row_id += num_rows_b;
        avg_length_b = (double)num_prims_b / (double)num_rows_b;
      }

      // Which sheet is better?
      if (edge_b != nullptr && avg_length_b >= avg_length_a) {
        // Sheet b.  That's easy.
        (*best).cut_sheet(first_row_id_b, true, _vertex_pool);

      } else {
        // Nope, sheet a is better.  This is a bit of a nuisance because we've
        // unfortunately wiped out the information we stored when we measured
        // sheet a.  We'll have to do it again.

        num_prims_a = 0;
        num_rows_a = 0;
        first_row_id_a = first_row_id;
        (*best).measure_sheet(edge_a, true, num_prims_a, num_rows_a,
                             first_row_id_a, 0, 0);
        first_row_id += num_rows_a;

        // Now we can cut it.
        (*best).cut_sheet(first_row_id_a, true, _vertex_pool);
      }
    }

    // Now put it somewhere.  We'll never see this quad again in
    // build_sheets().
    Strips &list = choose_strip_list(*best);
    list.splice(list.end(), pre_sheeted, best);
  }
}

/**
 * Looks for cases of multiple polygons all sharing a common vertex, and
 * replaces these with a single fan.
 *
 * This step is performed before detecting triangle strips.  We have to be
 * careful: if we are too aggressive in detecting fans, we may ruin the
 * ability to build good triangle strips, and we may thereby end up with a
 * less-than-optimal solution.
 */
void EggMesher::
find_fans() {
  PT(EggGroupNode) unrolled_tris = new EggGroup;

  // Consider all vertices.  Any vertex with over a certain number of edges
  // connected to it is eligible to become a fan.

  Verts::iterator vi;

  for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
    EdgePtrs &edges = (*vi).second;

    // 14 is the magic number of edges.  12 edges or fewer are likely to be
    // found on nearly every vertex in a quadsheet (six edges times two, one
    // each way).  We don't want to waste time fanning out each vertex of a
    // quadsheet, and we don't want to break up the quadsheets anyway.  We
    // bump this up to 14 because some quadsheets are defined with triangles
    // flipped here and there.
    if (edges.size() > 6) {
      int v = (*vi).first;

      // Build up a list of far fan edges.
      typedef pvector<EggMesherFanMaker> FanMakers;
      FanMakers fans;

      EdgePtrs::iterator ei;
      EggMesherEdge::Strips::iterator si;
      for (ei = edges.begin(); ei != edges.end(); ++ei) {
        for (si = (*ei)->_strips.begin();
             si != (*ei)->_strips.end();
             ++si) {
          EggMesherStrip *strip = *si;
          if (strip->_type == EggMesherStrip::PT_tri) {
            EggMesherFanMaker fan(v, strip, this);
            if (!fan._edges.empty()) {
              fans.push_back(fan);
            }
          }
        }
      }

      // Sort the fans list by edge pointers, and remove duplicates.
      sort(fans.begin(), fans.end());
      fans.erase(unique(fans.begin(), fans.end()),
                 fans.end());

      FanMakers::iterator fi, fi2;

      // Now pull out connected edges.
      bool joined_any;
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

  // Finally, add back in the triangles we might have produced by unrolling
  // some of the fans.  We can't add these back in safely until we're done
  // traversing all the vertices and primitives we had in the first place
  // (since adding them will affect the edge lists).
  EggGroupNode::iterator ti;
  for (ti = unrolled_tris->begin(); ti != unrolled_tris->end(); ++ti) {
    add_polygon(DCAST(EggPolygon, (*ti)), EggMesherStrip::MO_fanpoly);
  }
}

/**
 * Attempts to join up each single tri to its neighbor, to reconstruct a
 * pattern of quads, suitable for making into quadsheets or at least
 * quadstrips.
 *
 * Quads have some nice properties that make them easy to manipulate when
 * meshing.  We will ultimately convert the quadsheets and quadstrips into
 * tristrips, but it's easier to work with them first while they're quads.
 */
void EggMesher::
make_quads() {
  // Ideally, we want to match tris across their hypotenuse to make a pattern
  // of quads.  (This assumes that we are working with a triangulated mesh
  // pattern, of course.  If we have some other pattern of tris, all bets are
  // off and it doesn't really matter anyway.)

  // First, we'll find all the tris that have no doubt about their ideal mate,
  // and pair them up right away.  The others we'll get to later.  This way,
  // the uncertain matches won't pollute the quad alignment for everyone else.

  typedef std::pair<EggMesherStrip *, EggMesherStrip *> Pair;
  typedef std::pair<Pair, EggMesherEdge *> Matched;
  typedef pvector<Matched> SoulMates;

  SoulMates soulmates;

  EggMesherStrip *tri, *mate, *mate2;
  EggMesherEdge *common_edge, *common_edge2;

  Strips::iterator si;
  for (si = _tris.begin(); si != _tris.end(); ++si) {
    tri = &(*si);

    if (tri->_status == EggMesherStrip::MS_alive) {
      if (tri->find_ideal_mate(mate, common_edge, _vertex_pool)) {
        // Does our chosen mate want us too?
        if (mate->_type == EggMesherStrip::PT_tri &&
            mate->_status == EggMesherStrip::MS_alive &&
            mate->find_ideal_mate(mate2, common_edge2, _vertex_pool) &&
            mate2 == tri) {
          // Hooray!
          soulmates.push_back(Matched(Pair(tri, mate), common_edge));
          // We'll temporarily mark the two tris as paired.
          tri->_status = EggMesherStrip::MS_paired;
          mate->_status = EggMesherStrip::MS_paired;
        }
      }
    }
  }

  // Now that we've found all the tris that are sure about each other, mate
  // them.
  SoulMates::iterator mi;
  for (mi = soulmates.begin(); mi != soulmates.end(); ++mi) {
    tri = (*mi).first.first;
    mate = (*mi).first.second;
    common_edge = (*mi).second;

    nassertv(tri->_status == EggMesherStrip::MS_paired);
    nassertv(mate->_status == EggMesherStrip::MS_paired);
    tri->_status = EggMesherStrip::MS_alive;
    mate->_status = EggMesherStrip::MS_alive;

    EggMesherStrip::mate_pieces(common_edge, *tri, *mate, _vertex_pool);
    tri->_origin = EggMesherStrip::MO_firstquad;
  }

  // Now move all the strips off the tri list that no longer belong.
  Strips::iterator next;
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

/**
 * Processes all of the strips on the indicated list.
 */
void EggMesher::
mesh_list(Strips &strips) {
  while (!strips.empty()) {
    // Pick the first strip on the list.

    Strips::iterator best = strips.begin();

    if ((*best)._status == EggMesherStrip::MS_alive) {
      (*best).mate(_vertex_pool);
    }

    // Put the strip back on the end of whichever list it wants.  This might
    // be the same list, if the strip is still alive, or it might be _done or
    // _dead.
    Strips &list = choose_strip_list(*best);
    list.splice(list.end(), strips, best);
  }
}

/**
 * Chooses a reasonable random color.
 */
void EggMesher::
make_random_color(LColor &color) {
  LVector3 rgb;
  PN_stdfloat len;
  do {
    for (int i = 0; i < 3; i++) {
      rgb[i] = (double)rand() / (double)RAND_MAX;
    }
    len = length(rgb);

    // Repeat until we have a color that's not too dark or too light.
  } while (len < .1 || len > 1.5);

  color.set(rgb[0], rgb[1], rgb[2],
            0.25 + 0.75 * (double)rand() / (double)RAND_MAX);
}
