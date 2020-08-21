/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animPreloadTable.I
 * @author drose
 * @date 2008-08-05
 */

/**
 *
 */
INLINE AnimPreloadTable::AnimRecord::
AnimRecord() {
}

/**
 *
 */
INLINE bool AnimPreloadTable::AnimRecord::
operator < (const AnimRecord &other) const {
  return _basename < other._basename;
}

/**
 * Returns the basename stored for the nth animation record.  See find_anim().
 */
INLINE std::string AnimPreloadTable::
get_basename(int n) const {
  nassertr(n >= 0 && n < (int)_anims.size(), std::string());
  consider_sort();
  return _anims[n]._basename;
}

/**
 * Returns the frame rate stored for the nth animation record.
 */
INLINE PN_stdfloat AnimPreloadTable::
get_base_frame_rate(int n) const {
  nassertr(n >= 0 && n < (int)_anims.size(), 0.0f);
  consider_sort();
  return _anims[n]._base_frame_rate;
}

/**
 * Returns the number of frames stored for the nth animation record.
 */
INLINE int AnimPreloadTable::
get_num_frames(int n) const {
  nassertr(n >= 0 && n < (int)_anims.size(), 0);
  consider_sort();
  return _anims[n]._num_frames;
}

/**
 * Ensures the table is kept in alphabetical order by basename.
 */
INLINE void AnimPreloadTable::
consider_sort() const {
  if (_needs_sort) {
    ((AnimPreloadTable *)this)->_anims.sort();
    ((AnimPreloadTable *)this)->_needs_sort = false;
  }
}
