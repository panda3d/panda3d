/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file referenceCount.I
 * @author drose
 * @date 1998-10-23
 */

template<class Base>
TypeHandle RefCountProxy<Base>::_type_handle;

template<class Base>
TypeHandle RefCountObj<Base>::_type_handle;

/**
 * The ReferenceCount constructor is protected because you almost never want
 * to create just a ReferenceCount object by itself, and it's probably a
 * mistake if you try.
 *
 * ReferenceCount doesn't store any useful information in its own right; its
 * only purpose is to add reference-counting to some other class via
 * inheritance.
 */
INLINE ReferenceCount::
ReferenceCount() {
  _weak_list = nullptr;
  _ref_count = 0;
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this);
#endif
}

/**
 * The copies of reference-counted objects do not themselves inherit the
 * reference count!
 *
 * This copy constructor is protected because you almost never want to create
 * just a ReferenceCount object by itself, and it's probably a mistake if you
 * try.
 */
INLINE ReferenceCount::
ReferenceCount(const ReferenceCount &) {
  _weak_list = nullptr;
  _ref_count = 0;
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this);
#endif
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
INLINE void ReferenceCount::
operator = (const ReferenceCount &) {
  // If this assertion fails, our own pointer was recently deleted.  Possibly
  // you used a real pointer instead of a PointerTo at some point, and the
  // object was deleted when the PointerTo went out of scope.  Maybe you tried
  // to create an automatic (local variable) instance of a class that derives
  // from ReferenceCount.  Or maybe your headers are out of sync, and you need
  // to make clean in direct or some higher tree.
  nassertv(_ref_count != deleted_ref_count);
}

/**
 *
 */
ReferenceCount::
~ReferenceCount() {
  TAU_PROFILE("ReferenceCount::~ReferenceCount()", " ", TAU_USER);
  // If this assertion fails, we're trying to delete an object that was just
  // deleted.  Possibly you used a real pointer instead of a PointerTo at some
  // point, and the object was deleted when the PointerTo went out of scope.
  // Maybe you tried to create an automatic (local variable) instance of a
  // class that derives from ReferenceCount.  Or maybe your headers are out of
  // sync, and you need to make clean in direct or some higher tree.
  nassertv(_ref_count != deleted_ref_count);

  // If this assertion fails, we're trying to delete a static object that
  // still has an outstanding reference count.  You should make sure that all
  // references to your static objects are gone by the time the object itself
  // destructs.
  nassertv(_ref_count <= local_ref_count);

  // If this assertion fails, the reference counts are all screwed up
  // altogether.  Maybe some errant code stomped all over memory somewhere.
  nassertv(_ref_count >= 0);

  // If this assertion fails, someone tried to delete this object while its
  // reference count was still positive.  Maybe you tried to point a PointerTo
  // at a static object (a local variable, instead of one allocated via new)?
  // The test below against 0x7f is supposed to check for that, but it's a
  // pretty hokey test.

  // Another possibility is you inadvertently omitted a copy constructor for a
  // ReferenceCount object, and then bitwise copied a dynamically allocated
  // value--reference count and all--onto a locally allocated one.
  nassertv(_ref_count == 0 || _ref_count == local_ref_count);

  // Tell our weak reference holders that we're going away now.
  if (_weak_list != nullptr) {
    ((WeakReferenceList *)_weak_list)->mark_deleted();
    _weak_list = nullptr;
  }

#ifndef NDEBUG
  // Ok, all clear to delete.  Now set the reference count to
  // deleted_ref_count, so we'll have a better chance of noticing if we happen
  // to have a stray pointer to it still out there.
  _ref_count = deleted_ref_count;
#endif

#ifdef DO_MEMORY_USAGE
  MemoryUsage::remove_pointer(this);
#endif
}

/**
 * Returns the current reference count.
 */
INLINE int ReferenceCount::
get_ref_count() const {
#ifdef _DEBUG
  test_ref_count_integrity();
#endif
  return (int)AtomicAdjust::get(_ref_count);
}

