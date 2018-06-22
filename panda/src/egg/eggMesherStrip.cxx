/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherStrip.cxx
 * @author drose
 * @date 2005-03-13
 */

#include "eggMesherStrip.h"
#include "eggMesherEdge.h"
#include "eggPrimitive.h"
#include "eggTriangleFan.h"
#include "eggTriangleStrip.h"
#include "eggPolygon.h"
#include "dcast.h"
#include "config_egg.h"

/**
 *
 */
EggMesherStrip::
EggMesherStrip(PrimType prim_type, MesherOrigin origin) {
  _origin = origin;
  _type = prim_type;

  _index = -1;
  _row_id = 0;
  _status = MS_alive;
  _planar = false;
  _flat_shaded = false;
}

/**
 *
 */
EggMesherStrip::
EggMesherStrip(const EggPrimitive *prim, int index,
               const EggVertexPool *vertex_pool,
               bool flat_shaded) {
  _index = index;
  _row_id = 0;
  _status = MS_alive;
  _origin = MO_unknown;
  _flat_shaded = flat_shaded;

  _type = PT_poly; //prim.get_type();

  // We care only about the prim's attributes in the _prims array.  The
  // vertices get re-added later by EggMesher::add_prim().
  _prims.push_back(prim);

  if (_type == PT_poly) {
    switch (prim->size()) {
    case 3:
      _type = PT_tri;
      break;

    case 4:
      _type = PT_quad;
      break;
    }
  }

  if (_type == PT_quad) {
    // A quad has two internal triangles; we therefore push the prim
    // attributes twice.
    _prims.push_back(prim);
  }

  _planar = false;

  if (prim->is_of_type(EggPolygon::get_class_type())) {
    // Although for the most part we ignore the actual value of the vertices,
    // we will ask the polygon for its plane equation (i.e.  its normal).
    LNormald normal;
    if (DCAST(EggPolygon, prim)->calculate_normal(normal)) {
      _plane_normal = normal;
      _planar = true;
      LPoint3d p1 = prim->get_vertex(0)->get_pos3();
      _plane_offset = -dot(_plane_normal, p1);
    }
  }
}


/**
 * Creates an EggPrimitive corresponding to the strip represented by this
 * node.
 */
PT(EggPrimitive) EggMesherStrip::
make_prim(const EggVertexPool *vertex_pool) {
  PT(EggPrimitive) prim;
  PrimType dest_type;

  switch (_type) {
  case PT_quad:
  case PT_tristrip:
  case PT_quadstrip:
    dest_type = PT_tristrip;
    break;

  case PT_trifan:
    dest_type = PT_trifan;
    break;

  default:
    dest_type = _type;
  }

  if (dest_type != PT_tristrip && dest_type != PT_trifan) {
    // The easy case: a simple primitive, i.e.  a polygon.
    prim = new EggPolygon;
    prim->copy_attributes(*_prims.front());

    Verts::iterator vi;
    for (vi = _verts.begin(); vi != _verts.end(); ++vi) {
      prim->add_vertex(vertex_pool->get_vertex(*vi));
    }

  } else {
    // The harder case: a tristrip of some kind.
    convert_to_type(dest_type);
    if (dest_type == PT_trifan) {
      prim = new EggTriangleFan;
    } else {
      prim = new EggTriangleStrip;
    }
    prim->copy_attributes(*_prims.front());

    // Now store all the vertices.  Each individual triangle's attributes, if
    // any, get applied to the third vertex of each triangle.
    Verts::iterator vi;
    Prims::iterator pi;
    pi = _prims.begin();
    int count = 0;
    for (vi = _verts.begin();
         vi != _verts.end() && pi != _prims.end();
         ++vi) {
      PT(EggVertex) vertex = vertex_pool->get_vertex(*vi);
      prim->add_vertex(vertex);

      ++count;
      if (count >= 3) {
        // Beginning with the third vertex, we increment pi.  Thus, the first
        // two vertices stand alone, then each vertex beginning with the third
        // completes a triangle.
        const EggAttributes *attrib = (*pi);
        ++pi;
        DCAST(EggCompositePrimitive, prim)->set_component(count - 3, attrib);
      }
    }

    // If either of these fail, there weren't num_prims + 2 vertices in the
    // tristrip!
    nassertr(vi == _verts.end(), prim);
    nassertr(pi == _prims.end(), prim);
  }

  return prim;
}

/**
 * Determines the extents of the quadsheet that can be derived by starting
 * with this strip, and searching in the direction indicated by the given
 * edge.
 */
