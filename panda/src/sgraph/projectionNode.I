// Filename: projectionNode.I
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
//     Function: ProjectionNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ProjectionNode::
ProjectionNode(const string &name) :
  NamedNode(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ProjectionNode::
ProjectionNode(const ProjectionNode &copy) :
  NamedNode(copy),
  _projection(copy._projection)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void ProjectionNode::
operator = (const ProjectionNode &copy) {
  NamedNode::operator = (copy);
  _projection = copy._projection;
}
