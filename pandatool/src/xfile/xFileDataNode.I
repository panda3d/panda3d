// Filename: xFileDataNode.I
// Created by:  drose (08Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: XFileDataNode::get_data_child
//       Access: Public
//  Description: Since the children of an XFileDataNode are
//               syntactically constrained to themselves be
//               XFileDataNodes, this is a convenience function that
//               returns the same thing as XFileNode::get_child(), but
//               it is cast to a type XFileDataNode and dereferenced.
////////////////////////////////////////////////////////////////////
INLINE const XFileDataNode &XFileDataNode::
get_data_child(int n) const {
  return *DCAST(XFileDataNode, get_child(n));
}


////////////////////////////////////////////////////////////////////
//     Function: XFileDataNode::get_template
//       Access: Public
//  Description: Returns the template used to define this data object.
//               Since the only classes to inherit from XFileDataNode
//               are XFileDataNodeTemplate and XFileDataNodeReference,
//               both of which represent a class that is defined by a
//               template, it makes sense to put this common method
//               here in the base class.
////////////////////////////////////////////////////////////////////
INLINE XFileTemplate *XFileDataNode::
get_template() const {
  return _template;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataNode::get_template_name
//       Access: Public
//  Description: A convenience function to return the name of the
//               template used to define this data object.
////////////////////////////////////////////////////////////////////
INLINE const string &XFileDataNode::
get_template_name() const {
  return _template->get_name();
}