void EggMesherStrip::
measure_sheet(const EggMesherEdge *edge, int new_row, int &num_prims,
              int &num_rows, int first_row_id, int this_row_id,
              int this_row_distance) {
  if (new_row) {
    // If we would create a new row by stepping here, we won't stay if there
    // was any other row already defined here.
    if (_row_id >= first_row_id) {
      return;
    }
  } else {
    // On the other hand, if this is a continuation of the current row, we'll
    // stay if the other row had to travel farther to get here.
    if (_row_id >= first_row_id && _row_distance <= this_row_distance) {
      return;
    }
  }

  num_prims += _prims.size();
  if (new_row) {
    ++num_rows;
    this_row_id = first_row_id + num_rows - 1;
  }

  _row_id = this_row_id;

  Edges::iterator ei;
  EggMesherEdge::Strips::iterator si;

  if (_type == PT_quad) {
    // If this is a quad, it has four neighbors: two in the direction we are
    // testing, and two in an orthagonal direction.

    int vi_a = edge->_vi_a;
    int vi_b = edge->_vi_b;

    // We use these vertices to differentiate the edges that run in our
    // primary direction from those in the secondary direction.  For each
    // edge, we count the number of vertices that the edge shares with our
    // starting edge.  There are then three cases:

    // (a) The edge shares two vertices.  It is the direction we came from;
    // forget it.

    // (b) The edge shares one vertex.  It is at right angles to our starting
    // edge.  This is the primary direction if new_row is true, and the
    // secondary direction if new_row is false.

    // (c) The edge shares no vertices.  It is directly opposite our starting
    // edge.  This is the primary direction if new_row is false, and the
    // secondary direction if new_row is true.


    // Here's a silly little for loop that executes the following code twice:
    // once with secondary == 0, and once with secondary == 1. This is because
    // we want to find all the primary edges first, and then all the secondary
    // edges.

    for (int secondary = 0; secondary <= 1; secondary++) {
      // How many common vertices are we looking for this pass (see above)?

      int want_count;
      if (secondary) {
        want_count = new_row ? 0 : 1;
      } else {
        want_count = new_row ? 1 : 0;
      }

      for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
        int common_verts =
          ((*ei)->_vi_a == vi_a || (*ei)->_vi_a == vi_b) +
          ((*ei)->_vi_b == vi_a || (*ei)->_vi_b == vi_b);

        if (common_verts == want_count) {
          // Here's the edge.  Look at all its connections.  Hopefully, there
          // will only be one besides ourselves, but there may be more.  Pick
          // the best.

          EggMesherEdge::Strips &strips = (*ei)->_strips;
          EggMesherStrip *mate = nullptr;
          for (si = strips.begin(); si != strips.end(); ++si) {
            if ((*si)->_row_id < first_row_id) {
              if (mate == nullptr || pick_sheet_mate(**si, *mate)) {
                mate = *si;
              }
            }
          }
          if (mate!=nullptr) {
            mate->measure_sheet(*ei, secondary, num_prims, num_rows,
                                first_row_id, this_row_id,
                                this_row_distance + secondary);
          }
        }
      }
    }

  } else {
    // Otherwise, this is not a quad.  It's certainly not a triangle, because
    // we've built all the single triangles already.
    nassertv(_type != PT_tri);

    // Therefore, it must be a tristrip or quadstrip.
    nassertv(_type == PT_tristrip || _type == PT_quadstrip);

    // Since it's a strip, it only has two neighbors: the one we came from,
    // and the other one.  Find the other one.

    for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
      if (!(*ei)->matches(*edge)) {
        // Here's the edge.  Same drill as above.

        EggMesherEdge::Strips &strips = (*ei)->_strips;
        EggMesherStrip *mate = nullptr;
        for (si = strips.begin(); si != strips.end(); ++si) {
          if ((*si)->_row_id < first_row_id) {
            if (mate == nullptr || pick_sheet_mate(**si, *mate)) {
              mate = *si;
            }
          }
        }
        if (mate != nullptr) {
          mate->measure_sheet(*ei, false, num_prims, num_rows,
                              first_row_id, this_row_id, this_row_distance);
        }
      }
    }
  }
}


/**
 *
 */
void EggMesherStrip::
cut_sheet(int first_row_id, int do_mate, const EggVertexPool *vertex_pool) {
  Edges::iterator ei;
  EggMesherEdge::Strips::iterator si;

  // First, start the process going on any neighbors that belong to a later
  // row.  (We must do these first, because we'll change our neighbor list
  // when we start to mate.)

  // We need to build a temporary list of neighbors first, because calling
  // cut_sheet() recursively will start things mating, and could damage our
  // edge list.

  typedef plist<EggMesherStrip *> StripPtrs;
  StripPtrs strip_ptrs;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    EggMesherEdge::Strips &strips = (*ei)->_strips;
    for (si = strips.begin(); si != strips.end(); ++si) {
      if ((*si)->_row_id > _row_id) {
        // Here's a different row in the sheet!
        strip_ptrs.push_back(*si);
      }
    }
  }

  // Now walk the temporary list and do some damage.  We pass do_mate = true
  // to each of these neighbors, because as far as we know, they're the first
  // nodes of a particular row.
  StripPtrs::iterator spi;
  for (spi = strip_ptrs.begin(); spi != strip_ptrs.end(); ++spi) {
    if ((*spi)->_status == MS_alive) {
      (*spi)->cut_sheet(first_row_id, true, vertex_pool);
    }
  }


  if (do_mate && _status == MS_alive) {
    // Now mate like a bunny until we don't have any more eligible mates.
    int not_any;
    do {
      not_any = true;

      ei = _edges.begin();
      while (not_any && ei != _edges.end()) {
        EggMesherEdge::Strips &strips = (*ei)->_strips;
        si = strips.begin();
        while (not_any && si != strips.end()) {
          if (*si != this && (*si)->_row_id == _row_id) {
            // Here's one!
            not_any = false;
            EggMesherStrip *mate = *si;

            // We also recurse on these guys so they can spread the word to
            // their own neighbors.  This time we don't need to build a
            // temporary list, because we'll be restarting from the beginning
            // of our edge list after we do this.  We also pass do_mate =
            // false to these guys because we're the ones doing the mating
            // here.
            mate->cut_sheet(first_row_id, false, vertex_pool);

            if (_status == MS_alive && mate->_status == MS_alive) {
              // Now mate.  This will either succeed or fail.  It ought to
              // succeed, but if it doesn't, no harm done; it will simply
              // remove the common edge and return.  We'll go around again and
              // not encounter this neighbor next time.
              mate_pieces(*ei, *this, *mate, vertex_pool);
            }
          }
          if (not_any) {
            ++si;
          }
        }
        if (not_any) {
          ++ei;
        }
      }
    } while (!not_any);

    // All done.  Mark this one as down for the count.
    _row_id = -first_row_id;
  }
}




