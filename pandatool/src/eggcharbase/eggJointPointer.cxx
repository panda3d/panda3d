/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggJointPointer.cxx
 * @author drose
 * @date 2001-02-26
 */

#include "eggJointPointer.h"


TypeHandle EggJointPointer::_type_handle;


/**
 * Appends a new frame onto the end of the data, if possible; returns true if
 * not possible, or false otherwise (e.g.  for a static joint).
 */
bool EggJointPointer::
add_frame(const LMatrix4d &) {
  return false;
}

/**
 * Moves the vertices assigned to this joint into the other joint (which
 * should be of the same type).
 */
void EggJointPointer::
move_vertices_to(EggJointPointer *) {
}

/**
 * Rebuilds the entire table all at once, based on the frames added by
 * repeated calls to add_rebuild_frame() since the last call to
 * begin_rebuild().
 *
 * Until do_rebuild() is called, the animation table is not changed.
 *
 * The return value is true if all frames are acceptable, or false if there is
 * some problem.
 */
bool EggJointPointer::
do_rebuild(EggCharacterDb &db) {
  return true;
}

/**
 * Resets the table before writing to disk so that redundant rows (e.g.  i { 1
 * 1 1 1 1 1 1 1 }) are collapsed out.
 */
void EggJointPointer::
optimize() {
}

/**
 * Flags the joint with the indicated DCS flag so that it will be loaded as a
 * separate node in the player.
 */
void EggJointPointer::
expose(EggGroup::DCSType) {
}

/**
 * Zeroes out the named components of the transform in the animation frames.
 */
void EggJointPointer::
zero_channels(const std::string &) {
}

/**
 * Rounds the named components of the transform to the nearest multiple of
 * quantum.
 */
void EggJointPointer::
quantize_channels(const std::string &, double) {
}

/**
 * Applies the pose from the indicated frame of the indicated source joint as
 * the initial pose for this joint.
 */
void EggJointPointer::
apply_default_pose(EggJointPointer *source_joint, int frame) {
}
