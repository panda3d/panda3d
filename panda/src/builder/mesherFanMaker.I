// Filename: mesherFanMaker.I
// Created by:  drose (21Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

/* okcircular */
#include "builderFuncs.h"

#include <notify.h>
#include <mathNumbers.h>

#include <math.h>

template <class PrimType>
INLINE bool MesherFanMaker<PrimType>::
operator < (const MesherFanMaker &other) const {
  return _edges.front() < other._edges.front();
}

template <class PrimType>
INLINE bool MesherFanMaker<PrimType>::
operator != (const MesherFanMaker &other) const {
  return !operator == (other);
}

template <class PrimType>
INLINE bool MesherFanMaker<PrimType>::
operator == (const MesherFanMaker &other) const {
  return _edges.front() == other._edges.front();
}

template <class PrimType>
INLINE bool MesherFanMaker<PrimType>::
is_empty() const {
  return (_edges.empty());
}

template <class PrimType>
INLINE bool MesherFanMaker<PrimType>::
is_valid() const {
  return (_edges.size() > 2);
}

////////////////////////////////////////////////////////////////////
//     Function: MesherFanMaker::is_coplanar_with
//       Access: Public
//  Description: Returns true if the strip and the other strip are
//               coplanar.
////////////////////////////////////////////////////////////////////
template <class PrimType>
INLINE bool MesherFanMaker<PrimType>::
is_coplanar_with(const MesherFanMaker &other) const {
  return _planar && other._planar &&
    _strips.front()->is_coplanar_with(*other._strips.front(),
                                    _bucket->_coplanar_threshold);
}

template <class PrimType>
MesherFanMaker<PrimType>::
MesherFanMaker(const Vertex *vertex, Strip *tri, Mesher *mesher) {
  _vertex = vertex;
  const Edge *edge = tri->find_opposite_edge(vertex);
  if (edge != (const Edge *)NULL) {
    _edges.push_back(edge);
  }
  _strips.push_back(tri);
  _planar = tri->_planar;
  _mesher = mesher;
  _bucket = _mesher->_bucket;
}

template <class PrimType>
bool MesherFanMaker<PrimType>::
join(MesherFanMaker &other) {
  nassertr(_vertex == other._vertex, false);
  nassertr(_mesher == other._mesher, false);
  nassertr(_bucket == other._bucket, false);

  nassertr(!_edges.empty() && !other._edges.empty(), false);

  const Edge *my_back = _edges.back();
  const Edge *other_front = other._edges.front();
  nassertr(my_back != (Edge *)NULL && other_front != (Edge *)NULL, false);

  const Vertex *my_back_b = my_back->_b;
  const Vertex *other_front_a = other_front->_a;

  if (my_back_b == other_front_a) {
    _planar = is_coplanar_with(other);
    _edges.splice(_edges.end(), other._edges);
    _strips.splice(_strips.end(), other._strips);
    return true;
  }

  const Edge *my_front = _edges.front();
  const Edge *other_back = other._edges.back();
  nassertr(my_front != (Edge *)NULL && other_back != (Edge *)NULL, false);

  const Vertex *my_front_a = my_front->_a;
  const Vertex *other_back_b = other_back->_b;

  if (my_front_a == other_back_b) {
    _planar = is_coplanar_with(other);
    _edges.splice(_edges.begin(), other._edges);
    _strips.splice(_strips.begin(), other._strips);
    return true;
  }

  return false;
}


template <class PrimType>
float MesherFanMaker<PrimType>::
compute_angle() const {
  // We sum up the angles of each triangle.  This is more correct than
  // taking the net angle from the first edge to the last (since we
  // may not be in a plane).
  nassertr(is_valid(), 0.0);

  double angle = 0.0;
  Vertexf v0 = _vertex->get_coord_value(*_bucket);

  TYPENAME Edges::const_iterator ei;
  for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
    Normalf v1 = (*ei)->_a->get_coord_value(*_bucket) - v0;
    Normalf v2 = (*ei)->_b->get_coord_value(*_bucket) - v0;

    v1 = normalize(v1);
    v2 = normalize(v2);
    angle += acos(dot(v1, v2));
  }

  return rad_2_deg(angle);
}

