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
//  Description: Returns the nth geom of the node.  This object should
//               not be modified, since the same object might be
//               shared between multiple different GeomNodes, but see
//               get_unique_geom().
////////////////////////////////////////////////////////////////////
INLINE const Geom *GeomNode::
get_geom(int n) const {
  CDReader cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_geoms.size(), NULL);
  return cdata->_geoms[n]._geom;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_unique_geom
//       Access: Public
//  Description: Returns the nth geom of the node, suitable for
//               modifying it.  If the nth Geom has multiple reference
//               counts to it, reassigns it to an identical copy
//               first, and returns the new copy--this provides a
//               "copy on write" that ensures that the Geom that is
//               returned is unique to this GeomNode and is not shared
//               with any other GeomNodes.
////////////////////////////////////////////////////////////////////
INLINE Geom *GeomNode::
get_unique_geom(int n) {
  CDWriter cdata(_cycler);
  nassertr(n >= 0 && n < (int)cdata->_geoms.size(), NULL);
  Geom *geom = cdata->_geoms[n]._geom;
  if (geom->get_ref_count() > 1) {
    geom = cdata->_geoms[n]._geom = geom->make_copy();
  }

  return geom;
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

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_default_collide_mask
//       Access: Published, Static
//  Description: Returns the default into_collide_mask assigned to new
//               GeomNodes.
////////////////////////////////////////////////////////////////////
INLINE CollideMask GeomNode::
get_default_collide_mask() {
  return default_geom_node_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::count_name
//       Access: Private
//  Description: Increments the count for the indicated TexCoordName.
////////////////////////////////////////////////////////////////////
INLINE void GeomNode::
count_name(GeomNode::NameCount &name_count, const TexCoordName *name) {
  pair<NameCount::iterator, bool> result = 
    name_count.insert(NameCount::value_type(name, 1));
  if (!result.second) {
    (*result.first).second++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_name_count
//       Access: Private
//  Description: Returns the count for the indicated TexCoordName.
////////////////////////////////////////////////////////////////////
INLINE int GeomNode::
get_name_count(const GeomNode::NameCount &name_count, const TexCoordName *name) {
  NameCount::const_iterator ni;
  ni = name_count.find(name);
  if (ni != name_count.end()) {
    return (*ni).second;
  }
  return 0;
}
