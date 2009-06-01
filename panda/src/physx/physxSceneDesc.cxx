// Filename: physxSceneDesc.cxx
// Created by:  pratt (Apr 7, 2006)
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

#ifdef HAVE_PHYSX

#include "physxSceneDesc.h"

#include "luse.h"
#include "physxBounds3.h"


////////////////////////////////////////////////////////////////////
//     Function : PhysxSceneDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSceneDesc::
PhysxSceneDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function : get_dynamic_structure
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPruningStructure PhysxSceneDesc::
get_dynamic_structure() const {
  return (PhysxPruningStructure)nSceneDesc.dynamicStructure;
}

////////////////////////////////////////////////////////////////////
//     Function : get_gravity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxSceneDesc::
get_gravity() const {
  return PhysxManager::nxVec3_to_lVecBase3(nSceneDesc.gravity);
}

////////////////////////////////////////////////////////////////////
//     Function : get_max_bounds
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxBounds3 * PhysxSceneDesc::
get_max_bounds() const {
  throw "Not Implemented"; // return nSceneDesc.maxBounds;
}

////////////////////////////////////////////////////////////////////
//     Function : get_sim_thread_priority
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxThreadPriority PhysxSceneDesc::
get_sim_thread_priority() const {
  return (PhysxThreadPriority)nSceneDesc.simThreadPriority;
}

////////////////////////////////////////////////////////////////////
//     Function : get_sim_type
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxSimulationType PhysxSceneDesc::
get_sim_type() const {
  return (PhysxSimulationType)nSceneDesc.simType;
}

////////////////////////////////////////////////////////////////////
//     Function : get_static_structure
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxPruningStructure PhysxSceneDesc::
get_static_structure() const {
  return (PhysxPruningStructure)nSceneDesc.staticStructure;
}

////////////////////////////////////////////////////////////////////
//     Function : get_time_step_method
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxTimeStepMethod PhysxSceneDesc::
get_time_step_method() const {
  return (PhysxTimeStepMethod)nSceneDesc.timeStepMethod;
}

////////////////////////////////////////////////////////////////////
//     Function : get_worker_thread_priority
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxThreadPriority PhysxSceneDesc::
get_worker_thread_priority() const {
  return (PhysxThreadPriority)nSceneDesc.workerThreadPriority;
}

////////////////////////////////////////////////////////////////////
//     Function : set_dynamic_structure
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_dynamic_structure(PhysxPruningStructure value) {
  nSceneDesc.dynamicStructure = (NxPruningStructure)value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_gravity
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_gravity(LVecBase3f value) {
  nSceneDesc.gravity = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_max_bounds
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_max_bounds(PhysxBounds3 * value) {
  nSceneDesc.maxBounds = value->nBounds3;
}

////////////////////////////////////////////////////////////////////
//     Function : set_sim_thread_priority
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_sim_thread_priority(PhysxThreadPriority value) {
  nSceneDesc.simThreadPriority = (NxThreadPriority)value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_sim_type
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_sim_type(PhysxSimulationType value) {
  nSceneDesc.simType = (NxSimulationType)value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_static_structure
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_static_structure(PhysxPruningStructure value) {
  nSceneDesc.staticStructure = (NxPruningStructure)value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_time_step_method
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_time_step_method(PhysxTimeStepMethod value) {
  nSceneDesc.timeStepMethod = (NxTimeStepMethod)value;
}

////////////////////////////////////////////////////////////////////
//     Function : set_worker_thread_priority
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxSceneDesc::
set_worker_thread_priority(PhysxThreadPriority value) {
  nSceneDesc.workerThreadPriority = (NxThreadPriority)value;
}

#endif // HAVE_PHYSX

