// Filename: transform2sg.cxx
// Created by:  drose (12Mar02)
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
#include "transformState.h"


TypeHandle Transform2SG::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Transform2SG::
Transform2SG(const string &name) :
  DataNode(name)
{
  _transform_input = define_input("transform", EventStoreMat4::get_class_type());
  _velocity_input = define_input("velocity", EventStoreVec3::get_class_type());

  _node = NULL;
  _velocity_node = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::set_node
//       Access: Public
//  Description: Sets the node that this object will adjust.
////////////////////////////////////////////////////////////////////
void Transform2SG::
set_node(PandaNode *node) {
  _node = node;
}

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::get_node
//       Access: Public
//  Description: Returns the node that this object will adjust, or NULL
//               if the node has not yet been set.
////////////////////////////////////////////////////////////////////
PandaNode *Transform2SG::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::set_velocity_node
//       Access: Public
//  Description: Sets the node that this object will assign the
//               computed velocity to.  Normally this is a
//               CollisionNode parented below the node indicated by
//               set_node().  Setting this node allows the collision
//               system to track the velocity imparted to the
//               CollisionNode by the data graph object that set its
//               transform, if that data is available.
////////////////////////////////////////////////////////////////////
void Transform2SG::
set_velocity_node(PandaNode *node) {
  _velocity_node = node;
}

////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::get_velocity_node
//       Access: Public
//  Description: Returns the node that this object will assign the
//               computed velocity to.  See set_velocity_node().
////////////////////////////////////////////////////////////////////
PandaNode *Transform2SG::
get_velocity_node() const {
  return _velocity_node;
}


////////////////////////////////////////////////////////////////////
//     Function: Transform2SG::do_transmit_data
//       Access: Protected, Virtual
//  Description: The virtual implementation of transmit_data().  This
//               function receives an array of input parameters and
//               should generate an array of output parameters.  The
//               input parameters may be accessed with the index
//               numbers returned by the define_input() calls that
//               were made earlier (presumably in the constructor);
//               likewise, the output parameters should be set with
//               the index numbers returned by the define_output()
//               calls.
////////////////////////////////////////////////////////////////////
void Transform2SG::
do_transmit_data(const DataNodeTransmit &input, DataNodeTransmit &) {
  if (input.has_data(_transform_input)) {
    const EventStoreMat4 *transform;
    DCAST_INTO_V(transform, input.get_data(_transform_input).get_ptr());
    const LMatrix4f &mat = transform->get_value();
    if (_node != (PandaNode *)NULL) {
      _node->set_transform(TransformState::make_mat(mat));
    }
  }

  if (input.has_data(_velocity_input)) {
    const EventStoreVec3 *velocity;
    DCAST_INTO_V(velocity, input.get_data(_velocity_input).get_ptr());
    LVector3f vel = velocity->get_value();
    if (_velocity_node != (PandaNode *)NULL) {
      _velocity_node->set_velocity(vel);
    }
  }
}