/**
 * Finds a neighboring strip and joins up with it to make a larger strip.
 * Returns true if mating was successful or at least possible, false if the
 * strip has no neighbors.
 */
bool EggMesherStrip::
mate(const EggVertexPool *vertex_pool) {
  // We must walk through the list of our neighbors and choose our best mate.
  nassertr(_status == MS_alive, false);

  EggMesherStrip *mate;
  EggMesherEdge *common_edge;

  if (!find_ideal_mate(mate, common_edge, vertex_pool)) {
    // We have no more eligible neighbors.  Call us done.
    _status = MS_done;

    return false;
  }

  nassertr(!mate->_prims.empty(), false);
  nassertr(!mate->_verts.empty(), false);

  mate_pieces(common_edge, *this, *mate, vertex_pool);

  // Whether the mate failed or not, the strip still (probably) has other
  // neighbors to consider.  Return true regardless.
  return true;
}

/**
 * Searches our neighbors for the most suitable mate.  Returns true if one is
 * found, false if we have no neighbors.
 */
bool EggMesherStrip::
find_ideal_mate(EggMesherStrip *&mate, EggMesherEdge *&common_edge,
                const EggVertexPool *vertex_pool) {
  Edges::iterator ei;

  mate = nullptr;
  common_edge = nullptr;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    EggMesherEdge::Strips &strips = (*ei)->_strips;
    EggMesherEdge::Strips::iterator si;
    for (si = strips.begin(); si != strips.end(); ++si) {
      if (*si != this) {
        if (mate==nullptr || pick_mate(**si, *mate, **ei, *common_edge,
                                    vertex_pool)) {
          mate = *si;
          common_edge = *ei;
        }
      }
    }
  }

  return (mate!=nullptr);
}




/**
 * Connects two pieces of arbitrary type, if possible.  Returns true if
 * successful, false if failure.
 */
bool EggMesherStrip::
mate_pieces(EggMesherEdge *common_edge, EggMesherStrip &front,
            EggMesherStrip &back, const EggVertexPool *vertex_pool) {
  nassertr(front._status == MS_alive, false);
  nassertr(back._status == MS_alive, false);
  nassertr(&front != &back, false);

  bool success = true;
  // remove_sides tracks whether we want to remove all but the leading edges
  // of the newly joined piece if we succeed.
  bool remove_sides = true;

  bool is_coplanar = front.is_coplanar_with(back, egg_coplanar_threshold);

  if (front._type == PT_tri && back._type == PT_tri) {

    if (is_coplanar && egg_retesselate_coplanar &&
        front._prims.front() == back._prims.front() &&
        convex_quad(common_edge, front, back, vertex_pool)) {

      // If we're joining two equivalent coplanar triangles, call it a quad.
      front._type = PT_quad;

      // We add one additional vertex for the new triangle, the one vertex we
      // didn't already share.
      int new_vert = back.find_uncommon_vertex(common_edge);

      // Now we just need to find the right place to insert it.  It belongs in
      // the middle of the common edge, i.e.  after the first vertex that is
      // on the common edge and before the second vertex.
      Verts::iterator a = front._verts.begin();
      Verts::iterator b = a;
      ++b;

      if (common_edge->contains_vertex(*a)) {
        if (common_edge->contains_vertex(*b)) {
          // It goes between a and b.
          front._verts.insert(b, new_vert);
        } else {
          // It goes at the end.
          front._verts.push_back(new_vert);
        }
      } else {
        // It goes between b and c.
        ++b;
        front._verts.insert(b, new_vert);
      }

      front._prims.splice(front._prims.end(), back._prims);
      back._verts.clear();

      // We leave all four surrounding edges for now, since the quad might
      // still be joined up in any direction.
      remove_sides = false;

    } else {
      // Otherwise, connect the two tris into a tristrip.
      front._type = PT_tristrip;

      int new_vert = back.find_uncommon_vertex(common_edge);
      front.rotate_to_back(common_edge);

      front._verts.push_back(new_vert);
      front._prims.splice(front._prims.end(), back._prims);
      back._verts.clear();
    }

  } else if ((front._type == PT_quad || front._type == PT_quadstrip) &&
             (back._type == PT_quad || back._type == PT_quadstrip)) {
    // Joining two quads, two quadstrips, or a quad and a quadstrip.  This
    // makes another quadstrip.

    // We expect this to succeed every time with quadstrips.
    success = mate_strips(common_edge, front, back, PT_quadstrip);

    if (!success) {
      // Although it might fail in rare circumstances (specifically, if the
      // two strips we attempted to join were backfacing to each other).  If
      // so, remove the adjoining edge so these two don't get tried again.
      common_edge->remove(&front);
      common_edge->remove(&back);
    }

  } else {

    // Otherwise.  This might be two tristrips, a quad and a tristrip, a
    // triangle and a quad, a triangle and a tristrip, a triangle and a
    // quadstrip, or a tristrip and a quadstrip.  In any case, we'll end up
    // with a tristrip.

    // This might fail if the tristrips don't match polarity.
    success = mate_strips(common_edge, front, back, PT_tristrip);

    if (!success) {
      // If it does fail, we'll try reversing the connection.  This makes
      // sense if we are joining a tri or tristrip to a quad or quadstrip,
      // which might fail in one direction but succeed in the other.
      success = mate_strips(common_edge, back, front, PT_tristrip);

      if (success) {
        // Yay!  Now return all the stuff to front.
        front._verts.splice(front._verts.end(), back._verts);
        front._prims.splice(front._prims.end(), back._prims);

      } else {
        // A miserable failure.  Never try to join these two again.
        common_edge->remove(&front);
        common_edge->remove(&back);
      }
    }
  }

  if (success) {
    front.combine_edges(back, remove_sides);
    if (!remove_sides) {
      // If we didn't want to remove the side edges, at least remove the join
      // edge, which is now internal.
      common_edge->remove(&front);
    }

    nassertr(back._prims.empty(), false);
    nassertr(back._verts.empty(), false);

    // Strip back is no more.
    back._status = MS_dead;

    // The result is planar if and only if we joined two coplanar pieces.
    front._planar = is_coplanar;
    front._origin = MO_mate;
  }

  return success;
}

