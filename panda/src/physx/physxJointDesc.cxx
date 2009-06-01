// Filename: physxJointDesc.cxx
// Created by:  pratt (Jun 20, 2006)
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

#include "physxJointDesc.h"

#include "luse.h"
#include "physxActorNode.h"

////////////////////////////////////////////////////////////////////
//     Function : PhysxJointDesc
//       Access : protected
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJointDesc::
PhysxJointDesc(NxJointDesc *subJointDesc) {
  nJointDesc = subJointDesc;
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysxJointDesc
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
PhysxJointDesc::
~PhysxJointDesc() {

}

////////////////////////////////////////////////////////////////////
//     Function : set_global_anchor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_global_anchor(const LVecBase3f & ws_anchor) {
  nassertv(nJointDesc != NULL);

  nJointDesc->setGlobalAnchor(PhysxManager::lVecBase3_to_nxVec3(ws_anchor));
}

////////////////////////////////////////////////////////////////////
//     Function : set_global_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_global_axis(const LVecBase3f & ws_axis) {
  nassertv(nJointDesc != NULL);

  nJointDesc->setGlobalAxis(PhysxManager::lVecBase3_to_nxVec3(ws_axis));
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_anchor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxJointDesc::
get_local_anchor(int index) const {
  nassertr(nJointDesc != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nJointDesc->localAnchor[index]);
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxJointDesc::
get_local_axis(int index) const {
  nassertr(nJointDesc != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nJointDesc->localAxis[index]);
}

////////////////////////////////////////////////////////////////////
//     Function : get_local_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
LVecBase3f PhysxJointDesc::
get_local_normal(int index) const {
  nassertr(nJointDesc != NULL, *((LVecBase3f *)NULL));

  return PhysxManager::nxVec3_to_lVecBase3(nJointDesc->localNormal[index]);
}

////////////////////////////////////////////////////////////////////
//     Function : set_actor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_actor(int index, PhysxActorNode * value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->actor[index] = value->nActor;
}

////////////////////////////////////////////////////////////////////
//     Function : set_local_anchor
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_local_anchor(int index, LVecBase3f value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->localAnchor[index] = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_local_axis
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_local_axis(int index, LVecBase3f value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->localAxis[index] = PhysxManager::lVecBase3_to_nxVec3(value);
}

////////////////////////////////////////////////////////////////////
//     Function : set_local_normal
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
void PhysxJointDesc::
set_local_normal(int index, LVecBase3f value) {
  nassertv(nJointDesc != NULL);

  nJointDesc->localNormal[index] = PhysxManager::lVecBase3_to_nxVec3(value);
}

#endif // HAVE_PHYSX