/**
 * Explicitly increments the reference count.  User code should avoid using
 * ref() and unref() directly, which can result in missed reference counts.
 * Instead, let a PointerTo object manage the reference counting
 * automatically.
 *
 * This function is const, even though it changes the object, because
 * generally fiddling with an object's reference count isn't considered part
 * of fiddling with the object.  An object might be const in other ways, but
 * we still need to accurately count the number of references to it.
 */
INLINE void ReferenceCount::
ref() const {
  TAU_PROFILE("void ReferenceCount::ref()", " ", TAU_USER);
#ifdef _DEBUG
  nassertv(test_ref_count_integrity());
#endif

  AtomicAdjust::inc(_ref_count);
}

/**
 * Explicitly decrements the reference count.  Note that the object will not
 * be implicitly deleted by unref() simply because the reference count drops
 * to zero.  (Having a member function delete itself is problematic.) However,
 * see the helper function unref_delete().
 *
 * User code should avoid using ref() and unref() directly, which can result
 * in missed reference counts.  Instead, let a PointerTo object manage the
 * reference counting automatically.
 *
 * This function is const, even though it changes the object, because
 * generally fiddling with an object's reference count isn't considered part
 * of fiddling with the object.  An object might be const in other ways, but
 * we still need to accurately count the number of references to it.
 *
 * The return value is true if the new reference count is nonzero, false if it
 * is zero.
 */
INLINE bool ReferenceCount::
unref() const {
  TAU_PROFILE("void ReferenceCount::unref()", " ", TAU_USER);
#ifdef _DEBUG
  nassertr(test_ref_count_integrity(), 0);

  // If this assertion fails, you tried to unref an object with a zero
  // reference count.  Are you using ref() and unref() directly?  Are you sure
  // you can't use PointerTo's?
  nassertr(_ref_count > 0, 0);
#endif
  return AtomicAdjust::dec(_ref_count);
}

/**
 * Does some easy checks to make sure that the reference count isn't
 * completely bogus.  Returns true if ok, false otherwise.
 */
INLINE bool ReferenceCount::
test_ref_count_integrity() const {
#ifndef NDEBUG
  return do_test_ref_count_integrity();
#else
  return true;
#endif
}

/**
 * Does some easy checks to make sure that the reference count isn't zero, or
 * completely bogus.  Returns true if ok, false otherwise.
 */
INLINE bool ReferenceCount::
test_ref_count_nonzero() const {
#ifndef NDEBUG
  return do_test_ref_count_nonzero();
#else
  return true;
#endif
}

/**
 * This function should be called, once, immediately after creating a new
 * instance of some ReferenceCount-derived object on the stack.
 *
 * This allows the object to be passed to functions that will increment and
 * decrement the object's reference count temporarily, and it will prevent the
 * object from being deleted (inappropriately), when the reference count
 * returns to zero.  It actually achieves this by setting a large positive
 * value in the reference count field.
 */
INLINE void ReferenceCount::
local_object() {
  // If this assertion fails, you didn't call this immediately after creating
  // a local object.
  nassertv(_ref_count == 0);

  _ref_count = local_ref_count;
}

/**
 * Returns true if this particular ReferenceCount object has a
 * WeakReferenceList created, false otherwise.  In general, this will be true
 * if there was ever a WeakPointerTo created for this object (even if there is
 * not any for it now).
 */
INLINE bool ReferenceCount::
has_weak_list() const {
  return _weak_list != nullptr;
}

/**
 * Returns the WeakReferenceList associated with this ReferenceCount object.
 * If there has never been a WeakReferenceList associated with this object,
 * creates one now.
 *
 * The returned object will be deleted automatically when all weak and strong
 * references to the object have gone.
 */
INLINE WeakReferenceList *ReferenceCount::
get_weak_list() const {
  if (AtomicAdjust::get_ptr(_weak_list) == nullptr) {
    ((ReferenceCount *)this)->create_weak_list();
  }
  return (WeakReferenceList *)AtomicAdjust::get_ptr(_weak_list);
}