/**
 * Stitches two strips together, producing in "front" a new strip of the
 * indicated type (quadstrip or tristrip).  The front strip stores the result,
 * and the back strip is emptied on success.
 *
 * Returns true if successful, false if failure (generally because of
 * incorrect polarity of tristrips), in which case nothing has changed (or at
 * least, not much).
 */
bool EggMesherStrip::
mate_strips(EggMesherEdge *common_edge, EggMesherStrip &front,
            EggMesherStrip &back, EggMesherStrip::PrimType type) {
  // We don't allow making odd-length strips at all.  Odd-length strips can't
  // be rotated if they're flat-shaded, and they can't be joined end-to-end
  // using degenerate triangles.  So forget 'em.

  // This might not be the right place to impose this rule, because it tends
  // to end up with lots of independent triangles in certain kinds of meshes,
  // but it's the easiest place to impose it.
  if ((front._type != PT_tri && back._type == PT_tri) ||
      (front._type == PT_tri && back._type != PT_tri) ||
      (front._type == PT_tristrip && back._type == PT_tristrip &&
       ((front._verts.size() + back._verts.size()) & 1) != 0)) {
    return false;
  }

  // If we start with a quad or tri, rotate the vertices around so we start
  // with the common edge.
  if (front._type == PT_tri || front._type == PT_quad) {
    front.rotate_to_back(common_edge);
  }
  if (back._type == PT_tri || back._type == PT_quad) {
    back.rotate_to_front(common_edge);
  }

  bool reverse_front = common_edge->matches(front.get_head_edge());
  bool reverse_back = !common_edge->matches(back.get_head_edge());

  bool invert_front = false;
  bool invert_back = false;

  if (reverse_front && front.is_odd()) {
    // If we're going to reverse the front strip, we have to be careful.  This
    // will also reverse the facing direction if it has an odd number of
    // prims.
    if (!front.can_invert()) {
      return false;
    }
    invert_front = true;
  }

  if (must_invert(front, back, reverse_back, type)) {
    if (!back.can_invert()) {
      return false;
    }
    invert_back = true;
    back.invert();
  }

  if (invert_front) {
    front.invert();
  }

  if (reverse_front) {
    reverse(front._verts.begin(), front._verts.end());
    reverse(front._prims.begin(), front._prims.end());
  }

  if (reverse_back) {
    reverse(back._verts.begin(), back._verts.end());
    reverse(back._prims.begin(), back._prims.end());
  }

  bool will_reverse = front.would_reverse_tail(type);
  bool is_headtotail = (front.get_tail_edge() == back.get_head_edge());
  if (will_reverse == is_headtotail) {
    // Oops, we tried to join two backfacing strips.  This really shouldn't
    // happen, but it occasionally does for some mysterious reason.  Maybe one
    // day I'll understand why.  In the meantime, just recover and carry on.
    if (reverse_back) {
      reverse(back._verts.begin(), back._verts.end());
      reverse(back._prims.begin(), back._prims.end());
    }
    if (invert_back) {
      back.invert();
    }
    if (reverse_front) {
      reverse(front._verts.begin(), front._verts.end());
      reverse(front._prims.begin(), front._prims.end());
    }
    if (invert_front) {
      front.invert();
    }
    return false;
  }

  front.convert_to_type(type);
  back.convert_to_type(type);

  /*
  if (! (front.get_tail_edge() == back.get_head_edge()) ) {
    builder_cat.error()
         << "\nFailure, trying to connect " << front
         << "\nto " << back
         << "\nreverse_front = " << reverse_front
         << " reverse_back = " << reverse_back
         << " invert_front = " << invert_front
         << "\n";
    Edges::iterator ei;

    nout << "\nFront edges:\n";
    for (ei = front._edges.begin(); ei != front._edges.end(); ++ei) {
      nout << **ei << "\n";
    }

    nout << "\nBack edges:\n";
    for (ei = back._edges.begin(); ei != back._edges.end(); ++ei) {
      nout << **ei << "\n";
    }
  }
  */

  // If this assertion fails, we were misinformed about our ability to join
  // these two strips.  Either the must_invert() call returned the incorrect
  // value, or our edge-detection logic failed and we attempted to join two
  // oppositely-facing strips.  nassertr(front.get_tail_edge() ==
  // back.get_head_edge(), false);

  front._verts.pop_back();
  front._verts.pop_back();
  front._verts.splice(front._verts.end(), back._verts);
  front._prims.splice(front._prims.end(), back._prims);

  return true;
}

