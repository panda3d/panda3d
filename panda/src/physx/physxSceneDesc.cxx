// Filename: physxSceneDesc.cxx
// Created by:  enn0x (05Sep09)
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

#include "physxSceneDesc.h"
#include "physxManager.h"
#include "physxBounds3.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_gravity
//       Access: Published
//  Description: Sets the gravity vector.
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_gravity(const LVector3f &gravity) {

  nassertv_always(!gravity.is_nan());
  _desc.gravity = PhysxManager::vec3_to_nxVec3(gravity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_gravity
//       Access: Published
//  Description: Get the gravity vector.
////////////////////////////////////////////////////////////////////
LVector3f PhysxSceneDesc::
get_gravity() const {

  return PhysxManager::nxVec3_to_vec3(_desc.gravity);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_flag
//       Access: Published
//  Description: Raise or lower individual scene flags.
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_flag(const PhysxSceneFlag flag, bool value) {

  if (value == true) {
    _desc.flags |= flag;
  }
  else {
    _desc.flags &= ~(flag);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_flag
//       Access: Published
//  Description: Returns the specified scene flag.
////////////////////////////////////////////////////////////////////
bool PhysxSceneDesc::
get_flag(const PhysxSceneFlag flag) const {

  return (_desc.flags & flag) ? true : false;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_max_bounds
//       Access: Published
//  Description: Set the max scene bounds. 
//
//               If scene bounds are provided (maxBounds in the
//               descriptor), the SDK takes advantage of this
//               information to accelerate scene-level collision
//               queries (e.g. raycasting). When using maxBounds,
//               you have to make sure created objects stay within
//               the scene bounds. In particular, the position of
//               dynamic shapes should stay within the provided
//               bounds. Otherwise the shapes outside the bounds
//               will not be taken into account by all scene queries
//               (raycasting, sweep tests, overlap tests, etc). They
//               will nonetheless still work correctly for the main
//               physics simulation.
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_max_bounds(PhysxBounds3 &bounds) {

  _desc.maxBounds = &(bounds._bounds);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_max_bounds
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxBounds3 PhysxSceneDesc::
get_max_bounds() const {

  throw "Not Implemented";

  //PhysxBounds3 value;
  //value._bounds = *(_desc.maxBounds);
  //return value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_static_structure
//       Access: Published
//  Description: Defines the structure used to store static
//               objects.
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_static_structure(PhysxPruningStructure value) {

  _desc.staticStructure = (NxPruningStructure)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_dynamic_structure
//       Access: Published
//  Description: Defines the subdivision level for acceleration
//               structures used for scene queries.
//               This is only used when maxBounds are defined!
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_dynamic_structure(PhysxPruningStructure value) {

  _desc.dynamicStructure = (NxPruningStructure)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_static_structure
//       Access: Published
//  Description: Returns the structure used to store static
//               objects.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxPruningStructure PhysxSceneDesc::
get_static_structure() const {

  return (PhysxPruningStructure)_desc.staticStructure;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_dynamic_structure
//       Access: Published
//  Description: Returns the subdivision level for acceleration
//               structures used for scene queries.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxPruningStructure PhysxSceneDesc::
get_dynamic_structure() const {

  return (PhysxPruningStructure)_desc.dynamicStructure;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_subdivision_level
//       Access: Published
//  Description: Defines the subdivision level for acceleration
//               structures used for scene queries. 
//               This is only used when maxBounds are defined!
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_subdivision_level(unsigned int value) {

  _desc.subdivisionLevel = (NxU32)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_num_grid_cells_x
//       Access: Published
//  Description: Defines the number of broadphase cells along the
//               grid x-axis. Must be power of two. Max is 8 at the
//               moment. The broadphase type must be set to
//               BPT_sap_multi for this parameter to have
//               an effect.
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_num_grid_cells_x(unsigned int value) {

  _desc.nbGridCellsX = (NxU32)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_num_grid_cells_y
//       Access: Published
//  Description: Defines the number of broadphase cells along the
//               grid y-axis. Must be power of two. Max is 8 at the
//               moment. The broadphase type must be set to
//               BPT_sap_multi for this parameter to have
//               an effect.
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_num_grid_cells_y(unsigned int value) {

  _desc.nbGridCellsY = (NxU32)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_subdivision_level
//       Access: Published
//  Description: Returns the subdivision level for acceleration
//               structures used for scene queries. 
////////////////////////////////////////////////////////////////////
unsigned int PhysxSceneDesc::
get_subdivision_level() const {

  return _desc.subdivisionLevel;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_num_grid_cells_x
//       Access: Published
//  Description: Returns the number of broadphase cells along the
//               grid x-axis.
////////////////////////////////////////////////////////////////////
unsigned int PhysxSceneDesc::
get_num_grid_cells_x() const {

  return _desc.nbGridCellsX;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_num_grid_cells_y
//       Access: Published
//  Description: Returns the number of broadphase cells along the
//               grid y-axis.
////////////////////////////////////////////////////////////////////
unsigned int PhysxSceneDesc::
get_num_grid_cells_y() const {

  return _desc.nbGridCellsY;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::set_bp_type
//       Access: Published
//  Description: Defines which type of broadphase to use.
//
//               (1) BPT_sap_single: A sweep-and-prune (SAP)
//               algorithm to find pairs of potentially colliding
//               shapes.
//
//               (2) BPT_sap_multi: A multi sweep-and-prune
//               algorithm to find pairs of potentially colliding
//               shapes. Uses a configurable 2D grid to divide the
//               scene space into cells. The potentially overlapping
//               shape pairs are detected in each cell and the
//               information is merged together. This approach is
//               usually faster than BPT_sap_single in scenarios
//               with many shapes and a high creation/deletion rate
//               of shapes. However, the amount of memory required
//               is considerably higher depending on the number of
//               grid cells used.
//               The following extra parameters need to be defined: 
//               - PhysxSceneDesc.set_max_bounds
//               - PhysxSceneDesc.set_num_grid_cells_x
//               - PhysxSceneDesc.set_num_grid_cells_y
//               (the scene up direction is set via config options)
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_bp_type(PhysxBroadPhaseType value) {

  _desc.bpType = (NxBroadPhaseType)value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSceneDesc::get_bp_type
//       Access: Published
//  Description: Returns the type of broadphase to use.
////////////////////////////////////////////////////////////////////
PhysxEnums::PhysxBroadPhaseType PhysxSceneDesc::
get_bp_type() const {

  return (PhysxBroadPhaseType)_desc.bpType;
}

