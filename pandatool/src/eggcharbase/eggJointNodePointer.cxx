// Filename: eggJointNodePointer.cxx
// Created by:  drose (26Feb01)
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

#include "eggJointNodePointer.h"

#include "dcast.h"
#include "eggObject.h"
#include "eggGroup.h"
#include "pointerTo.h"


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

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::add_rebuild_frame
//       Access: Public, Virtual
//  Description: Adds a new frame to the set of rebuild frames.  See
//               begin_rebuild() and do_rebuild().  Returns true if
//               this is valid, false otherwise (e.g. adding multiple
//               frames to a static joint).
////////////////////////////////////////////////////////////////////
bool EggJointNodePointer::
add_rebuild_frame(const LMatrix4d &mat) {
  if (!_rebuild_frames.empty()) {
    // Only one frame may be added to a <Joint>.
    return false;
  }
  return EggJointPointer::add_rebuild_frame(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::do_rebuild
//       Access: Public, Virtual
//  Description: Rebuilds the entire table all at once, based on the
//               frames added by repeated calls to add_rebuild_frame()
//               since the last call to begin_rebuild().
//
//               Until do_rebuild() is called, the animation table is
//               not changed.
//
//               The return value is true if all frames are
//               acceptable, or false if there is some problem.
////////////////////////////////////////////////////////////////////
bool EggJointNodePointer::
do_rebuild() {
  if (_rebuild_frames.empty()) {
    return true;
  }

  if (_rebuild_frames.size() != 1) {
    return false;
  }

  _joint->set_transform(_rebuild_frames[0]);
  _rebuild_frames.clear();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointNodePointer::has_vertices
//       Access: Public, Virtual
//  Description: Returns true if there are any vertices referenced by
//               the node this points to, false otherwise.  For
//               certain kinds of back pointers (e.g. table animation
//               entries), this is always false.
////////////////////////////////////////////////////////////////////
bool EggJointNodePointer::
has_vertices() const {
  if (_joint != (EggGroup *)NULL) {
    return (_joint->vref_size() != 0);
  }

  return false;
}
