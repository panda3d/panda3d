// Filename: softNodeDesc.cxx
// Created by:  masad (03Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2003, Disney Enterprises, Inc.  All rights reserved
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

#include "softNodeDesc.h"

TypeHandle SoftNodeDesc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftNodeDesc::
SoftNodeDesc(const string &name) :
  Namable(name)
  //  _parent(parent)
{
  _model = (SAA_Elem *)NULL;
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;
  _joint_type = JT_none;
#if 0
  // Add ourselves to our parent.
  if (_parent != (SoftNodeDesc *)NULL) {
    _parent->_children.push_back(this);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftNodeDesc::
~SoftNodeDesc() {
  if (_model != (SAA_Elem *)NULL) {
    delete _model;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::set_model
//       Access: Public
//  Description: Indicates an associated between the SoftNodeDesc and
//               some SAA_Elem instance.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
set_model(SAA_Elem *model) {
  _model = model;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::has_model
//       Access: Public
//  Description: Returns true if a Soft dag path has been associated
//               with this node, false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
has_model() const {
  return (_model != (SAA_Elem *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::get_model
//       Access: Public
//  Description: Returns the SAA_Elem * associated with this node.  It
//               is an error to call this unless has_model()
//               returned true.
////////////////////////////////////////////////////////////////////
SAA_Elem *SoftNodeDesc::
get_model() const {
  nassertr(_model != (SAA_Elem *)NULL, _model);
  return _model;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::is_joint
//       Access: Private
//  Description: Returns true if the node should be treated as a joint
//               by the converter.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
is_joint() const {
  return _joint_type == JT_joint || _joint_type == JT_pseudo_joint;
}
#if 0
////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::is_joint_parent
//       Access: Private
//  Description: Returns true if the node is the parent or ancestor of
//               a joint.
////////////////////////////////////////////////////////////////////
bool SoftNodeDesc::
is_joint_parent() const {
  return _joint_type == JT_joint_parent;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::clear_egg
//       Access: Private
//  Description: Recursively clears the egg pointers from this node
//               and all children.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
clear_egg() {
  _egg_group = (EggGroup *)NULL;
  _egg_table = (EggTable *)NULL;
  _anim = (EggXfmSAnim *)NULL;

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    SoftNodeDesc *child = (*ci);
    child->clear_egg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::mark_joint_parent
//       Access: Private
//  Description: Indicates that this node has at least one child that
//               is a joint or a pseudo-joint.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
mark_joint_parent() {
  if (_joint_type == JT_none) {
    _joint_type = JT_joint_parent;
    if (_parent != (SoftNodeDesc *)NULL) {
      _parent->mark_joint_parent();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftNodeDesc::check_pseudo_joints
//       Access: Private
//  Description: Walks the hierarchy, looking for non-joint nodes that
//               are both children and parents of a joint.  These
//               nodes are deemed to be pseudo joints, since the
//               converter must treat them as joints.
////////////////////////////////////////////////////////////////////
void SoftNodeDesc::
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

    bool any_joints = false;
    Children::const_iterator ci;
    for (ci = _children.begin(); ci != _children.end(); ++ci) {
      SoftNodeDesc *child = (*ci);
      child->check_pseudo_joints(joint_above);
      if (child->is_joint()) {
        any_joints = true;
      }
    }

    // If any children qualify as joints, then any sibling nodes that
    // are parents of joints are also elevated to joints.
    if (any_joints) {
      bool all_joints = true;
      for (ci = _children.begin(); ci != _children.end(); ++ci) {
        SoftNodeDesc *child = (*ci);
        if (child->_joint_type == JT_joint_parent) {
          child->_joint_type = JT_pseudo_joint;
        } else if (child->_joint_type == JT_none) {
          all_joints = false;
        }
      }

      if (all_joints) {
        // Finally, if all children are joints, then we are too.
        if (_joint_type == JT_joint_parent) {
          _joint_type = JT_pseudo_joint;
        }
      }
    }
  }
}
#endif