template <class PrimType>
int MesherFanMaker<PrimType>::
build(pvector<Prim> &unrolled_tris) {
  nassertr(_edges.size() == _strips.size(), 0);

  int num_tris = _edges.size();
  float net_angle = compute_angle();
  float avg_angle = net_angle / num_tris;

  if (avg_angle > _bucket->_max_tfan_angle) {
    // The triangles are too loose to justify making a fan; it'll
    // probably make a better quadsheet.
    return 0;
  }

  if (_bucket->_min_tfan_tris==0 || num_tris < _bucket->_min_tfan_tris) {
    // Oops, not enough triangles to justify a fan.
    if (!_bucket->_unroll_fans) {
      return 0;
    }

    // However, we could (maybe) make it a few tristrips!

    // Each section of the fan which is made up of coplanar tris with
    // identical properties may be retesselated into a tristrip.  What
    // a sneaky trick!  To do this, we must first identify each such
    // qualifying section.

    // We define a seam as the edge between any two tris which are
    // noncoplanar or which do not share identical properties.  Then
    // we can send each piece between the seams to unroll().

    TYPENAME Strips::iterator si, last_si;
    TYPENAME Edges::iterator ei, last_ei;

    // First, rotate the fan so it begins at a seam.  We do this so we
    // won't be left out with part of one piece at the beginning and
    // also at the end.
    si = _strips.begin();
    last_si = si;
    ei = _edges.begin();
    last_ei = ei;
    int found_seam = false;

    for (++si, ++ei; si != _strips.end() && !found_seam; ++si, ++ei) {
      nassertr(ei != _edges.end(), 0);
      if ( !((*si)->_prims.front() == (*last_si)->_prims.front()) ||
           !(*si)->is_coplanar_with(*(*last_si), _bucket->_coplanar_threshold)) {
        // Here's a seam.  Break the fan here.
        found_seam = true;
        _edges.splice(_edges.begin(), _edges, ei, _edges.end());
        _strips.splice(_strips.begin(), _strips, si, _strips.end());
      }
    }

    // Now break the fan up along its seams and unroll each piece
    // separately.
    si = _strips.begin();
    last_si = si;
    ei = _edges.begin();
    last_ei = ei;

    int count = 0;
    for (++si, ++ei; si != _strips.end(); ++si, ++ei) {
      nassertr(ei != _edges.end(), 0);
      if ( !((*si)->_prims.front() == (*last_si)->_prims.front()) ||
           !(*si)->is_coplanar_with(*(*last_si), _bucket->_coplanar_threshold)) {
        // Here's the end of a run of matching pieces.
        count += unroll(last_si, si, last_ei, ei, unrolled_tris);
        last_si = si;
        last_ei = ei;
      }
    }
    count += unroll(last_si, si, last_ei, ei, unrolled_tris);

    return count;

  } else {
    Strip new_fan;
    new_fan._type = BPT_trifan;
    new_fan._verts.push_back(_vertex);

    new_fan._verts.push_back(_edges.front()->_a);
    TYPENAME Edges::iterator ei;
    for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
      new_fan._verts.push_back((*ei)->_b);
    }

    TYPENAME Strips::iterator si;
    for (si = _strips.begin(); si != _strips.end(); ++si) {
      new_fan._prims.splice(new_fan._prims.end(), (*si)->_prims);
      (*si)->remove_all_edges();
      (*si)->_verts.clear();
      (*si)->_status = MS_dead;
    }

    // If we'd built our list of edges and strips right, this sum should
    // come out so that there are two more vertices than triangles in
    // the new fan.
    nassertr(new_fan._verts.size() == new_fan._prims.size() + 2, 0);

    // Now we've built a fan, and it won't be able to mate with
    // anything else, so add it to the done list.
    _mesher->_done.push_back(new_fan);
  }

  return 1;
}



template <class PrimType>
int MesherFanMaker<PrimType>::
unroll(TYPENAME Strips::iterator strip_begin, TYPENAME Strips::iterator strip_end,
       TYPENAME Edges::iterator edge_begin, TYPENAME Edges::iterator edge_end,
       pvector<Prim> &unrolled_tris) {
  TYPENAME Edges::iterator ei;
  TYPENAME Strips::iterator si;

  int num_tris = 0;
  for (ei = edge_begin; ei != edge_end; ++ei) {
    num_tris++;
  }

  if (num_tris < 3) {
    // Don't even bother.
    return 0;
  }

  Prim poly;

  // Now we build an n-sided polygon.  We'll decompose it into tris
  // in a second.
  poly.set_type(BPT_poly);
  poly.set_attrib((*strip_begin)->_prims.front());

  ei = edge_end;
  --ei;
  if ( !((*ei)->_b == (*edge_begin)->_a)) {
    // If the fan is less than a full circle, we need to keep the
    // hub vertex and initial vertex in the poly.  Otherwise, we'll
    // discard them.
    poly.add_vertex(*_vertex);
    poly.add_vertex(*(*edge_begin)->_a);
  }

  for (ei = edge_begin; ei != edge_end; ++ei) {
    poly.add_vertex(*(*ei)->_b);
  }

  int result = true;

  if (_bucket->_show_quads) {
    // If we're showing quads, also show retesselated triangles.

    // We can't add it directly to the mesher, that's unsafe; instead,
    // we'll just add it to the end of the unrolled_tris list.  This
    // does mean we won't be able to color it a fancy color, but too
    // bad.
    //_mesher->add_prim(poly, MO_fanpoly);
    unrolled_tris.push_back(poly);

  } else {
    // Now decompose the new polygon into triangles.
    pvector<Prim> tris;
    result = expand(poly, *_bucket, back_inserter(tris));

    if (result) {
      unrolled_tris.insert(unrolled_tris.end(),
                           tris.begin(), tris.end());
    }
  }

  if (result) {
    // Now that we've created a new poly, kill off all the old ones.
    for (si = strip_begin; si != strip_end; ++si) {
      (*si)->remove_all_edges();
      (*si)->_verts.clear();
      (*si)->_prims.clear();
      (*si)->_status = MS_dead;
    }
    return 1;
  } else {
    return 0;
  }

}

template <class PrimType>
ostream &MesherFanMaker<PrimType>::
output(ostream &out) const {
  out << *_vertex << ":[";
  if (!_edges.empty()) {
    TYPENAME Edges::const_iterator ei;
    for (ei = _edges.begin(); ei != _edges.end(); ++ei) {
      out << " " << *(*ei)->_a;
    }
    out << " " << *_edges.back()->_b;
  }
  out << " ]";

  if (_planar) {
    out << " (planar)";
  }
  return out;
}
