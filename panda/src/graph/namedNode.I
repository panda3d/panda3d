// Filename: namedNode.I
// Created by:  drose (23May00)
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
//     Function: NamedNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NamedNode::
NamedNode(const string &initial_name) :
  Namable(initial_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NamedNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NamedNode::
NamedNode(const NamedNode &copy) :
  Node(copy), Namable(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NamedNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void NamedNode::
operator = (const NamedNode &copy) {
  Node::operator = (copy);
  Namable::operator = (copy);
}

EXPCL_PANDA INLINE_GRAPH ostream & operator << (ostream &out, const NamedNode &nod) {
  nod.output(out);
  return out;
}
