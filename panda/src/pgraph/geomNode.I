// Filename: geomNode.I
// Created by:  drose (23Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: GeomNode::GeomEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomNode::GeomEntry::
GeomEntry(Geom *geom, const RenderState *state) :
  _geom(geom),
  _state(state)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomNode::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_num_geoms
//       Access: Public
//  Description: Returns the number of geoms in the node.
////////////////////////////////////////////////////////////////////
INLINE int GeomNode::
get_num_geoms() const {
  CDReader cdata(_cycler);
  return cdata->_geoms.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_geom
//       Access: Public
//  Description: Returns the nth geom of the node.
////////////////////////////////////////////////////////////////////
INLINE Geom *GeomNode::
get_geom(int n) const {
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_geoms.size(), NULL);
  return cdata->_geoms[n]._geom;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_geom_state
//       Access: Public
//  Description: Returns the RenderState associated with the nth geom
//               of the node.  This is just the RenderState directly
//               associated with the Geom; the actual state in which
//               the Geom is rendered will also be affected by
//               RenderStates that appear on the scene graph in nodes
//               above this GeomNode.
////////////////////////////////////////////////////////////////////
INLINE const RenderState *GeomNode::
get_geom_state(int n) const {
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_geoms.size(), NULL);
  return cdata->_geoms[n]._state;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::set_geom_state
//       Access: Public
//  Description: Changes the RenderState associated with the nth geom
//               of the node.  This is just the RenderState directly
//               associated with the Geom; the actual state in which
//               the Geom is rendered will also be affected by
//               RenderStates that appear on the scene graph in nodes
//               above this GeomNode.
////////////////////////////////////////////////////////////////////
INLINE void GeomNode::
set_geom_state(int n, const RenderState *state) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_geoms.size());
  cdata->_geoms[n]._state = state;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_geom
//       Access: Published
//  Description: Adds a new Geom to the node.  The geom is given the
//               indicated state (which may be
//               RenderState::make_empty(), to completely inherit its
//               state from the scene graph).
//
//               The return value is the index number of the new Geom.
////////////////////////////////////////////////////////////////////
INLINE int GeomNode::
add_geom(Geom *geom, const RenderState *state) {
  nassertr(geom != (Geom *)NULL, -1);
  nassertr(state != (RenderState *)NULL, -1);
  CDWriter cdata(_cycler);

  cdata->_geoms.push_back(GeomEntry(geom, state));
  mark_bound_stale();
  return cdata->_geoms.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::remove_geom
//       Access: Published
//  Description: Removes the nth geom from the node.
////////////////////////////////////////////////////////////////////
INLINE void GeomNode::
remove_geom(int n) {
  CDWriter cdata(_cycler);
  nassertv(n >= 0 && n < (int)cdata->_geoms.size());

  cdata->_geoms.erase(cdata->_geoms.begin() + n);
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::remove_all_geoms
//       Access: Published
//  Description: Removes all the geoms from the node at once.
////////////////////////////////////////////////////////////////////
INLINE void GeomNode::
remove_all_geoms() {
  CDWriter cdata(_cycler);
  cdata->_geoms.clear();
  mark_bound_stale();
}
