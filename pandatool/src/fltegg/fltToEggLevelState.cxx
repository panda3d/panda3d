// Filename: fltToEggLevelState.cxx
// Created by:  drose (18Apr01)
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

#include "fltToEggLevelState.h"

#include <eggGroup.h>


////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltToEggLevelState::
~FltToEggLevelState() {
  Parents::iterator pi;
  for (pi = _parents.begin(); pi != _parents.end(); ++pi) {
    delete (*pi).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::ParentNodes::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltToEggLevelState::ParentNodes::
ParentNodes() {
  _axial_billboard = (EggGroup *)NULL;
  _point_billboard = (EggGroup *)NULL;
  _plain = (EggGroup *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggLevelState::get_synthetic_group
//       Access: Public
//  Description: Sometimes it is necessary to synthesize a group
//               within a particular EggGroup, for instance to insert
//               a transform or billboard flag.  This function will
//               synthesize a group as needed, or return an existing
//               group (if the group need not be synthesized, or if a
//               matching group was previously synthesized).
//
//               This collects together polygons that share the same
//               billboard axis and/or transform space into the same
//               group, rather that wastefully creating a group per
//               polygon.
////////////////////////////////////////////////////////////////////
EggGroupNode *FltToEggLevelState::
get_synthetic_group(const string &name,
                    const LMatrix4d &transform,
                    FltGeometry::BillboardType type) {

  bool is_identity = transform.almost_equal(LMatrix4d::ident_mat());
  if (is_identity && type == FltGeometry::BT_none) {
    // Trivial case: the primitive belongs directly in its parent
    // group node.
    return _egg_parent;
  }

  // For other cases, we may have to create a subgroup to put the
  // primitive into.
  Parents::iterator pi;
  pi = _parents.find(transform);
  ParentNodes *nodes;
  if (pi != _parents.end()) {
    nodes = (*pi).second;
  } else {
    nodes = new ParentNodes;
    _parents.insert(Parents::value_type(transform, nodes));
  }

  switch (type) {
  case FltGeometry::BT_axial:
    if (nodes->_axial_billboard == (EggGroupNode *)NULL) {
      nodes->_axial_billboard = new EggGroup(name);
      _egg_parent->add_child(nodes->_axial_billboard);
      nodes->_axial_billboard->set_billboard_type(EggGroup::BT_axis);
      if (!is_identity) {
        nodes->_axial_billboard->set_transform(transform);
        nodes->_axial_billboard->set_group_type(EggGroup::GT_instance);
      }
    }
    return nodes->_axial_billboard;

  case FltGeometry::BT_point:
    if (nodes->_point_billboard == (EggGroupNode *)NULL) {
      nodes->_point_billboard = new EggGroup(name);
      _egg_parent->add_child(nodes->_point_billboard);
      nodes->_point_billboard->set_billboard_type(EggGroup::BT_point_world_relative);
      if (!is_identity) {
        nodes->_point_billboard->set_transform(transform);
        nodes->_point_billboard->set_group_type(EggGroup::GT_instance);
      }
    }
    return nodes->_point_billboard;

  default:
    if (nodes->_plain == (EggGroupNode *)NULL) {
      nodes->_plain = new EggGroup(name);
      _egg_parent->add_child(nodes->_plain);
      if (!is_identity) {
        nodes->_plain->set_transform(transform);
        nodes->_plain->set_group_type(EggGroup::GT_instance);
      }
    }
    return nodes->_plain;
  }
}