/**
 * Adds the indicated PointerToVoid as a weak reference to this object.
 * Returns an object that will persist as long as any reference (strong or
 * weak) exists, for calling unref() or checking whether the object still
 * exists.
 */
INLINE WeakReferenceList *ReferenceCount::
weak_ref() {
  TAU_PROFILE("void ReferenceCount::weak_ref()", " ", TAU_USER);
#ifdef _DEBUG
  nassertr(test_ref_count_integrity(), nullptr);
#else
  nassertr(_ref_count != deleted_ref_count, nullptr);
#endif
  WeakReferenceList *weak_ref = get_weak_list();
  weak_ref->ref();
  return weak_ref;
}

/**
 * Removes the indicated PointerToVoid as a weak reference to this object.  It
 * must have previously been added via a call to weak_ref().
 */
INLINE void ReferenceCount::
weak_unref() {
  TAU_PROFILE("void ReferenceCount::weak_unref()", " ", TAU_USER);
#ifdef _DEBUG
  nassertv(test_ref_count_integrity());
#endif
  WeakReferenceList *weak_list = (WeakReferenceList *)_weak_list;
  nassertv(weak_list != nullptr);
  bool nonzero = weak_list->unref();
  nassertv(nonzero);
}

/**
 * Atomically increases the reference count of this object if it is not zero.
 * Do not use this.  This exists only to implement a special case for weak
 * pointers.
 * @return true if the reference count was incremented, false if it was zero.
 */
INLINE bool ReferenceCount::
ref_if_nonzero() const {
#ifdef _DEBUG
  test_ref_count_integrity();
#endif
  AtomicAdjust::Integer ref_count;
  do {
    ref_count = AtomicAdjust::get(_ref_count);
    if (ref_count <= 0) {
      return false;
    }
  } while (ref_count != AtomicAdjust::compare_and_exchange(_ref_count, ref_count, ref_count + 1));
  return true;
}

/**
 * This global helper function will unref the given ReferenceCount object, and
 * if the reference count reaches zero, automatically delete it.  It can't be
 * a member function because it's usually a bad idea to delete an object from
 * within its own member function.  It's a template function so the destructor
 * doesn't have to be virtual.
 */
template<class RefCountType>
INLINE void
unref_delete(RefCountType *ptr) {
  TAU_PROFILE("void unref_delete(RefCountType *)", " ", TAU_USER);
  // Although it may be tempting to try to upcast ptr to a ReferenceCount
  // object (particularly to get around inheritance issues), resist that
  // temptation, since some classes (in particular, TransformState and
  // RenderState) rely on a non-virtual overloading of the unref() method.

  if (!ptr->unref()) {
    // If the reference count has gone to zero, delete the object.
    delete ptr;
  }
}

/**
 *
 */
template<class Base>
INLINE RefCountProxy<Base>::
RefCountProxy() {
}

/**
 *
 */
template<class Base>
INLINE RefCountProxy<Base>::
RefCountProxy(const Base &copy) : _base(copy) {
}

/**
 *
 */
template<class Base>
INLINE RefCountProxy<Base>::
operator Base &() {
  return _base;
}

/**
 *
 */
template<class Base>
INLINE RefCountProxy<Base>::
operator const Base &() const {
  return _base;
}

/**
 *
 */
template<class Base>
void RefCountProxy<Base>::
init_type() {
  do_init_type(Base);
  register_type(_type_handle,
                "RefCountProxy<" + get_type_handle(Base).get_name() + ">",
                get_type_handle(Base));
}


/**
 *
 */
template<class Base>
INLINE RefCountObj<Base>::
RefCountObj() {
}

/**
 *
 */
template<class Base>
INLINE RefCountObj<Base>::
RefCountObj(const Base &copy) : Base(copy) {
}


/**
 *
 */
template<class Base>
void RefCountObj<Base>::
init_type() {
  std::string base_name = get_type_name(Base);
  TypeHandle base_type = register_dynamic_type(base_name);

  ReferenceCount::init_type();
  _type_handle =
    register_dynamic_type("RefCountObj<" + base_name + ">",
                          base_type, ReferenceCount::get_class_type());
}
