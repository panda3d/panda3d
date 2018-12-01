/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherFanMaker.cxx
 * @author drose
 * @date 2005-03-22
 */

#include "eggMesherFanMaker.h"
#include "eggMesher.h"
#include "eggPolygon.h"
#include "eggGroupNode.h"

/**
 *
 */
EggMesherFanMaker::
EggMesherFanMaker(int vertex, EggMesherStrip *tri,
                  EggMesher *mesher) {
  _vertex = vertex;
  const EggMesherEdge *edge = tri->find_opposite_edge(vertex);
  if (edge != nullptr) {
    _edges.push_back(edge);
  }
  _strips.push_back(tri);
  _planar = tri->_planar;
  _mesher = mesher;
}

/**
 *
 */
EggMesherFanMaker::
EggMesherFanMaker(const EggMesherFanMaker &copy) :
  _vertex(copy._vertex),
  _edges(copy._edges),
  _strips(copy._strips),
  _planar(copy._planar),
  _mesher(copy._mesher)
{
}

/**
 *
 */
void EggMesherFanMaker::
operator = (const EggMesherFanMaker &copy) {
  _vertex = copy._vertex;
  _edges = copy._edges;
  _strips = copy._strips;
  _planar = copy._planar;
  _mesher = copy._mesher;
}

/**
 * Attempts to connect two fans end-to-end.  They must both share the same
 * common vertex and a common edge.
 *
 * The return value is true if the fans were successfully joined, or false if
 * they could not be.
 */
bool EggMesherFanMaker::
join(EggMesherFanMaker &other) {
  nassertr(_vertex == other._vertex, false);
  nassertr(_mesher == other._mesher, false);

  nassertr(!_edges.empty() && !other._edges.empty(), false);

  const EggMesherEdge *my_back = _edges.back();
  const EggMesherEdge *other_front = other._edges.front();
  nassertr(my_back != nullptr &&
           other_front != nullptr, false);

  int my_back_b = my_back->_vi_b;
  int other_front_a = other_front->_vi_a;

  if (my_back_b == other_front_a) {
    _planar = is_coplanar_with(other);
    _edges.splice(_edges.end(), other._edges);
    _strips.splice(_strips.end(), other._strips);
    return true;
  }

  const EggMesherEdge *my_front = _edges.front();
  const EggMesherEdge *other_back = other._edges.back();
  nassertr(my_front != nullptr &&
           other_back != nullptr, false);

  int my_front_a = my_front->_vi_a;
  int other_back_b = other_back->_vi_b;

  if (my_front_a == other_back_b) {
    _planar = is_coplanar_with(other);
    _edges.splice(_edges.begin(), other._edges);
    _strips.splice(_strips.begin(), other._strips);
    return true;
  }

  return false;
}

/**
 * Returns the overall angle subtended by the fan, from the leading edge to
 * the trailing edge, in degrees.
 */
double EggMesherFanMaker::
compute_angle() const {
  // We sum up the angles of each triangle.  This is more correct than taking
  // the net angle from the first edge to the last (since we may not be in a
  // plane).
  nassertr(is_valid(), 0.0);

  EggVertexPool *vertex_pool = _mesher->_vertex_pool;

  double angle = 0.0;
  LPoint3d v0 = vertex_pool->get_vertex(_vertex)->get_pos3();

  Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    LVector3d v1 = vertex_pool->get_vertex((*ei)->_vi_a)->get_pos3() - v0;
    LVector3d v2 = vertex_pool->get_vertex((*ei)->_vi_b)->get_pos3() - v0;

    v1 = normalize(v1);
    v2 = normalize(v2);
    angle += acos(dot(v1, v2));
  }

  return rad_2_deg(angle);
}

/**
 * Begins the fanning process.  Searches for triangles and connects them into
 * a fan.
 *
 * In certain cases, if egg_unroll_fans is enabled, the resulting fan may be
 * retesselated into a series of zig-zag triangles, which are stored in
 * unrolled_tris.  Otherwise, an EggMesherStrip (representing the fan) is
 * created and added to the mesher.
 *
 * The return value is (loosely) the number of primitives created.
 */
