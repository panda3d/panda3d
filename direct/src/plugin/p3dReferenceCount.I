/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dReferenceCount.I
 * @author drose
 * @date 2009-07-09
 */

/**

 */
inline P3DReferenceCount::
P3DReferenceCount() {
  _ref_count = 0;
}

/**

 */
inline P3DReferenceCount::
~P3DReferenceCount() {
  assert(_ref_count == 0);
}

/**
 * Explicitly increments the reference count.
 */
inline void P3DReferenceCount::
ref() const {
  ++((P3DReferenceCount *)this)->_ref_count;
}

/**
 * Explicitly decrements the reference count.  Usually, you should call
 * p3d_unref_delete() instead.  The return value is true if the new reference
 * count is nonzero, false if it is zero.
 */
inline bool P3DReferenceCount::
unref() const {
  return --(((P3DReferenceCount *)this)->_ref_count) != 0;
}

/**
 * Returns the current reference count.
 */
inline int P3DReferenceCount::
get_ref_count() const {
  return _ref_count;
}

/**
 * This global helper function will unref the given P3DReferenceCount object,
 * and if the reference count reaches zero, automatically delete it.
 */
template<class RefCountType>
inline void
p3d_unref_delete(RefCountType *ptr) {
  if (!ptr->unref()) {
    delete ptr;
  }
}
