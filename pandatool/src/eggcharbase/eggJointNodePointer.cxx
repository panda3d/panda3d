// Filename: eggJointNodePointer.cxx
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggJointNodePointer.h"

#include <eggObject.h>
#include <eggGroup.h>
#include <pointerTo.h>


TypeHandle EggJointNodePointer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggJointNodePointer::
EggJointNodePointer(EggObject *object) {
  _joint = DCAST(EggGroup, object);

  if (_joint != (EggGroup *)NULL) {
    // Quietly insist that the joint has a transform, for neatness.  If
    // it does not, give it the identity transform.
    if (!_joint->has_transform()) {
      _joint->set_transform(LMatrix4d::ident_mat());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::get_num_frames
//       Access: Public, Virtual
//  Description: Returns the number of frames of animation for this
//               particular joint.
//
//               In the case of a EggJointNodePointer, which just
//               stores a pointer to a <Joint> entry for a character
//               model (not an animation table), there is always
//               exactly one frame: the rest pose.
////////////////////////////////////////////////////////////////////
int EggJointNodePointer::
get_num_frames() const {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::get_frame
//       Access: Public, Virtual
//  Description: Returns the transform matrix corresponding to this
//               joint position in the nth frame.
//
//               In the case of a EggJointNodePointer, which just
//               stores a pointer to a <Joint> entry for a character
//               model (not an animation table), there is always
//               exactly one frame: the rest pose.
////////////////////////////////////////////////////////////////////
LMatrix4d EggJointNodePointer::
get_frame(int n) const {
  nassertr(n == 0, LMatrix4d::ident_mat());
  return _joint->get_transform();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::set_frame
//       Access: Public, Virtual
//  Description: Sets the transform matrix corresponding to this
//               joint position in the nth frame.
//
//               In the case of a EggJointNodePointer, which just
//               stores a pointer to a <Joint> entry for a character
//               model (not an animation table), there is always
//               exactly one frame: the rest pose.
////////////////////////////////////////////////////////////////////
void EggJointNodePointer::
set_frame(int n, const LMatrix4d &mat) {
  nassertv(n == 0);
  _joint->set_transform(mat);
}
