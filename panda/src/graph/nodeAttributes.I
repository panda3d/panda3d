// Filename: nodeAttributes.I
// Created by:  drose (21Mar00)
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

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes::size_type NodeAttributes::
size() const {
  return _attributes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes::iterator NodeAttributes::
begin() {
  return _attributes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes::iterator NodeAttributes::
end() {
  return _attributes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes::const_iterator NodeAttributes::
begin() const {
  return _attributes.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes::const_iterator NodeAttributes::
end() const {
  return _attributes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes::iterator NodeAttributes::
insert(NodeAttributes::iterator position,
       const NodeAttributes::value_type &x) {
  return _attributes.insert(position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeAttributes::
erase(NodeAttributes::iterator position) {
  _attributes.erase(position);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::apply_in_place
//       Access: Public
//  Description: Modifies the current NodeAttributes object to reflect
//               the application of the indicated transitions.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NodeAttributes::
apply_in_place(const NodeTransitionCache &trans) {
  apply_from(*this, trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributes::apply
//       Access: Public
//  Description: Allocates and returns a new NodeAttributes object
//               that represents the application of this
//               NodeAttributes to the indicated transition cache.
//               This NodeAttributes object is not changed.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeAttributes *NodeAttributes::
apply(const NodeTransitionCache &trans) const {
  NodeAttributes *na = new NodeAttributes;
  na->apply_from(*this, trans);
  return na;
}

INLINE_GRAPH ostream &operator << (ostream &out, const NodeAttributes &nas) {
  nas.output(out);
  return out;
}

