// Filename: nodeTransitions.I
// Created by:  drose (20Mar00)
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
//     Function: NodeTransitions::set_transition
//       Access: Public
//  Description: This flavor of set_transition() accepts a pointer to
//               a NodeTransition only.  It infers the type of the
//               NodeTransition from the pointer.  However, it is not
//               valid to pass a NULL pointer to this flavor of
//               set_transition; if the pointer might be NULL, use the
//               above flavor instead (or just call clear_transition).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH PT(NodeTransition) NodeTransitions::
set_transition(NodeTransition *trans) {
  nassertr(trans != (NodeTransition *)NULL, NULL);
  nassertr(trans->get_handle() != TypeHandle::none(), NULL);
  return set_transition(trans->get_handle(), trans);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitions::size_type NodeTransitions::
size() const {
  return _transitions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitions::const_iterator NodeTransitions::
begin() const {
  return _transitions.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: NodeTransitions::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeTransitions::const_iterator NodeTransitions::
end() const {
  return _transitions.end();
}

INLINE_GRAPH ostream &operator << (ostream &out, const NodeTransitions &nts) {
  nts.output(out);
  return out;
}
