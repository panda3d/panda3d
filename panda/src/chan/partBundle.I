/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundle.I
 * @author drose
 * @date 1999-02-22
 */

/**
 * Returns the AnimPreloadTable associated with the PartBundle.  This table,
 * if present, can be used for the benefit of load_bind_anim() to allow
 * asynchronous binding.
 */
INLINE CPT(AnimPreloadTable) PartBundle::
get_anim_preload() const {
  return _anim_preload.get_read_pointer();
}

/**
 * Returns a modifiable pointer to the AnimPreloadTable associated with the
 * PartBundle, if any.
 */
INLINE PT(AnimPreloadTable) PartBundle::
modify_anim_preload() {
  return _anim_preload.get_write_pointer();
}

/**
 * Replaces the AnimPreloadTable associated with the PartBundle.
 */
INLINE void PartBundle::
set_anim_preload(AnimPreloadTable *anim_preload) {
  _anim_preload = anim_preload;
}

/**
 * Removes any AnimPreloadTable associated with the PartBundle.
 */
INLINE void PartBundle::
clear_anim_preload() {
  _anim_preload = nullptr;
}

/**
 * Defines the algorithm that is used when blending multiple frames or
 * multiple animations together, when either anim_blend_flag or
 * frame_blend_flag is set to true.
 *
 * See partBundle.h for a description of the meaning of each of the BlendType
 * values.
 */
INLINE void PartBundle::
set_blend_type(PartBundle::BlendType bt) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_blend_type = bt;
}

/**
 * Returns the algorithm that is used when blending multiple frames or
 * multiple animations together, when either anim_blend_flag or
 * frame_blend_flag is set to true.
 */
INLINE PartBundle::BlendType PartBundle::
get_blend_type() const {
  CDReader cdata(_cycler);
  return cdata->_blend_type;
}

/**
 * Returns whether the character allows multiple different animations to be
 * bound simultaneously.  See set_anim_blend_flag().
 */
INLINE bool PartBundle::
get_anim_blend_flag() const {
  CDReader cdata(_cycler);
  return cdata->_anim_blend_flag;
}

/**
 * Specifies whether the character interpolates (blends) between two
 * sequential frames of an active animation, showing a smooth intra-frame
 * motion, or whether it holds each frame until the next frame is ready,
 * showing precisely the specified animation.
 *
 * When this value is false, the character holds each frame until the next is
 * ready.  When this is true, the character will interpolate between two
 * consecutive frames of animation for each frame the animation is onscreen,
 * according to the amount of time elapsed between the frames.
 *
 * The default value of this flag is determined by the interpolate-frames
 * Config.prc variable.
 *
 * Use set_blend_type() to change the algorithm that the character uses to
 * interpolate matrix positions.
 */
INLINE void PartBundle::
set_frame_blend_flag(bool frame_blend_flag) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_frame_blend_flag = frame_blend_flag;
}

/**
 * Returns whether the character interpolates (blends) between two sequential
 * animation frames, or whether it holds the current frame until the next one
 * is ready.  See set_frame_blend_flag().
 */
INLINE bool PartBundle::
get_frame_blend_flag() const {
  CDReader cdata(_cycler);
  return cdata->_frame_blend_flag;
}

/**
 * Specifies the transform matrix which is implicitly applied at the root of
 * the animated hierarchy.
 */
INLINE void PartBundle::
set_root_xform(const LMatrix4 &root_xform) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_root_xform = root_xform;
  cdata->_anim_changed = true;
}

/**
 * Applies the indicated transform to the root of the animated hierarchy.
 */
INLINE void PartBundle::
xform(const LMatrix4 &mat) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  CDWriter cdata(_cycler);
  cdata->_root_xform = cdata->_root_xform * mat;
  do_xform(mat, invert(mat));
  cdata->_anim_changed = true;
}

/**
 * Returns the transform matrix which is implicitly applied at the root of the
 * animated hierarchy.
 */
INLINE const LMatrix4 &PartBundle::
get_root_xform() const {
  CDReader cdata(_cycler);
  return cdata->_root_xform;
}

/**
 * Returns the number of PartBundleNodes that contain a pointer to this
 * PartBundle.
 */
INLINE int PartBundle::
get_num_nodes() const {
  return _nodes.size();
}

/**
 * Returns the nth PartBundleNode associated with this PartBundle.
 */
INLINE PartBundleNode *PartBundle::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_nodes.size(), nullptr);
  return _nodes[n];
}


/**
 * Sets the amount by which the character is affected by the indicated
 * AnimControl (and its associated animation).  Normally, this will only be
 * zero or one.  Zero indicates the animation does not affect the character,
 * and one means it does.
 *
 * If the _anim_blend_flag is not false (see set_anim_blend_flag()), it is
 * possible to have multiple AnimControls in effect simultaneously.  In this
 * case, the effect is a weight that indicates the relative importance of each
 * AnimControl to the final animation.
 */
void PartBundle::
set_control_effect(AnimControl *control, PN_stdfloat effect) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDWriter cdata(_cycler);
  do_set_control_effect(control, effect, cdata);
}

/**
 * Returns the amount by which the character is affected by the indicated
 * AnimControl and its associated animation.  See set_control_effect().
 */
INLINE PN_stdfloat PartBundle::
get_control_effect(AnimControl *control) const {
  CDReader cdata(_cycler);
  return do_get_control_effect(control, cdata);
}

/**
 * Specifies the minimum amount of time, in seconds, that should elapse
 * between any two consecutive updates.  This is normally used by
 * Character::set_lod_animation(), and should not be called directly.
 */
INLINE void PartBundle::
set_update_delay(double delay) {
  _update_delay = delay;
}
