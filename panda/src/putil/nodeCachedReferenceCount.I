// Filename: nodeCachedReferenceCount.I
// Created by:  drose (07May05)
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
//     Function: NodeCachedReferenceCount::Constructor
//       Access: Protected
//  Description: The ReferenceCount constructor is protected because
//               you almost never want to create just a ReferenceCount
//               object by itself, and it's probably a mistake if you
//               try.
//
//               ReferenceCount doesn't store any useful information
//               in its own right; its only purpose is to add
//               reference-counting to some other class via
//               inheritance.
////////////////////////////////////////////////////////////////////
INLINE NodeCachedReferenceCount::
NodeCachedReferenceCount() {
  _node_ref_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::Copy Constructor
//       Access: Protected
//  Description: The copies of reference-counted objects do not
//               themselves inherit the reference count!
//
//               This copy constructor is protected because you almost
//               never want to create just a ReferenceCount object by
//               itself, and it's probably a mistake if you try.
////////////////////////////////////////////////////////////////////
INLINE NodeCachedReferenceCount::
NodeCachedReferenceCount(const NodeCachedReferenceCount &copy) : CachedTypedWritableReferenceCount(copy) {
  _node_ref_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::Copy Assignment Operator
//       Access: Protected
//  Description: The copies of reference-counted objects do not
//               themselves inherit the reference count!
//
//               This copy assignment operator is protected because
//               you almost never want to copy just a ReferenceCount
//               object by itself, and it's probably a mistake if you
//               try.  Instead, this should only be called from a
//               derived class that implements this operator and then
//               calls up the inheritance chain.
////////////////////////////////////////////////////////////////////
INLINE void NodeCachedReferenceCount::
operator = (const NodeCachedReferenceCount &copy) {
  nassertv(this != NULL);

  // If this assertion fails, our own pointer was recently deleted.
  // Possibly you used a real pointer instead of a PointerTo at some
  // point, and the object was deleted when the PointerTo went out of
  // scope.  Maybe you tried to create an automatic (local variable)
  // instance of a class that derives from ReferenceCount.  Or maybe
  // your headers are out of sync, and you need to make clean in
  // direct or some higher tree.
  nassertv(_node_ref_count != -100);

  CachedTypedWritableReferenceCount::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::Destructor
//       Access: Protected
//  Description: The ReferenceCount destructor is protected to
//               discourage users from accidentally trying to delete a
//               ReferenceCount pointer directly.  This is almost
//               always a bad idea, since the destructor is not
//               virtual, and you've almost certainly got some pointer
//               to something that inherits from ReferenceCount, not
//               just a plain old ReferenceCount object.
////////////////////////////////////////////////////////////////////
INLINE NodeCachedReferenceCount::
~NodeCachedReferenceCount() {
  nassertv(this != NULL);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertv(_node_ref_count != -100);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_node_ref_count >= 0);

  // If this assertion fails, someone tried to delete this object
  // while its reference count was still positive.  Maybe you tried
  // to point a PointerTo at a static object (a local variable,
  // instead of one allocated via new)?  The test below against 0x7f
  // is supposed to check for that, but it's a pretty hokey test.

  // Another possibility is you inadvertently omitted a copy
  // constructor for a ReferenceCount object, and then bitwise
  // copied a dynamically allocated value--reference count and
  // all--onto a locally allocated one.
  nassertv(_node_ref_count == 0);

#ifndef NDEBUG
  // Ok, all clear to delete.  Now set the reference count to -100,
  // so we'll have a better chance of noticing if we happen to have
  // a stray pointer to it still out there.
  _node_ref_count = -100;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::get_node_ref_count
//       Access: Published
//  Description: Returns the current reference count.
////////////////////////////////////////////////////////////////////
INLINE int NodeCachedReferenceCount::
get_node_ref_count() const {
#ifndef NDEBUG
  test_ref_count_integrity();
#endif
  return _node_ref_count;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::node_ref
//       Access: Published
//  Description: Explicitly increments the reference count.
//
//               This function is const, even though it changes the
//               object, because generally fiddling with an object's
//               reference count isn't considered part of fiddling
//               with the object.  An object might be const in other
//               ways, but we still need to accurately count the
//               number of references to it.
//
//               The return value is the new reference count.
////////////////////////////////////////////////////////////////////
INLINE int NodeCachedReferenceCount::
node_ref() const {
  nassertr(this != NULL, 0);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertr(_node_ref_count != -100, 0);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertr(_node_ref_count >= 0, 0);

  ref();
  return AtomicAdjust::inc(((NodeCachedReferenceCount *)this)->_node_ref_count);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::node_unref
//       Access: Published
//  Description: Explicitly decrements the reference count.  Note that
//               the object will not be implicitly deleted by unref()
//               simply because the reference count drops to zero.
//               (Having a member function delete itself is
//               problematic; plus, we don't have a virtual destructor
//               anyway.) However, see the helper function
//               unref_delete().
//
//               User code should avoid using ref() and unref()
//               directly, which can result in missed reference
//               counts.  Instead, let a PointerTo object manage the
//               reference counting automatically.
//
//               This function is const, even though it changes the
//               object, because generally fiddling with an object's
//               reference count isn't considered part of fiddling
//               with the object.  An object might be const in other
//               ways, but we still need to accurately count the
//               number of references to it.
//
//               The return value is the new reference count.
////////////////////////////////////////////////////////////////////
INLINE int NodeCachedReferenceCount::
node_unref() const {
  nassertr(this != NULL, false);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertr(_node_ref_count != -100, false);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertr(_node_ref_count >= 0, false);

  // If this assertion fails, you tried to unref an object with a
  // zero reference count.  Are you using ref() and unref()
  // directly?  Are you sure you can't use PointerTo's?
  nassertr(_node_ref_count > 0, false);

  unref();
  return AtomicAdjust::dec(((NodeCachedReferenceCount *)this)->_node_ref_count);
}


////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::test_ref_count_integrity
//       Access: Published
//  Description: Does some easy checks to make sure that the reference
//               count isn't completely bogus.
////////////////////////////////////////////////////////////////////
INLINE void NodeCachedReferenceCount::
test_ref_count_integrity() const {
#ifndef NDEBUG
  nassertv(this != NULL);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertv(_node_ref_count != -100);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_node_ref_count >= 0);

  CachedTypedWritableReferenceCount::test_ref_count_integrity();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: NodeCachedReferenceCount::get_referenced_bits
//       Access: Published
//  Description: Returns the union of the values defined in the
//               Referenced enum that represents the various things
//               that appear to be holding a pointer to this object.
//
//               If R_node is included, at least one node is holding a
//               pointer; if R_cache is included, at least one cache
//               element is.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: node_unref_delete
//  Description: This global helper function will unref the given
//               ReferenceCount object, and if the reference count
//               reaches zero, automatically delete it.  It can't be a
//               member function because it's usually a bad idea to
//               delete an object from within its own member function.
//               It's a template function so the destructor doesn't
//               have to be virtual.
////////////////////////////////////////////////////////////////////
template<class RefCountType>
INLINE void
node_unref_delete(RefCountType *ptr) {
  ptr->node_unref();
  if (ptr->get_ref_count() == 0) {
#ifndef NDEBUG
    if (get_leak_memory()) {
      // In leak-memory mode, we don't actually delete the pointer,
      // although we do call the destructor explicitly.  This has
      // exactly the same effect as deleting it, without actually
      // freeing up the memory it uses.

      // Furthermore, if we have never-destruct set, we don't even
      // call the destructor.
      if (!get_never_destruct()) {
        ptr->~RefCountType();
      }
      return;
    }
#endif
    delete ptr;
  }
}

