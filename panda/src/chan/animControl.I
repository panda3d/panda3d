/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animControl.I
 * @author drose
 * @date 1999-02-19
 */

/**
 * Returns true if the AnimControl is being bound asynchronously, and has not
 * yet finished.  If this is true, the AnimControl's interface is still
 * available and will be perfectly useful (though get_anim() might return
 * NULL), but nothing visible will happen immediately.
 */
INLINE bool AnimControl::
is_pending() const {
  return _pending;
}

/**
 * Returns true if the AnimControl was successfully loaded, or false if there
 * was a problem.  This may return false while is_pending() is true.
 */
INLINE bool AnimControl::
has_anim() const {
  return (_anim != nullptr);
}

/**
 * Returns the AnimBundle bound in with this AnimControl.
 */
INLINE AnimBundle *AnimControl::
get_anim() const {
  return _anim;
}

/**
 * Returns the particular channel index associated with this AnimControl.
 * This channel index is the slot on which each AnimGroup is bound to its
 * associated PartGroup, for each joint in the animation.
 *
 * It will be true that
 * get_part()->find_child("n")->get_bound(get_channel_index()) ==
 * get_anim()->find_child("n"), for each joint "n".
 */
INLINE int AnimControl::
get_channel_index() const {
  return _channel_index;
}

/**
 * Returns the subset of joints controlled by this AnimControl.  Most of the
 * time, this will be BitArray::all_on(), for a normal full-body animation.
 * For a subset animation, however, this will be just a subset of those bits,
 * corresponding to the set of joints and sliders actually bound (as
 * enumerated by bind_hierarchy() in depth-first LIFO order).
 */
INLINE const BitArray &AnimControl::
get_bound_joints() const {
  return _bound_joints;
}

/**
 * Associates the indicated PandaNode with the AnimControl.  By convention,
 * this node represents the root node of the model file that corresponds to
 * this AnimControl's animation file, though nothing in this code makes this
 * assumption or indeed does anything with this node.
 *
 * The purpose of this is simply to allow the AnimControl to keep a reference
 * count on the ModelRoot node that generated it, so that the model will not
 * disappear from the model pool until it is no longer referenced.
 */
INLINE void AnimControl::
set_anim_model(PandaNode *model) {
  _anim_model = model;
}

/**
 * Retrieves the pointer set via set_anim_model().  See set_anim_model().
 */
INLINE PandaNode *AnimControl::
get_anim_model() const {
  return _anim_model;
}

INLINE std::ostream &
operator << (std::ostream &out, const AnimControl &control) {
  control.output(out);
  return out;
}