/**
 * Returns false if the strips can be mated as they currently are.  Returns
 * true if the back strip must be inverted first.
 */
bool EggMesherStrip::
must_invert(const EggMesherStrip &front, const EggMesherStrip &back,
            bool will_reverse_back, EggMesherStrip::PrimType type) {
  bool invert = false;

  if ((front._type == PT_quad || front._type == PT_quadstrip) &&
      type == PT_tristrip) {
    // If we'll be converting from quads to tris, the tail edge of the front
    // strip will always be even.

  } else if (front.is_odd()) {
    // Otherwise, we have to flip if the tail edge is odd.
    invert = !invert;
  }

  if (will_reverse_back) {
    // With the back strip, we don't care about what will happen to its tail
    // edge when we convert it, but we do care what happens to its front edge
    // if we reverse it.
    if (back.is_odd()) {
      // Specifically, the front edge will be reversed when the strip is
      // reversed only if the strip is odd.
      invert = !invert;
    }
  }

  return invert;
}

/**
 * Returns true if the quad that would be formed by connecting coplanar tris
 * front and back along common_edge is convex, false otherwise.
 */
bool EggMesherStrip::
convex_quad(EggMesherEdge *common_edge, EggMesherStrip &front,
            EggMesherStrip &back, const EggVertexPool *vertex_pool) {
  // Find the edge from the apex of one triangle to the apex of the other.
  // This is the "other" diagonal of the quad-to-be, other than the
  // common_edge.
  int vi_a = front.find_uncommon_vertex(common_edge);
  int vi_b = back.find_uncommon_vertex(common_edge);
  nassertr(vi_a >= 0 && vi_b >= 0, false);

  LPoint3d a3, b3, c3, d3;
  a3 = vertex_pool->get_vertex(vi_a)->get_pos3();
  b3 = vertex_pool->get_vertex(vi_b)->get_pos3();

  c3 = vertex_pool->get_vertex(common_edge->_vi_a)->get_pos3();
  d3 = vertex_pool->get_vertex(common_edge->_vi_b)->get_pos3();

  // Project both edges into the 2-d axis plane most nearly perpendicular to
  // the normal.  We're assuming both tris have the same normal.

  nassertr(front._planar, false);

  LNormald n(front._plane_normal);
  int xi, yi;

  // Find the largest dimension of the normal.
  if (fabs(n[0]) > fabs(n[1])) {
    if (fabs(n[0]) > fabs(n[2])) {
      xi = 1;
      yi = 2;
    } else {
      xi = 0;
      yi = 1;
    }
  } else {
    if (fabs(n[1]) > fabs(n[2])) {
      xi = 0;
      yi = 2;
    } else {
      xi = 0;
      yi = 1;
    }
  }

  LVecBase2d a2, b2, c2, d2;
  a2.set(a3[xi], a3[yi]);
  b2.set(b3[xi], b3[yi]);
  c2.set(c3[xi], c3[yi]);
  d2.set(d3[xi], d3[yi]);

  // Now (c2-d2) is the common edge, and (a2-b2) is the new edge.  The quad is
  // convex iff (c2-d2) intersects (a2-b2).  We actually only need to test
  // whether (c2-d2) intersects the infinite line passing through (a2-b2).

  // The equation for the infinite line containing (a2-b2): Ax + By + C = 0
  double A = (b2[1] - a2[1]);
  double B = (a2[0] - b2[0]);
  double C = -(A*b2[0] + B*b2[1]);

  // The parametric equations for the line segment (c2-d2): x = c2[0] +
  // (d2[0]-c2[0])t y = c2[1] + (d2[1]-c2[1])t

  // Solved for t:
  double t = - ((A*c2[0] + B*c2[1]) + C) / (A*(d2[0]-c2[0]) + B*(d2[1]-c2[1]));

  // Now the lines intersect if t is in [0, 1].
  return (0.0 <= t && t <= 1.0);
}

