// Filename: maxNodeDesc.cxx
// Created by:  crevilla
// from mayaNodeDesc.cxx created by:  drose (06Jun03)
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

#include "maxNodeDesc.h"
#include "maxLogger.h"
#define MTEC Logger::ST_MAP_ME_TO_APP_SPECIFIC_SYSTEM4

TypeHandle MaxNodeDesc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MaxNodeDesc::
MaxNodeDesc(MaxNodeDesc *parent, const string &name) :
  Namable(name),
  _parent(parent)
{
  _max_node = (INode *)NULL;
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;
  _joint_type = JT_none;
  _joint_entry = NULL;

  // Add ourselves to our parent.
  if (_parent != (MaxNodeDesc *)NULL) {
    _parent->_children.push_back(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MaxNodeDesc::
~MaxNodeDesc() {}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::from_INode
//       Access: Public
//  Description: Indicates an associated between the MaxNodeDesc and
//               some Max Node instance.
////////////////////////////////////////////////////////////////////
void MaxNodeDesc::
from_INode(INode *max_node) {
  if (_max_node == (INode *)NULL) {
    _max_node = max_node;

    // This is how I decided to check to see if this max node is a 
    // joint.  It works in all instances I've seen so far, but this 
    // may be a good starting place to look if joints are not being
    // picked up correctly in the future.

    //Check to see if the node's controller is a biped
    //If so treat it as a joint
    // Get the node's transform control
    Control *c = max_node->GetTMController();
    if (_max_node->GetBoneNodeOnOff() ||
        (c && //c exists and it's type is a biped
        ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
         (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) ||
         (c->ClassID() == FOOTPRINT_CLASS_ID)))) {
      Logger::Log( MTEC, Logger::SAT_MEDIUM_LEVEL, "Found a joint." );
      
      // This node is a joint.
      _joint_type = JT_node_joint;
      if (_parent != (MaxNodeDesc *)NULL) {
        _parent->mark_joint_parent();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::has_max_node
//       Access: Public
//  Description: Returns true if a Max INode has been associated
//               with this node, false otherwise.
////////////////////////////////////////////////////////////////////
bool MaxNodeDesc::
has_max_node() const {
  return (_max_node != (INode *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::get_max_node
//       Access: Public
//  Description: Returns the INode associated with this node.  It
//               is an error to call this unless has_max_node()
//               returned true.
////////////////////////////////////////////////////////////////////
INode *MaxNodeDesc::
get_max_node() const {
  nassertr(_max_node != (INode *)NULL, _max_node);
  return _max_node;
}


void MaxNodeDesc::
set_joint(bool onoff) {
  if (onoff)
    _joint_type = JT_joint;
  else
    _joint_type = JT_none;
}
    
////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::is_joint
//       Access: Private
//  Description: Returns true if the node should be treated as a joint
//               by the converter.
////////////////////////////////////////////////////////////////////
bool MaxNodeDesc::
is_joint() const {
  return _joint_type == JT_joint || _joint_type == JT_pseudo_joint;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::is_joint_parent
//       Access: Private
//  Description: Returns true if the node is the parent or ancestor of
//               a joint.
////////////////////////////////////////////////////////////////////
bool MaxNodeDesc::
is_joint_parent() const {
  return _joint_type == JT_joint_parent;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::is_joint_parent
//       Access: Private
//  Description: Returns true if the node is the parent or ancestor of
//               a joint.
////////////////////////////////////////////////////////////////////
bool MaxNodeDesc::
is_node_joint() const {
  return _joint_type == JT_node_joint;
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::clear_egg
//       Access: Private
//  Description: Recursively clears the egg pointers from this node
//               and all children.
////////////////////////////////////////////////////////////////////
void MaxNodeDesc::
clear_egg() {
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    MaxNodeDesc *child = (*ci);
    child->clear_egg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::mark_joint_parent
//       Access: Private
//  Description: Indicates that this node has at least one child that
//               is a joint or a pseudo-joint.
////////////////////////////////////////////////////////////////////
void MaxNodeDesc::
mark_joint_parent() {
  if (_joint_type == JT_none) {
    _joint_type = JT_joint_parent;
    if (_parent != (MaxNodeDesc *)NULL) {
      _parent->mark_joint_parent();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MaxNodeDesc::check_pseudo_joints
//       Access: Private
//  Description: Walks the hierarchy, looking for non-joint nodes that
//               are both children and parents of a joint.  These
//               nodes are deemed to be pseudo joints, since the
//               converter must treat them as joints.
////////////////////////////////////////////////////////////////////
void MaxNodeDesc::
check_pseudo_joints(bool joint_above) {
  if (_joint_type == JT_joint_parent && joint_above) {
    // This is one such node: it is the parent of a joint
    // (JT_joint_parent is set), and it is the child of a joint
    // (joint_above is set).
    _joint_type = JT_pseudo_joint;
  }

  if (_joint_type == JT_joint) {
    // If this node is itself a joint, then joint_above is true for
    // all child nodes.
    joint_above = true;
  }

  // Don't bother traversing further if _joint_type is none, since
  // that means this node has no joint children.
  if (_joint_type != JT_none) {
    Children::const_iterator ci;
    for (ci = _children.begin(); ci != _children.end(); ++ci) {
      MaxNodeDesc *child = (*ci);
      child->check_pseudo_joints(joint_above);
    }
  }
}
