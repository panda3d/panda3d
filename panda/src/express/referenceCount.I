// Filename: referenceCount.I
// Created by:  drose (23Oct98)
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
  MemoryUsage::record_pointer(this);
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
  MemoryUsage::record_pointer(this);
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
//               try.
////////////////////////////////////////////////////////////////////
INLINE void ReferenceCount::
operator = (const ReferenceCount &) {
  nassertv(this != NULL);
  
  // If this assertion fails, our own pointer was recently deleted.
  // Yikes!
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
  prepare_delete();
  MemoryUsage::remove_pointer(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::prepare_delete
//       Access: Public
//  Description: This function is called by the ReferenceCount
//               destructor to check on all of the numbers and make
//               sure it's all right to delete the object.  Normally
//               it will never be called explicitly elsewhere, except
//               maybe in unref_delete() to simulate deleting the node
//               without actually deleting it, when we want to test
//               ref count integrities.
////////////////////////////////////////////////////////////////////
INLINE void ReferenceCount::
prepare_delete() {
  nassertv(this != NULL);
  
  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Probably you've accidentally made a bitwise
  // copy of a PointerTo, by forgetting to write a copy constructor
  // for a class that contains PointerTo's.
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
  
  // Ok, all clear to delete.  Now set the reference count to -100,
  // so we'll have a better chance of noticing if we happen to have
  // a stray pointer to it still out there.
  _ref_count = -100;
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::unref_consider_delete
//       Access: Public
//  Description: This function is called by the global unref_delete()
//               function.  This function unrefs the pointer and
//               returns true if it should be deleted (i.e. the count
//               has gone to zero), or false otherwise.  It doesn't
//               delete itself because (a) a method cannot safely
//               delete this, and (b) we don't have a virtual
//               destructor anyway.  The decision of whether to delete
//               is left up to unref_delete().
////////////////////////////////////////////////////////////////////
INLINE bool ReferenceCount::
unref_consider_delete() {
  unref();
  return (get_ref_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::get_ref_count
//       Access: Public
//  Description: Returns the current reference count.
////////////////////////////////////////////////////////////////////
INLINE int ReferenceCount::
get_ref_count() const {
  test_ref_count_integrity();
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
////////////////////////////////////////////////////////////////////
INLINE void ReferenceCount::
ref() const {
  nassertv(this != NULL);

  // If this assertion fails, we're trying to ref a pointer that was
  // just deleted.  Probably you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Either that, or you forgot to
  // define a copy constructor for a class that contains
  // PointerTo's.
  nassertv(_ref_count != -100);
  
  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_ref_count >= 0);

#if 0
  // This is an SGI-specific hack.  Maybe this isn't a valid test at
  // all.  This is supposed to fail if the 'this' pointer is on the
  // stack, instead of on the heap--I assume if the first two hex
  // digits of the pointer of 0x7f it's on the stack, possibly a bad
  // assumption.  In any case, if you ref-count a pointer on the
  // stack, you just goofed.  Don't assign a PointerTo to refer to
  // any local variables!
  nassertv(((long)this & 0xff000000) != 0x7f000000);
#endif
  
  ((ReferenceCount *)this)->_ref_count++;
  
  // If this assertion fails, we have just wrapped!  Either there
  // are actually 2^32 pointers to this thing somewhere, or
  // something is really screwy.
  nassertv(_ref_count > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::unref
//       Access: Public
//  Description: Explicitly decrements the reference count.  Note that
//               the object will not be implicitly deleted by unref()
//               simply because the reference count drops to zero.
//               However, see the helper function unref_delete().
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
////////////////////////////////////////////////////////////////////
INLINE void ReferenceCount::
unref() const {
  nassertv(this != NULL);

  // If this assertion fails, we're trying to unref a pointer that
  // was just deleted.  Probably you used a real pointer instead of
  // a PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Either that, or you forgot to
  // define a copy constructor for a class that contains
  // PointerTo's.
  nassertv(_ref_count != -100);
  
  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_ref_count >= 0);
  
  // If this assertion fails, you tried to unref an object with a
  // zero reference count.  Are you using ref() and unref()
  // directly?  Are you sure you can't use PointerTo's?
  nassertv(_ref_count > 0);
  ((ReferenceCount *)this)->_ref_count--;
}


////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::test_ref_count_integrity
//       Access: Public
//  Description: Does some easy checks to make sure that the reference
//               count isn't completely bogus.
////////////////////////////////////////////////////////////////////
INLINE void ReferenceCount::
test_ref_count_integrity() const {
  nassertv(this != NULL);

  // If this assertion fails, we're trying to access a pointer that
  // was just deleted.  Probably you used a real pointer instead of
  // a PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Either that, or you forgot to
  // define a copy constructor for a class that contains
  // PointerTo's.
  nassertv(_ref_count != -100);
  
  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertv(_ref_count >= 0);
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
  if (((ReferenceCount *)ptr)->unref_consider_delete()) {
#ifndef NDEBUG
    if (get_leak_memory()) {
      // In leak-memory mode, we don't actually delete the pointer,
      // although we do call the destructor explicitly.  This has
      // exactly the same effect as deleting it, without actually
      // freeing up the memory it uses.
      ptr->~RefCountType();
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
#ifdef RTTI
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