/**
 * Returns the number of neighbors the strip shares.
 */
int EggMesherStrip::
count_neighbors() const {
  int count = 0;
  Edges::const_iterator ei;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    count += (*ei)->_strips.size();
  }
  return count;
}

/**
 * Writes all the neighbor indexes to the ostream.
 */
void EggMesherStrip::
output_neighbors(std::ostream &out) const {
  Edges::const_iterator ei;
  EggMesherEdge::Strips::const_iterator si;

  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    for (si = (*ei)->_strips.begin();
         si != (*ei)->_strips.end();
         ++si) {
      out << " " << (*si)->_index;
    }
  }
}

/**
 * Returns the first vertex found that is not shared by the given edge.
 */
int EggMesherStrip::
find_uncommon_vertex(const EggMesherEdge *edge) const {
  int vi_a = edge->_vi_a;
  int vi_b = edge->_vi_b;

  Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    EggMesherEdge *e = (*ei);

    if (e->_vi_a != vi_a && e->_vi_a != vi_b) {
      return e->_vi_a;
    } else if (e->_vi_b != vi_a && e->_vi_b != vi_b) {
      return e->_vi_b;
    }
  }

  return -1;
}

/**
 * Returns the first edge found that does not contain the given vertex.  In a
 * tri, this will be the edge opposite the given vertex.
 */
const EggMesherEdge *EggMesherStrip::
find_opposite_edge(int vi) const {
  Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    EggMesherEdge *e = (*ei);
    if (!e->contains_vertex(vi)) {
      return e;
    }
  }

  return nullptr;
}

/**
 * Returns the first edge found that shares no vertices with the given edge.
 * In a quad, this will be the edge opposite the given edge.
 */
const EggMesherEdge *EggMesherStrip::
find_opposite_edge(const EggMesherEdge *edge) const {
  int vi_a = edge->_vi_a;
  int vi_b = edge->_vi_b;

  Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    EggMesherEdge *e = (*ei);
    if (!e->contains_vertex(vi_a) && !e->contains_vertex(vi_b)) {
      return e;
    }
  }

  return nullptr;
}

/**
 * Returns the first edge found that shares exactly one vertex with the given
 * edge.  In a quad, this will be one of two edges adjacent to the given edge.
 */
const EggMesherEdge *EggMesherStrip::
find_adjacent_edge(const EggMesherEdge *edge) const {
  int vi_a = edge->_vi_a;
  int vi_b = edge->_vi_b;

  Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    EggMesherEdge *e = (*ei);
    if (e->contains_vertex(vi_a) != e->contains_vertex(vi_b)) {
      return e;
    }
  }

  return nullptr;
}

/**
 * Rotates a triangle or quad so that the given edge is first in the vertex
 * list.
 */
void EggMesherStrip::
rotate_to_front(const EggMesherEdge *edge) {
  int vi_a = edge->_vi_a;
  int vi_b = edge->_vi_b;

  // See if we're already there.
  if (_verts.front() == vi_a || _verts.front() == vi_b) {
    Verts::iterator vi = _verts.begin();
    ++vi;
    if (*vi == vi_a || *vi == vi_b) {
      // Yes!
      return;
    }

    // No, we must be right on the line.  Roll back one.
    rotate_back();

  } else {
    // Roll forward until it comes into view.
    int num_verts = _verts.size();
    while (_verts.front() != vi_a && _verts.front() != vi_b) {
      // Make sure either vertex exists.
      num_verts--;
      nassertv(num_verts > 0);
      rotate_forward();
    }
  }

#ifndef NDEBUG
  // Now make sure the edge actually exists.
  Verts::iterator vi = _verts.begin();
  ++vi;
  nassertv(*vi == vi_a || *vi == vi_b);
#endif
}

/**
 * Rotates a triangle or quad so that the given edge is last in the vertex
 * list.
 */
void EggMesherStrip::
rotate_to_back(const EggMesherEdge *edge) {
  int vi_a = edge->_vi_a;
  int vi_b = edge->_vi_b;

  // See if we're already there.
  if (_verts.back() == vi_a || _verts.back() == vi_b) {
    Verts::reverse_iterator vi = _verts.rbegin();
    ++vi;
    if (*vi == vi_a || *vi == vi_b) {
      // Yes!
      return;
    }

    // No, we must be right on the line.  Roll forward one.
    rotate_forward();

  } else {
    // Roll backward until it comes into view.
    int num_verts = _verts.size();
    while (_verts.back() != vi_a && _verts.back() != vi_b) {
      // Make sure either vertex exists.
      num_verts--;
      nassertv(num_verts > 0);
      rotate_back();
    }
  }

#ifndef NDEBUG
  // Now make sure the edge actually exists.
  Verts::reverse_iterator vi = _verts.rbegin();
  ++vi;
  nassertv(*vi == vi_a || *vi == vi_b);
#endif
}


/**
 * Returns true if the strip can be inverted (reverse its facing direction).
 * Generally, this is true for quadstrips and false for tristrips.
 */
bool EggMesherStrip::
can_invert() const {
  return (_type == PT_quadstrip || _type == PT_quad);
}

