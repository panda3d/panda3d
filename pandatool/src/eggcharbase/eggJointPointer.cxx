// Filename: eggJointPointer.cxx
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

#include "eggJointPointer.h"


TypeHandle EggJointPointer::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::add_frame
//       Access: Public, Virtual
//  Description: Appends a new frame onto the end of the data, if
//               possible; returns true if not possible, or false
//               otherwise (e.g. for a static joint).
////////////////////////////////////////////////////////////////////
bool EggJointPointer::
add_frame(const LMatrix4d &) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::move_vertices_to
//       Access: Public, Virtual
//  Description: Moves the vertices assigned to this joint into the
//               other joint (which should be of the same type).
////////////////////////////////////////////////////////////////////
void EggJointPointer::
move_vertices_to(EggJointPointer *) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::begin_rebuild
//       Access: Public
//  Description: Resets the set of rebuild frames in preparation for
//               rebuilding the complete table of frames.  Repeated
//               calls to add_rebuild_frame() will build up the frames
//               without changing the values returned by get_frame();
//               the table will eventually be updated when do_rebuild
//               is called.
////////////////////////////////////////////////////////////////////
void EggJointPointer::
begin_rebuild() {
  _rebuild_frames.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::add_rebuild_frame
//       Access: Public, Virtual
//  Description: Adds a new frame to the set of rebuild frames.  See
//               begin_rebuild() and do_rebuild().  Returns true if
//               this is valid, false otherwise (e.g. adding multiple
//               frames to a static joint).
////////////////////////////////////////////////////////////////////
bool EggJointPointer::
add_rebuild_frame(const LMatrix4d &mat) {
  _rebuild_frames.push_back(mat);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::do_rebuild
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
bool EggJointPointer::
do_rebuild() {
  if (_rebuild_frames.empty()) {
    return true;
  }
  _rebuild_frames.clear();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::optimize
//       Access: Public, Virtual
//  Description: Resets the table before writing to disk so that
//               redundant rows (e.g. i { 1 1 1 1 1 1 1 1 }) are
//               collapsed out.
////////////////////////////////////////////////////////////////////
void EggJointPointer::
optimize() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggJointPointer::expose
//       Access: Public, Virtual
//  Description: Flags the joint with the indicated DCS flag so that
//               it will be loaded as a separate node in the player.
////////////////////////////////////////////////////////////////////
void EggJointPointer::
expose(EggGroup::DCSType) {
}
