// Filename: physxActorNode.I
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

////////////////////////////////////////////////////////////////////
//     Function : update_transform
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE void PhysxActorNode::
update_transform() {
  _disable_transform_changed = true;
  get_global_pose_optimized(&_tempMat4);
  set_transform(TransformState::make_mat(_tempMat4));
  _disable_transform_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_pose_optimized
//       Access : Published
//  Description : This is an optimized version of get_global_pose that
//                copies the pose data to a passed matrix rather than
//                returning a new matrix, and also optimizes the
//                translation from PhysX matrix to Panda matrix.
//                It is primarily intended to be used by the
//                update_transform method, which is called in a tight
//                loop when transforms are copied from the PhysX
//                library to Panda.
////////////////////////////////////////////////////////////////////
INLINE void PhysxActorNode::
get_global_pose_optimized(LMatrix4f *result) {
  nassertv(nActor != NULL);

  nActor->getGlobalPose().getColumnMajor44(_tempCells);
  result->set(
    _tempCells[0],  _tempCells[1],  _tempCells[2],  _tempCells[3],
    _tempCells[4],  _tempCells[5],  _tempCells[6],  _tempCells[7],
    _tempCells[8],  _tempCells[9],  _tempCells[10], _tempCells[11],
    _tempCells[12], _tempCells[13], _tempCells[14], _tempCells[15]
  );
}

////////////////////////////////////////////////////////////////////
//     Function : get_global_pose
//       Access : Published
//  Description :
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f PhysxActorNode::
get_global_pose() const {
  nassertr(nActor != NULL, *((LMatrix4f *)NULL));

  return PhysxManager::nxMat34_to_lMatrix4(nActor->getGlobalPose());
}

////////////////////////////////////////////////////////////////////
//     Function : is_valid
//       Access : Published
//  Description : Returns true if the NxActor associated with this
//                ActorNode is not NULL.  The NxActor will be NULL if
//                the ActorNode has been released.
////////////////////////////////////////////////////////////////////
INLINE bool PhysxActorNode::
is_valid() {
  return nActor != NULL;
}