/**
 * Reverses the facing of a quadstrip by reversing pairs of vertices.  Returns
 * true if successful, false if failure (for instance, on a tristrip).
 */
bool EggMesherStrip::
invert() {
  if (!can_invert()) {
    return false;
  }
  Verts::iterator vi, vi2;
  vi = _verts.begin();
  while (vi != _verts.end()) {
    vi2 = vi;
    ++vi2;
    nassertr(vi2 != _verts.end(), false);

    // Exchange vi and vi2
    int t = *vi2;
    *vi2 = *vi;
    *vi = t;

    // Increment
    vi = vi2;
    ++vi;
  }
  return true;
}

/**
 * Returns true if the tristrip or quadstrip contains an odd number of pieces.
 */
bool EggMesherStrip::
is_odd() const {
  if (_type == PT_quadstrip || _type == PT_quad) {
    // If a quadstrip has a multiple of four vertices, it has an odd number of
    // quads.
    return (_verts.size() % 4 == 0);
  } else {
    // If a tristrip has an odd number of vertices, it has an odd number of
    // tris.
    return (_verts.size() % 2 == 1);
  }
}

/**
 * Returns true if convert_to_type() would reverse the tail edge of the given
 * strip, false otherwise.
 */
bool EggMesherStrip::
would_reverse_tail(EggMesherStrip::PrimType want_type) const {
  bool reverse = false;

  if (_type == want_type) {
    return false;
  }
  if (want_type == PT_tristrip) {
    switch (_type) {
    case PT_tri:
    case PT_tristrip:
      break;

    case PT_quad:
    case PT_quadstrip:
      // When we convert a quadstrip to a tristrip, we reverse the tail edge
      // if we have a multiple of four verts.
      reverse = (_verts.size() % 4 == 0);
      break;

    default:
      egg_cat.fatal() << "Invalid conversion!\n";
      abort();
      break;
    }

  } else if (want_type == PT_quadstrip) {
    switch (_type) {
    case PT_quad:
    case PT_quadstrip:
      break;

    case PT_tri:
    case PT_tristrip:
      // We don't convert tristrips to quadstrips; fall through.

    default:
      egg_cat.fatal() << "Invalid conversion!\n";
      abort();
      break;
    }
  }

  return reverse;
}


/**
 * Converts the EggMesherStrip from whatever form it is--triangle, quad, or
 * quadstrip--into a tristrip or quadstrip.
 */
void EggMesherStrip::
convert_to_type(EggMesherStrip::PrimType want_type) {
  Verts::iterator vi, vi2;
  int even;

  if (_type == want_type) {
    return;
  }
  if (want_type == PT_tristrip) {
    switch (_type) {
    case PT_tri:
    case PT_tristrip:
      break;

    case PT_quad:
    case PT_quadstrip:
      // To convert from quadquadstrip to tristrip, we reverse every other
      // pair of vertices.

      vi = _verts.begin();
      even = 0;
      while (vi != _verts.end()) {
        vi2 = vi;
        ++vi2;
        nassertv(vi2 != _verts.end());

        // vi and vi2 are a pair.  Should we reverse them?
        if (even) {
          int t = *vi2;
          *vi2 = *vi;
          *vi = t;
        }

        // Increment
        vi = vi2;
        ++vi;
        even = !even;
      }
      break;

    default:
      egg_cat.fatal() << "Invalid conversion!\n";
      abort();
    }

  } else if (want_type == PT_quadstrip) {
    switch (_type) {
    case PT_quad:
    case PT_quadstrip:
      break;

    case PT_tri:
    case PT_tristrip:
      // We don't convert tristrips to quadstrips; fall through.

    default:
      egg_cat.fatal() << "Invalid conversion!\n";
      abort();
    }
  }

  _type = want_type;
}

/**
 * Removes the edges from the given strip and appends them to our own.  If
 * remove_sides is true, then removes all the edges except the head and the
 * tail.
 */
void EggMesherStrip::
combine_edges(EggMesherStrip &other, int remove_sides) {
  Edges::iterator ei;
  for (ei = other._edges.begin(); ei != other._edges.end(); ++ei) {
    (*ei)->change_strip(&other, this);
  }

  _edges.splice(_edges.end(), other._edges);

  if (remove_sides) {
    // Identify the head and tail edges so we can remove everything else.
    EggMesherEdge head = get_head_edge();
    EggMesherEdge tail = get_tail_edge();

    if (!is_odd()) {
      // If the strip is odd, its true tail edge is the inverse of its actual
      // edge.
      tail = EggMesherEdge(tail._vi_b, tail._vi_a);
    }

    Edges junk_edges;

    Edges::iterator next_ei;
    ei = _edges.begin();
    while (ei != _edges.end()) {
      next_ei = ei;
      ++next_ei;

      // Is this edge to be saved or is it fodder?
      if (!(**ei == head) && !(**ei == tail)) {
        // Fodder!  But we can't remove it right away, because this will upset
        // the current list; instead, we'll splice it to junk_edges.
        junk_edges.splice(junk_edges.end(), _edges, ei);
      }
      ei = next_ei;
    }

    // Now we can safely remove all the to-be-junked edges.
    for (ei = junk_edges.begin(); ei != junk_edges.end(); ++ei) {
      (*ei)->remove(this);
    }
  }
}


