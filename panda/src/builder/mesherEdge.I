// Filename: mesherEdge.I
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



template <class PrimType>
INLINE MesherEdge<PrimType>::
MesherEdge(const Vertex *a, const Vertex *b) : _a(a), _b(b) {
  _opposite = NULL;
}

template <class PrimType>
INLINE MesherEdge<PrimType>::
MesherEdge(const MesherEdge &copy) :
  _a(copy._a),
  _b(copy._b),
  _strips(copy._strips),
  _opposite(copy._opposite)
{
}

template <class PrimType>
INLINE bool MesherEdge<PrimType>::
contains_vertex(const Vertex *v) const {
  return (_a==v || _b==v);
}


template <class PrimType>
INLINE bool MesherEdge<PrimType>::
matches(const MesherEdge &other) const {
  return (_a == other._a && _b == other._b) ||
    (_b == other._a && _a == other._b);
}

template <class PrimType>
INLINE MesherEdge<PrimType> *MesherEdge<PrimType>::
common_ptr() {
  return min(this, _opposite);
}

template <class PrimType>
INLINE bool MesherEdge<PrimType>::
operator == (const MesherEdge &other) const {
  return _a == other._a && _b == other._b;
}

template <class PrimType>
INLINE bool MesherEdge<PrimType>::
operator != (const MesherEdge &other) const {
  return !operator == (other);
}

template <class PrimType>
INLINE bool MesherEdge<PrimType>::
operator < (const MesherEdge &other) const {
  return _a < other._a || (_a == other._a && _b < other._b);
}

template <class PrimType>
INLINE float MesherEdge<PrimType>::
compute_length(const BuilderBucket &bucket) const {
  LVector3f v = ((const Vertexf &)_a->get_coord_value(bucket) -
                 (const Vertexf &)_b->get_coord_value(bucket));
  return length(v);
}

template <class PrimType>
INLINE Vertexf MesherEdge<PrimType>::
compute_box(const BuilderBucket &bucket) const {
  LVector3f v = ((const Vertexf &)_a->get_coord_value(bucket) -
                 (const Vertexf &)_b->get_coord_value(bucket));
  return Vertexf(fabs(v[0]), fabs(v[1]), fabs(v[2]));
}


////////////////////////////////////////////////////////////////////
//     Function: MesherEdge::remove
//       Access: Public
//  Description: Removes an edge from a particular strip.
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherEdge<PrimType>::
remove(Strip *strip) {
  strip->_edges.remove(this);
  strip->_edges.remove(_opposite);

  _strips.remove(strip);
  _opposite->_strips.remove(strip);
}



////////////////////////////////////////////////////////////////////
//     Function: MesherEdge::change_strip
//       Access: Public
//  Description: Reparents the edge from strip "from" to strip "to".
////////////////////////////////////////////////////////////////////
template <class PrimType>
void MesherEdge<PrimType>::
change_strip(Strip *from, Strip *to) {
  TYPENAME Strips::iterator si;

  for (si = _strips.begin(); si != _strips.end(); ++si) {
    if (*si == from) {
      *si = to;
    }
  }

  for (si = _opposite->_strips.begin();
       si != _opposite->_strips.end();
       ++si) {
    if (*si == from) {
      *si = to;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MesherEdge::output
//       Access: Public
//  Description: Formats the edge for output in some sensible way.
////////////////////////////////////////////////////////////////////
template <class PrimType>
ostream &MesherEdge<PrimType>::
output(ostream &out) const {
  out << "Edge [" << *_a << " to " << *_b << "], "
      << _strips.size() << " strips:";

  TYPENAME Strips::const_iterator si;
  for (si = _strips.begin(); si != _strips.end(); ++si) {
    out << " " << (*si)->_index;
  }

  if (_opposite!=NULL) {
    out << " opposite "
        << _opposite->_strips.size() << " strips:";

    for (si = _opposite->_strips.begin();
         si != _opposite->_strips.end();
         ++si) {
      out << " " << (*si)->_index;
    }
  }

  return out;
}
