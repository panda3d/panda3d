// Filename: referenceCount.I
// Created by:  drose (23Oct98)
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
TypeHandle RefCountProxy<Base>::_type_handle;

template<class Base>
TypeHandle RefCountObj<Base>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::Constructor
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
INLINE ReferenceCount::
ReferenceCount() {
  _ref_count = 0;
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::Copy Constructor
//       Access: Protected
//  Description: The copies of reference-counted objects do not
//               themselves inherit the reference count!
//
//               This copy constructor is protected because you almost
//               never want to create just a ReferenceCount object by
//               itself, and it's probably a mistake if you try.
////////////////////////////////////////////////////////////////////
INLINE ReferenceCount::
ReferenceCount(const ReferenceCount &) {
  _ref_count = 0;
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::Copy Assignment Operator
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
INLINE void ReferenceCount::
operator = (const ReferenceCount &) {
  nassertv(this != NULL);

  // If this assertion fails, our own pointer was recently deleted.
  // Possibly you used a real pointer instead of a PointerTo at some
  // point, and the object was deleted when the PointerTo went out of
  // scope.  Maybe you tried to create an automatic (local variable)
  // instance of a class that derives from ReferenceCount.  Or maybe
  // your headers are out of sync, and you need to make clean in
  // direct or some higher tree.
  nassertv(_ref_count != -100);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::Destructor
//       Access: Protected
//  Description: The ReferenceCount destructor is protected to
//               discourage users from accidentally trying to delete a
//               ReferenceCount pointer directly.  This is almost
//               always a bad idea, since the destructor is not
//               virtual, and you've almost certainly got some pointer
//               to something that inherits from ReferenceCount, not
//               just a plain old ReferenceCount object.
////////////////////////////////////////////////////////////////////
INLINE ReferenceCount::
~ReferenceCount() {
  nassertv(this != NULL);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertv(_ref_count != -100);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_ref_count >= 0);

  // If this assertion fails, someone tried to delete this object
  // while its reference count was still positive.  Maybe you tried
  // to point a PointerTo at a static object (a local variable,
  // instead of one allocated via new)?  The test below against 0x7f
  // is supposed to check for that, but it's a pretty hokey test.

  // Another possibility is you inadvertently omitted a copy
  // constructor for a ReferenceCount object, and then bitwise
  // copied a dynamically allocated value--reference count and
  // all--onto a locally allocated one.
  nassertv(_ref_count == 0);

#ifndef NDEBUG
  // Ok, all clear to delete.  Now set the reference count to -100,
  // so we'll have a better chance of noticing if we happen to have
  // a stray pointer to it still out there.
  _ref_count = -100;
#endif

#ifdef DO_MEMORY_USAGE
  MemoryUsage::remove_pointer(this);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::get_ref_count
//       Access: Public
//  Description: Returns the current reference count.
////////////////////////////////////////////////////////////////////
INLINE int ReferenceCount::
get_ref_count() const {
#ifndef NDEBUG
  test_ref_count_integrity();
#endif
  return _ref_count;
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::ref
//       Access: Public
//  Description: Explicitly increments the reference count.  User code
//               should avoid using ref() and unref() directly, which
//               can result in missed reference counts.  Instead, let
//               a PointerTo object manage the reference counting
//               automatically.
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
INLINE int ReferenceCount::
ref() const {
  nassertr(this != NULL, 0);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertr(_ref_count != -100, 0);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertr(_ref_count >= 0, 0);

  return AtomicAdjust::inc(((ReferenceCount *)this)->_ref_count);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::unref
//       Access: Public
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
INLINE int ReferenceCount::
unref() const {
  nassertr(this != NULL, false);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertr(_ref_count != -100, false);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertr(_ref_count >= 0, false);

  // If this assertion fails, you tried to unref an object with a
  // zero reference count.  Are you using ref() and unref()
  // directly?  Are you sure you can't use PointerTo's?
  nassertr(_ref_count > 0, false);
  return AtomicAdjust::dec(((ReferenceCount *)this)->_ref_count);
}


////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::test_ref_count_integrity
//       Access: Public
//  Description: Does some easy checks to make sure that the reference
//               count isn't completely bogus.
////////////////////////////////////////////////////////////////////
INLINE void ReferenceCount::
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
  nassertv(_ref_count != -100);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_ref_count >= 0);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: unref_delete
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
unref_delete(RefCountType *ptr) {
  if (((ReferenceCount *)ptr)->unref() == 0) {
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


////////////////////////////////////////////////////////////////////
//     Function: RefCountProxy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE RefCountProxy<Base>::
RefCountProxy() {
}

////////////////////////////////////////////////////////////////////
//     Function: RefCountProxy::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE RefCountProxy<Base>::
RefCountProxy(const Base &copy) : _base(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: RefCountProxy::Base Typecast Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE RefCountProxy<Base>::
operator Base &() {
  return _base;
}

////////////////////////////////////////////////////////////////////
//     Function: RefCountProxy::Base Typecast Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE RefCountProxy<Base>::
operator const Base &() const {
  return _base;
}

////////////////////////////////////////////////////////////////////
//     Function: RefCountProxy::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
void RefCountProxy<Base>::
init_type() {
  do_init_type(Base);
  register_type(_type_handle,
                "RefCountProxy<" + get_type_handle(Base).get_name() + ">",
                get_type_handle(Base));
}


////////////////////////////////////////////////////////////////////
//     Function: RefCountObj::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE RefCountObj<Base>::
RefCountObj() {
}

////////////////////////////////////////////////////////////////////
//     Function: RefCountObj::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE RefCountObj<Base>::
RefCountObj(const Base &copy) : Base(copy) {
}


////////////////////////////////////////////////////////////////////
//     Function: RefCountObj::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
void RefCountObj<Base>::
init_type() {
#ifdef HAVE_RTTI
  // If we have RTTI, we can determine the name of the base type.
  string base_name = typeid(Base).name();
#else
  string base_name = "unknown";
#endif

  TypeHandle base_type = register_dynamic_type(base_name);

  ReferenceCount::init_type();
  _type_handle =
    register_dynamic_type("RefCountObj<" + base_name + ">",
                          base_type, ReferenceCount::get_class_type());
}