/**
 * Removes all active edges from the strip.  This effectively renders it
 * ineligible to mate with anything else.
 */
void EggMesherStrip::
remove_all_edges() {
  // First, move all the edges to a safe place so we can traverse the list
  // without it changing on us.
  Edges junk_edges;
  junk_edges.splice(junk_edges.end(), _edges);

  // Now we can safely remove all the to-be-junked edges.
  Edges::iterator ei;
  for (ei = junk_edges.begin(); ei != junk_edges.end(); ++ei) {
    (*ei)->remove(this);
  }
}

/**
 * Defines an ordering to select neighbors to mate with.  This compares strip
 * a with strip b and returns true if strip a is the preferable choice, false
 * if strip b.
 */
bool EggMesherStrip::
pick_mate(const EggMesherStrip &a_strip, const EggMesherStrip &b_strip,
          const EggMesherEdge &a_edge, const EggMesherEdge &b_edge,
          const EggVertexPool *vertex_pool) const {
  // First, try to avoid polluting quads, quadstrips, and tristrips with
  // arbitrary triangles.  When we mate a tri or tristrip to a quadstrip, we
  // end up with a tristrip that may be less versatile than the original
  // quadstrip.  Better to avoid this if we can.  Try to choose a mate that
  // more closely matches our own type.
  int a_cat = a_strip.type_category();
  int b_cat = b_strip.type_category();
  if (a_cat != b_cat) {
    int me_cat = type_category();
    return abs(a_cat - me_cat) < abs(b_cat - me_cat);
  }

  // Now, if we're connecting two tris, try to connect them up so they make
  // good quads.
  if (_type == PT_tri && a_strip._type == PT_tri &&
      b_strip._type == PT_tri) {

    // This will depend on both coplanarity and edge length.  We can't use
    // just one or the other, because some tris are nearly isosceles, and some
    // have more than one coplanar neighbor.  Hopefully the combination of
    // both factors will zero us in on the correct neighbor first.

    double a_coplanar = coplanarity(a_strip);
    double b_coplanar = coplanarity(b_strip);
    double coplanar_diff = a_coplanar - b_coplanar;

    double a_length = a_edge.compute_length(vertex_pool);
    double b_length = b_edge.compute_length(vertex_pool);
    double length_diff = (a_length - b_length) / (a_length + b_length);

    // These weights were chosen empirically to yield fairly good results.
    double sum = 4.0 * coplanar_diff - 1.0 * length_diff;
    return sum < 0;
  }

  // Then, get the smallest strip.
  if (a_strip._prims.size() != b_strip._prims.size()) {
    return a_strip._prims.size() < b_strip._prims.size();
  }

  // Finally, get the strip with the fewest neighbors.
  return a_strip.count_neighbors() < b_strip.count_neighbors();
}

/**
 * Defines an ordering to select neighbors to follow when measuring out a
 * quadsheet.  This is only called when three or more prims share a single
 * edge, which should be rarely--generally only when coplanar polys are going
 * on.
 */
bool EggMesherStrip::
pick_sheet_mate(const EggMesherStrip &a_strip,
                const EggMesherStrip &b_strip) const {
  // First, try to get the poly which is closest to our own normal.
  if (_planar && a_strip._planar && b_strip._planar) {
    double a_diff = dot(LNormald(_plane_normal), LNormald(a_strip._plane_normal));
    double b_diff = dot(LNormald(_plane_normal), LNormald(b_strip._plane_normal));

    if (fabs(a_diff - b_diff) > 0.0001) {
      return a_diff > b_diff;
    }
  }

  // Then, pick the one that's most like our own type.
  int a_cat = a_strip.type_category();
  int b_cat = b_strip.type_category();
  if (a_cat != b_cat) {
    int me_cat = type_category();
    return abs(a_cat - me_cat) < abs(b_cat - me_cat);
  }

  // Oh, just pick any old one.
  return false;
}

/**
 * Formats the vertex for output in some sensible way.
 */
void EggMesherStrip::
output(std::ostream &out) const {
  switch (_status) {
  case MS_alive:
    break;

  case MS_dead:
    out << "Dead ";
    break;

  case MS_done:
    out << "Done ";
    break;

  default:
    out << "Unknown status ";
  }

  switch (_type) {
  case PT_tri:
    out << "Tri";
    break;

  case PT_quad:
    out << "Quad";
    break;

  case PT_tristrip:
    out << "TriStrip";
    break;

  case PT_trifan:
    out << "TriFan";
    break;

  case PT_quadstrip:
    out << "QuadStrip";
    break;

  default:
    out << "Unknown";
  }

  if (_planar) {
    out << " (planar)";
  }

  out << " " << _index << " [";

  Verts::const_iterator vi;
  for (vi = _verts.begin(); vi != _verts.end(); vi++) {
    out << " " << *vi;
  }
  out << " ]: " << _prims.size()
      << " prims, " << count_neighbors() << " neighbors";

  output_neighbors(out);

  out << " edges";
  Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ei++) {
    out << " " << (void *)(*ei);
  }

  out << ".";
}
