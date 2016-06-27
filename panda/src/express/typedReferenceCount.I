/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedReferenceCount.I
 * @author drose
 * @date 2000-05-25
 */

/**
 *
 */
INLINE TypedReferenceCount::
TypedReferenceCount() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

/**
 *
 */
INLINE TypedReferenceCount::
TypedReferenceCount(const TypedReferenceCount &copy) :
  TypedObject(copy),
  ReferenceCount(copy)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
}

/**
 *
 */
INLINE void TypedReferenceCount::
operator = (const TypedReferenceCount &copy) {
  TypedObject::operator = (copy);
  ReferenceCount::operator = (copy);
}
