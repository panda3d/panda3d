// Filename: partBundle.I
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::set_blend_type
//       Access: Published
//  Description: Defines the algorithm that is used when blending
//               multiple frames or multiple animations together, when
//               either anim_blend_flag or frame_blend_flag is set
//               to true.
//
//               See partBundle.h for a description of the meaning of
//               each of the BlendType values.
////////////////////////////////////////////////////////////////////
INLINE void PartBundle::
set_blend_type(PartBundle::BlendType bt) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_blend_type = bt;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_blend_type
//       Access: Published
//  Description: Returns the algorithm that is used when blending
//               multiple frames or multiple animations together, when
//               either anim_blend_flag or frame_blend_flag is set
//               to true.
////////////////////////////////////////////////////////////////////
INLINE PartBundle::BlendType PartBundle::
get_blend_type() const {
  CDReader cdata(_cycler);
  return cdata->_blend_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_anim_blend_flag
//       Access: Published
//  Description: Returns whether the character allows multiple
//               different animations to be bound simultaneously.  See
//               set_anim_blend_flag().
////////////////////////////////////////////////////////////////////
INLINE bool PartBundle::
get_anim_blend_flag() const {
  CDReader cdata(_cycler);
  return cdata->_anim_blend_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::set_frame_blend_flag
//       Access: Published
//  Description: Specifies whether the character interpolates (blends)
//               between two sequential frames of an active animation,
//               showing a smooth intra-frame motion, or whether it
//               holds each frame until the next frame is ready,
//               showing precisely the specified animation.
//
//               When this value is false, the character holds each
//               frame until the next is ready.  When this is true,
//               the character will interpolate between two
//               consecutive frames of animation for each frame the
//               animation is onscreen, according to the amount of
//               time elapsed between the frames.
//
//               The default value of this flag is determined by the
//               interpolate-frames Config.prc variable.
//
//               Use set_blend_type() to change the algorithm that the
//               character uses to interpolate matrix positions.
////////////////////////////////////////////////////////////////////
INLINE void PartBundle::
set_frame_blend_flag(bool frame_blend_flag) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_frame_blend_flag = frame_blend_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_frame_blend_flag
//       Access: Published
//  Description: Returns whether the character interpolates (blends)
//               between two sequential animation frames, or whether
//               it holds the current frame until the next one is
//               ready.  See set_frame_blend_flag().
////////////////////////////////////////////////////////////////////
INLINE bool PartBundle::
get_frame_blend_flag() const {
  CDReader cdata(_cycler);
  return cdata->_frame_blend_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_node
//       Access: Published
//  Description: Returns the PartBundleNode associated with this
//               PartBundle.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode *PartBundle::
get_node() const {
  return _node;
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::set_control_effect
//       Access: Published
//  Description: Sets the amount by which the character is affected by
//               the indicated AnimControl (and its associated
//               animation).  Normally, this will only be zero or one.
//               Zero indicates the animation does not affect the
//               character, and one means it does.
//
//               If the _anim_blend_flag is not false (see
//               set_anim_blend_flag()), it is possible to have
//               multiple AnimControls in effect simultaneously.  In
//               this case, the effect is a weight that indicates the
//               relative importance of each AnimControl to the final
//               animation.
////////////////////////////////////////////////////////////////////
void PartBundle::
set_control_effect(AnimControl *control, float effect) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDWriter cdata(_cycler);
  do_set_control_effect(control, effect, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_control_effect
//       Access: Published
//  Description: Returns the amount by which the character is affected
//               by the indicated AnimControl and its associated
//               animation.  See set_control_effect().
////////////////////////////////////////////////////////////////////
INLINE float PartBundle::
get_control_effect(AnimControl *control) const {
  CDReader cdata(_cycler);
  return do_get_control_effect(control, cdata);
}