int EggMesherFanMaker::
build(EggGroupNode *unrolled_tris) {
  nassertr(_edges.size() == _strips.size(), 0);

  int num_tris = _edges.size();
  double net_angle = compute_angle();
  double avg_angle = net_angle / (double)num_tris;

  if (avg_angle > egg_max_tfan_angle) {
    // The triangles are too loose to justify making a fan; it'll probably
    // make a better quadsheet.
    return 0;
  }

  if (egg_min_tfan_tris == 0 || num_tris < egg_min_tfan_tris) {
    // Oops, not enough triangles to justify a fan.
    if (!egg_unroll_fans) {
      return 0;
    }

    // However, we could (maybe) make it a few tristrips!

    // Each section of the fan which is made up of coplanar tris, with
    // identical properties, may be retesselated into a tristrip.  What a
    // sneaky trick!  To do this, we must first identify each such qualifying
    // section.

    // We define a seam as the edge between any two tris which are noncoplanar
    // or have different properties.  Then we can send each piece between the
    // seams to unroll().

    Strips::iterator si, last_si;
    Edges::iterator ei, last_ei;

    // First, rotate the fan so it begins at a seam.  We do this so we won't
    // be left out with part of one piece at the beginning and also at the
    // end.
    si = _strips.begin();
    last_si = si;
    ei = _edges.begin();
    last_ei = ei;
    int found_seam = false;

    for (++si, ++ei; si != _strips.end() && !found_seam; ++si, ++ei) {
      nassertr(ei != _edges.end(), 0);
      if ( ((*si)->_prims.front()->compare_to(*(*last_si)->_prims.front()) != 0) ||
           !(*si)->is_coplanar_with(*(*last_si), egg_coplanar_threshold)) {
        // Here's a seam.  Break the fan here.
        found_seam = true;
        _edges.splice(_edges.begin(), _edges, ei, _edges.end());
        _strips.splice(_strips.begin(), _strips, si, _strips.end());
      }
    }

    // Now break the fan up along its seams and unroll each piece separately.
    si = _strips.begin();
    last_si = si;
    ei = _edges.begin();
    last_ei = ei;

    int count = 0;
    for (++si, ++ei; si != _strips.end(); ++si, ++ei) {
      nassertr(ei != _edges.end(), 0);
      if ( ((*si)->_prims.front()->compare_to(*(*last_si)->_prims.front()) != 0) ||
           !(*si)->is_coplanar_with(*(*last_si), egg_coplanar_threshold)) {
        // Here's the end of a run of matching pieces.
        count += unroll(last_si, si, last_ei, ei, unrolled_tris);
        last_si = si;
        last_ei = ei;
      }
    }
    count += unroll(last_si, si, last_ei, ei, unrolled_tris);

    return count;

  } else {
    EggMesherStrip new_fan(EggMesherStrip::PT_trifan, EggMesherStrip::MO_fanpoly);
    new_fan._verts.push_back(_vertex);

    new_fan._verts.push_back(_edges.front()->_vi_a);
    Edges::iterator ei;
    for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
      new_fan._verts.push_back((*ei)->_vi_b);
    }

    Strips::iterator si;
    for (si = _strips.begin(); si != _strips.end(); ++si) {
      new_fan._prims.splice(new_fan._prims.end(), (*si)->_prims);
      (*si)->remove_all_edges();
      (*si)->_verts.clear();
      (*si)->_status = EggMesherStrip::MS_dead;
    }

    // If we'd built our list of edges and strips right, this sum should come
    // out so that there are two more vertices than triangles in the new fan.
    nassertr(new_fan._verts.size() == new_fan._prims.size() + 2, 0);

    // Now we've built a fan, and it won't be able to mate with anything else,
    // so add it to the done list.
    _mesher->_done.push_back(new_fan);
  }

  return 1;
}

/**
 * Unrolls a planar subset of the current working fan, defined by the given
 * iterators, into a series of triangles that zig-zag back and forth for
 * better tristripping properties.  The new triangles are added to
 * unrolled_tris; the return value is 1 if successful, or 0 otherwise.
 */
int EggMesherFanMaker::
unroll(Strips::iterator strip_begin, Strips::iterator strip_end,
       Edges::iterator edge_begin, Edges::iterator edge_end,
       EggGroupNode *unrolled_tris) {
  Edges::iterator ei;
  Strips::iterator si;

  int num_tris = 0;
  for (ei = edge_begin; ei != edge_end; ++ei) {
    num_tris++;
  }

  if (num_tris < 3) {
    // Don't even bother.
    return 0;
  }

  PT(EggPolygon) poly = new EggPolygon;

  // Now we build an n-sided polygon the same shape as the fan.  We'll
  // decompose it into tris in a second.
  poly->copy_attributes(*(*strip_begin)->_prims.front());
  EggVertexPool *vertex_pool = _mesher->_vertex_pool;

  ei = edge_end;
  --ei;
  if ((*ei)->_vi_b != (*edge_begin)->_vi_a) {
    // If the fan is less than a full circle, we need to keep the hub vertex
    // and initial vertex in the poly.  Otherwise, we'll discard them.
    poly->add_vertex(vertex_pool->get_vertex(_vertex));
    poly->add_vertex(vertex_pool->get_vertex((*edge_begin)->_vi_a));
  }

  for (ei = edge_begin; ei != edge_end; ++ei) {
    poly->add_vertex(vertex_pool->get_vertex((*ei)->_vi_b));
  }

  bool result = true;

  if (egg_show_quads) {
    // If we're showing quads, also show retesselated triangles.

    // We can't add it directly to the mesher, that's unsafe; instead, we'll
    // just add it to the end of the unrolled_tris list.  This does mean we
    // won't be able to color it a fancy color, but too bad.
    // _mesher->add_polygon(poly, EggMesherStrip::MO_fanpoly);
    unrolled_tris->add_child(poly);

  } else {
    // Now decompose the new polygon into triangles.
    result = poly->triangulate_into(unrolled_tris, true);
  }

  if (result) {
    // Now that we've created a new poly, kill off all the old ones.
    for (si = strip_begin; si != strip_end; ++si) {
      (*si)->remove_all_edges();
      (*si)->_verts.clear();
      (*si)->_prims.clear();
      (*si)->_status = EggMesherStrip::MS_dead;
    }
    return 1;
  } else {
    return 0;
  }
}

/**
 *
 */
void EggMesherFanMaker::
output(std::ostream &out) const {
  out << _vertex << ":[";
  if (!_edges.empty()) {
    Edges::const_iterator ei;
    for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
      out << " " << (*ei)->_vi_a;
    }
    out << " " << _edges.back()->_vi_b;
  }
  out << " ]";

  if (_planar) {
    out << " (planar)";
  }
}
