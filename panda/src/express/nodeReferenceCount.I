// Filename: nodeReferenceCount.I
// Created by:  drose (01May06)
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


template<class Base>
TypeHandle NodeRefCountObj<Base>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::Constructor
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
INLINE NodeReferenceCount::
NodeReferenceCount() {
  _node_ref_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::Copy Constructor
//       Access: Protected
//  Description: The copies of reference-counted objects do not
//               themselves inherit the reference count!
//
//               This copy constructor is protected because you almost
//               never want to create just a ReferenceCount object by
//               itself, and it's probably a mistake if you try.
////////////////////////////////////////////////////////////////////
INLINE NodeReferenceCount::
NodeReferenceCount(const NodeReferenceCount &copy) : ReferenceCount(copy) {
  _node_ref_count = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::Copy Assignment Operator
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
INLINE void NodeReferenceCount::
operator = (const NodeReferenceCount &copy) {
  nassertv(this != NULL);

  // If this assertion fails, our own pointer was recently deleted.
  // Possibly you used a real pointer instead of a PointerTo at some
  // point, and the object was deleted when the PointerTo went out of
  // scope.  Maybe you tried to create an automatic (local variable)
  // instance of a class that derives from ReferenceCount.  Or maybe
  // your headers are out of sync, and you need to make clean in
  // direct or some higher tree.
  nassertv(_node_ref_count != -100);

  ReferenceCount::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::Destructor
//       Access: Protected
//  Description: The ReferenceCount destructor is protected to
//               discourage users from accidentally trying to delete a
//               ReferenceCount pointer directly.  This is almost
//               always a bad idea, since the destructor is not
//               virtual, and you've almost certainly got some pointer
//               to something that inherits from ReferenceCount, not
//               just a plain old ReferenceCount object.
////////////////////////////////////////////////////////////////////
INLINE NodeReferenceCount::
~NodeReferenceCount() {
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
//     Function: NodeReferenceCount::get_node_ref_count
//       Access: Published
//  Description: Returns the current reference count.
////////////////////////////////////////////////////////////////////
INLINE int NodeReferenceCount::
get_node_ref_count() const {
#ifdef _DEBUG
  test_ref_count_integrity();
#endif
  return AtomicAdjust::get(_node_ref_count);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::node_ref
//       Access: Published
//  Description: Explicitly increments the reference count.
//
//               This function is const, even though it changes the
//               object, because generally fiddling with an object's
//               reference count isn't considered part of fiddling
//               with the object.  An object might be const in other
//               ways, but we still need to accurately count the
//               number of references to it.
////////////////////////////////////////////////////////////////////
INLINE void NodeReferenceCount::
node_ref() const {
#ifdef _DEBUG
  nassertv(test_ref_count_integrity());
#endif

  ref();
  AtomicAdjust::inc(((NodeReferenceCount *)this)->_node_ref_count);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::node_unref
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
//               The return value is true if the new reference count
//               is nonzero, false if it is zero.
////////////////////////////////////////////////////////////////////
INLINE bool NodeReferenceCount::
node_unref() const {
#ifdef _DEBUG
  nassertr(test_ref_count_integrity(), 0);
#endif

  // If this assertion fails, you tried to unref an object with a
  // zero reference count.  Are you using ref() and unref()
  // directly?  Are you sure you can't use PointerTo's?
  nassertr(_node_ref_count > 0, 0);

  unref();
  return AtomicAdjust::dec(((NodeReferenceCount *)this)->_node_ref_count);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeReferenceCount::test_ref_count_integrity
//       Access: Published
//  Description: Does some easy checks to make sure that the reference
//               count isn't completely bogus.
////////////////////////////////////////////////////////////////////
INLINE bool NodeReferenceCount::
test_ref_count_integrity() const {
#ifndef NDEBUG
  return do_test_ref_count_integrity();
#else
  return true;
#endif
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
    ObjectDeletor *deletor = ObjectDeletor::get_global_ptr();
    if (deletor != (ObjectDeletor *)NULL) {
      ptr->ref();
      deletor->delete_object(RefCountDeleteWrapper<RefCountType>::do_delete, (void *)ptr);
      return;
    }

    delete ptr;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: NodeRefCountObj::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE NodeRefCountObj<Base>::
NodeRefCountObj() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodeRefCountObj::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE NodeRefCountObj<Base>::
NodeRefCountObj(const Base &copy) : Base(copy) {
}


////////////////////////////////////////////////////////////////////
//     Function: NodeRefCountObj::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
void NodeRefCountObj<Base>::
init_type() {
#if defined(HAVE_RTTI) && !defined(__EDG__)
  // If we have RTTI, we can determine the name of the base type.
  string base_name = typeid(Base).name();
#else
  string base_name = "unknown";
#endif

  TypeHandle base_type = register_dynamic_type(base_name);

  ReferenceCount::init_type();
  _type_handle =
    register_dynamic_type("NodeRefCountObj<" + base_name + ">",
                          base_type, ReferenceCount::get_class_type());
}
