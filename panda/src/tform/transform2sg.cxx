// Filename: transform2sg.cxx
// Created by:  drose (27Jan99)
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

#include "transform2sg.h"

#include "nodeRelation.h"
#include "transformTransition.h"
#include "matrixDataTransition.h"
#include "allTransitionsWrapper.h"


TypeHandle Transform2SG::_type_handle;

TypeHandle Transform2SG::_transform_type;

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Transform2SG::
Transform2SG(const string &name) : DataNode(name) {
  _arc = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::set_arc
//       Access: Public
//  Description: Sets the arc that this node will adjust.
////////////////////////////////////////////////////////////////////
void Transform2SG::
set_arc(NodeRelation *arc) {
  _arc = arc;
}

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::get_arc
//       Access: Public
//  Description: Returns the arc that this node will adjust, or NULL
//               if the arc has not yet been set.
////////////////////////////////////////////////////////////////////
NodeRelation *Transform2SG::
get_arc() const {
  return _arc;
}


////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::transmit_data
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Transform2SG::
transmit_data(AllTransitionsWrapper &data) {
  const NodeTransition *transform = data.get_transition(_transform_type);

  if (transform != (NodeTransition *)NULL && _arc != (NodeRelation *)NULL) {
    const LMatrix4f &mat = DCAST(MatrixDataTransition, transform)->get_value();
    _arc->set_transition(new TransformTransition(mat));
  }

  // Clear the data below us.
  data.clear();
}


////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void Transform2SG::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "Transform2SG",
                DataNode::get_class_type());

  MatrixDataTransition::init_type();
  register_data_transition(_transform_type, "Transform",
                           MatrixDataTransition::get_class_type());
}
