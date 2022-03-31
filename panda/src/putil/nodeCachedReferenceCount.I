/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodeCachedReferenceCount.I
 * @author drose
 * @date 2005-05-07
 */

/**
 * The ReferenceCount constructor is protected because you almost never want
 * to create just a ReferenceCount object by itself, and it's probably a
 * mistake if you try.
 *
 * ReferenceCount doesn't store any useful information in its own right; its
 * only purpose is to add reference-counting to some other class via
 * inheritance.
 */
INLINE NodeCachedReferenceCount::
NodeCachedReferenceCount() {
  _node_ref_count = 0;
}

/**
 * The copies of reference-counted objects do not themselves inherit the
 * reference count!
 *
 * This copy constructor is protected because you almost never want to create
 * just a ReferenceCount object by itself, and it's probably a mistake if you
 * try.
 */
INLINE NodeCachedReferenceCount::
NodeCachedReferenceCount(const NodeCachedReferenceCount &copy) : CachedTypedWritableReferenceCount(copy) {
  _node_ref_count = 0;
}

/**
 * The copies of reference-counted objects do not themselves inherit the
 * reference count!
 *
 * This copy assignment operator is protected because you almost never want to
 * copy just a ReferenceCount object by itself, and it's probably a mistake if
 * you try.  Instead, this should only be called from a derived class that
 * implements this operator and then calls up the inheritance chain.
 */
INLINE void NodeCachedReferenceCount::
operator = (const NodeCachedReferenceCount &copy) {
  // If this assertion fails, our own pointer was recently deleted.  Possibly
  // you used a real pointer instead of a PointerTo at some point, and the
  // object was deleted when the PointerTo went out of scope.  Maybe you tried
  // to create an automatic (local variable) instance of a class that derives
  // from ReferenceCount.  Or maybe your headers are out of sync, and you need
  // to make clean in direct or some higher tree.
  nassertv(_node_ref_count != -100);

  CachedTypedWritableReferenceCount::operator = (copy);
}

/**
 * The ReferenceCount destructor is protected to discourage users from
 * accidentally trying to delete a ReferenceCount pointer directly.  This is
 * almost always a bad idea, since the destructor is not virtual, and you've
 * almost certainly got some pointer to something that inherits from
 * ReferenceCount, not just a plain old ReferenceCount object.
 */
INLINE NodeCachedReferenceCount::
~NodeCachedReferenceCount() {
  // If this assertion fails, we're trying to delete an object that was just
  // deleted.  Possibly you used a real pointer instead of a PointerTo at some
  // point, and the object was deleted when the PointerTo went out of scope.
  // Maybe you tried to create an automatic (local variable) instance of a
  // class that derives from ReferenceCount.  Or maybe your headers are out of
  // sync, and you need to make clean in direct or some higher tree.
  nassertv(_node_ref_count != -100);

  // If this assertion fails, the reference counts are all screwed up
  // altogether.  Maybe some errant code stomped all over memory somewhere.
  nassertv(_node_ref_count >= 0);

  // If this assertion fails, someone tried to delete this object while its
  // reference count was still positive.  Maybe you tried to point a PointerTo
  // at a static object (a local variable, instead of one allocated via new)?
  // The test below against 0x7f is supposed to check for that, but it's a
  // pretty hokey test.

  // Another possibility is you inadvertently omitted a copy constructor for a
  // ReferenceCount object, and then bitwise copied a dynamically allocated
  // value--reference count and all--onto a locally allocated one.
  nassertv(_node_ref_count == 0);

#ifndef NDEBUG
  // Ok, all clear to delete.  Now set the reference count to -100, so we'll
  // have a better chance of noticing if we happen to have a stray pointer to
  // it still out there.
  _node_ref_count = -100;
#endif
}

/**
 * Returns the current reference count.
 */
INLINE int NodeCachedReferenceCount::
get_node_ref_count() const {
#ifdef _DEBUG
  test_ref_count_integrity();
#endif
  return (int)AtomicAdjust::get(_node_ref_count);
}

/**
 * Explicitly increments the reference count.
 *
 * This function is const, even though it changes the object, because
 * generally fiddling with an object's reference count isn't considered part
 * of fiddling with the object.  An object might be const in other ways, but
 * we still need to accurately count the number of references to it.
 */
INLINE void NodeCachedReferenceCount::
node_ref() const {
#ifdef _DEBUG
  nassertv(test_ref_count_integrity());
#endif

  ref();
  AtomicAdjust::inc(((NodeCachedReferenceCount *)this)->_node_ref_count);
}

/**
 * Explicitly decrements the node reference count and the normal reference
 * count simultaneously.
 *
 * The return value is true if the new reference count is nonzero, false if it
 * is zero.
 */
INLINE bool NodeCachedReferenceCount::
node_unref() const {
  node_unref_only();
  return unref();
}

/**
 * Does some easy checks to make sure that the reference count isn't
 * completely bogus.
 */
INLINE bool NodeCachedReferenceCount::
test_ref_count_integrity() const {
#ifndef NDEBUG
  return do_test_ref_count_integrity();
#else
  return true;
#endif
}

/**
 * Returns the union of the values defined in the Referenced enum that
 * represents the various things that appear to be holding a pointer to this
 * object.
 *
 * If R_node is included, at least one node is holding a pointer; if R_cache
 * is included, at least one cache element is.
 */
INLINE int NodeCachedReferenceCount::
get_referenced_bits() const {
  int result = 0;
  if (get_node_ref_count() != 0) {
    result |= R_node;
  }
  if (get_cache_ref_count() != 0) {
    result |= R_cache;
  }

  return result;
}

/**
 * Decrements the node reference count without affecting the normal reference
 * count.  Intended to be called by derived classes only, presumably to
 * reimplement node_unref().
 */
INLINE void NodeCachedReferenceCount::
node_unref_only() const {
#ifdef _DEBUG
  nassertv(test_ref_count_integrity());
#endif

  // If this assertion fails, you tried to unref an object with a zero
  // reference count.  Are you using ref() and unref() directly?  Are you sure
  // you can't use PointerTo's?
  nassertv(_node_ref_count > 0);

  AtomicAdjust::dec(((NodeCachedReferenceCount *)this)->_node_ref_count);
}
