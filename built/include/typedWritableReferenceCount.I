/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedWritableReferenceCount.I
 * @author jason
 * @date 2000-06-08
 */

/**
 *
 */
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

/**
 *
 */
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount(const TypedWritableReferenceCount &copy) :
  TypedWritable(copy),
  ReferenceCount(copy) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

/**
 *
 */
INLINE void TypedWritableReferenceCount::
operator = (const TypedWritableReferenceCount &copy) {
  TypedWritable::operator = (copy);
  ReferenceCount::operator = (copy);
}
