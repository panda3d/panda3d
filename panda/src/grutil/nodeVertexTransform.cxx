// Filename: nodeVertexTransform.cxx
// Created by:  drose (22eb07)
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

#include "nodeVertexTransform.h"

TypeHandle NodeVertexTransform::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeVertexTransform::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NodeVertexTransform::
NodeVertexTransform(const PandaNode *node, 
                    const VertexTransform *prev) :
  _node(node),
  _prev(prev)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodeVertexTransform::get_matrix
//       Access: Published, Virtual
//  Description: Returns the transform of the associated node,
//               composed with the previous VertexTransform if any,
//               expressed as a matrix.
////////////////////////////////////////////////////////////////////
void NodeVertexTransform::
get_matrix(LMatrix4 &matrix) const {
  if (_prev != (const VertexTransform *)NULL) {
    LMatrix4 prev_matrix;
    _prev->get_matrix(prev_matrix);
    matrix.multiply(_node->get_transform()->get_mat(), prev_matrix);

  } else {
    matrix = _node->get_transform()->get_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeVertexTransform::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void NodeVertexTransform::
output(ostream &out) const {
  if (_prev != (const VertexTransform *)NULL) {
    _prev->output(out);
    out << " * ";
  }

  out << "NodeVertexTransform(" << _node->get_name() << ")";
}
