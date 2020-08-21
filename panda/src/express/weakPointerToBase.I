/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file weakPointerToBase.I
 * @author drose
 * @date 2004-09-27
 */

/**
 * Constructs a weak pointer from a plain pointer (or nullptr).  It is the
 * caller's responsibility to ensure that it points to a valid object.
 */
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(To *ptr) {
  _void_ptr = (To *)ptr;
  if (ptr != nullptr) {
    _weak_ref = ptr->weak_ref();
#ifdef DO_MEMORY_USAGE
    update_type(ptr);
#endif
  }
}

/**
 * Constructs a weak pointer from a reference-counting pointer.
 */
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(const PointerToBase<T> &copy) {
  // This double-casting is a bit of a cheat to get around the inheritance
  // issue--it's difficult to declare a template class to be a friend.
  To *ptr = (To *)((const WeakPointerToBase<To> *)&copy)->_void_ptr;
  _void_ptr = ptr;
  if (ptr != nullptr) {
    _weak_ref = ptr->weak_ref();
  }
}

/**
 * Copies a weak pointer.  This is always safe, even for expired pointers.
 */
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(const WeakPointerToBase<T> &copy) {
  _void_ptr = copy._void_ptr;

  // While it is tempting to stop maintaining the control block pointer after
  // the object has been deleted, we still need it in order to define a
  // consistent ordering in owner_before.
  WeakReferenceList *weak_ref = copy._weak_ref;
  if (weak_ref != nullptr/* && !weak_ref->was_deleted()*/) {
    _weak_ref = copy._weak_ref;
    _weak_ref->ref();
  }
}

/**
 * Moves a weak pointer.  This is always safe, even for expired pointers.
 */
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(WeakPointerToBase<T> &&from) noexcept {
  this->_void_ptr = from._void_ptr;
  this->_weak_ref = from._weak_ref;
  from._void_ptr = nullptr;
  from._weak_ref = nullptr;
}

/**
 * Copies a weak pointer from a cast-convertible weak pointer type.
 */
template<class T>
template<class Y>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(const WeakPointerToBase<Y> &r) {
  // If this next line gives an error, you are trying to convert a WeakPointerTo
  // from an incompatible type of another WeakPointerTo.
  To *ptr = (Y *)r._void_ptr;

  this->_void_ptr = ptr;

  WeakReferenceList *weak_ref = r._weak_ref;
  if (weak_ref != nullptr) {
    _weak_ref = weak_ref;
    weak_ref->ref();
  }
}

/**
 * Moves a weak pointer from a cast-convertible weak pointer type.
 */
template<class T>
template<class Y>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(WeakPointerToBase<Y> &&r) noexcept {
  // If this next line gives an error, you are trying to convert a WeakPointerTo
  // from an incompatible type of another WeakPointerTo.
  To *ptr = (Y *)r._void_ptr;

  this->_void_ptr = ptr;
  this->_weak_ref = r._weak_ref;
  r._void_ptr = nullptr;
  r._weak_ref = nullptr;
}

/**
 *
 */
template<class T>
INLINE WeakPointerToBase<T>::
~WeakPointerToBase() {
  WeakReferenceList *old_ref = (WeakReferenceList *)_weak_ref;
  if (old_ref != nullptr && !old_ref->unref()) {
    delete old_ref;
  }
}

/**
 * This is the main work of the PointerTo family.  When the pointer is
 * reassigned, decrement the old reference count and increment the new one.
 */
template<class T>
void WeakPointerToBase<T>::
reassign(To *ptr) {
  if (ptr != (To *)_void_ptr) {
    WeakReferenceList *old_ref = (WeakReferenceList *)_weak_ref;

    _void_ptr = (void *)ptr;
    if (ptr != nullptr) {
      _weak_ref = ptr->weak_ref();
#ifdef DO_MEMORY_USAGE
      update_type(ptr);
#endif
    } else {
      _weak_ref = nullptr;
    }

    // Now remove the old reference.
    if (old_ref != nullptr && !old_ref->unref()) {
      delete old_ref;
    }
  }
}

/**
 *
 */
template<class T>
INLINE void WeakPointerToBase<T>::
reassign(const PointerToBase<To> &copy) {
  // This double-casting is a bit of a cheat to get around the inheritance
  // issue--it's difficult to declare a template class to be a friend.
  reassign((To *)((const WeakPointerToBase<To> *)&copy)->_void_ptr);
}

/**
 *
 */
template<class T>
INLINE void WeakPointerToBase<T>::
reassign(const WeakPointerToBase<To> &copy) {
  void *new_ptr = copy._void_ptr;
  if (new_ptr != _void_ptr) {
    WeakReferenceList *old_ref = (WeakReferenceList *)_weak_ref;
    _void_ptr = new_ptr;

    // While it is tempting to stop maintaining the control block pointer
    // after the object has been deleted, we still need it in order to define
    // a consistent ordering in owner_before.
    WeakReferenceList *weak_ref = copy._weak_ref;
    if (weak_ref != nullptr/* && !weak_ref->was_deleted()*/) {
      weak_ref->ref();
      _weak_ref = weak_ref;
    } else {
      _weak_ref = nullptr;
    }

    // Now remove the old reference.
    if (old_ref != nullptr && !old_ref->unref()) {
      delete old_ref;
    }
  }
}

/**
 *
 */
template<class T>
INLINE void WeakPointerToBase<T>::
reassign(WeakPointerToBase<To> &&from) noexcept {
  // Protect against self-move-assignment.
  if (from._void_ptr != this->_void_ptr) {
    WeakReferenceList *old_ref = (WeakReferenceList *)this->_weak_ref;

    this->_void_ptr = from._void_ptr;
    this->_weak_ref = from._weak_ref;
    from._void_ptr = nullptr;
    from._weak_ref = nullptr;

    // Now delete the old pointer.
    if (old_ref != nullptr && !old_ref->unref()) {
      delete old_ref;
    }
  }
}

/**
 * Like above, but casts from a compatible pointer type.
 */
template<class T>
template<class Y>
INLINE void WeakPointerToBase<T>::
reassign(const WeakPointerToBase<Y> &copy) {
  // If there is a compile error on this line, it means you tried to assign
  // an incompatible type.
  To *new_ptr = (Y *)copy._void_ptr;

  if (new_ptr != (To *)_void_ptr) {
    WeakReferenceList *old_ref = (WeakReferenceList *)_weak_ref;
    WeakReferenceList *new_ref = copy._weak_ref;
    _void_ptr = new_ptr;
    _weak_ref = new_ref;

    if (new_ref != nullptr) {
      new_ref->ref();
    }

    // Now remove the old reference.
    if (old_ref != nullptr && !old_ref->unref()) {
      delete old_ref;
    }
  }
}

/**
 * Like above, but casts from a compatible pointer type.
 */
template<class T>
template<class Y>
INLINE void WeakPointerToBase<T>::
reassign(WeakPointerToBase<Y> &&from) noexcept {
  // Protect against self-move-assignment.
  if (from._void_ptr != this->_void_ptr) {
    WeakReferenceList *old_ref = (WeakReferenceList *)this->_weak_ref;

    // If there is a compile error on this line, it means you tried to assign
    // an incompatible type.
    To *new_ptr = (Y *)from._void_ptr;

    this->_void_ptr = new_ptr;
    this->_weak_ref = from._weak_ref;
    from._void_ptr = nullptr;
    from._weak_ref = nullptr;

    // Now delete the old pointer.
    if (old_ref != nullptr && !old_ref->unref()) {
      delete old_ref;
    }
  }
}

/**
 * Ensures that the MemoryUsage record for the pointer has the right type of
 * object, if we know the type ourselves.
 */
template<class T>
INLINE void WeakPointerToBase<T>::
update_type(To *ptr) {
#ifdef DO_MEMORY_USAGE
  if (MemoryUsage::get_track_memory_usage()) {
    TypeHandle type = get_type_handle(To);
    if (type == TypeHandle::none()) {
      do_init_type(To);
      type = get_type_handle(To);
    }
    if (type != TypeHandle::none()) {
      MemoryUsage::update_type(ptr, type);
    }
  }
#endif  // DO_MEMORY_USAGE
}

/**
 * A thread-safe way to access the underlying pointer; will only write to the
 * given pointer if the underlying pointer has not yet been deleted and is not
 * null.  Note that it may leave the pointer unassigned even if was_deleted()
 * still returns false, which can occur if the object has reached reference
 * count 0 and is about to be destroyed.
 */
template<class T>
INLINE void WeakPointerToBase<T>::
lock_into(PointerToBase<To> &locked) const {
  WeakReferenceList *weak_ref = this->_weak_ref;
  if (weak_ref != nullptr) {
    weak_ref->_lock.lock();
    if (!weak_ref->was_deleted()) {
      // We also need to check that the reference count is not zero (which can
      // happen if the object is currently being destructed), since that could
      // cause double deletion.
      To *plain_ptr = (To *)WeakPointerToBase<T>::_void_ptr;
      if (plain_ptr != nullptr && plain_ptr->ref_if_nonzero()) {
        // It is valid and we successfully grabbed a reference.  Assign it,
        // noting we have already incremented the reference count.
        locked._void_ptr = plain_ptr;
      }
    }
    weak_ref->_lock.unlock();
  }
}

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const To *other) const {
  return (To *)_void_ptr == other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const To *other) const {
  return (To *)_void_ptr != other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const To *other) const {
  return (To *)_void_ptr > other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const To *other) const {
  return (To *)_void_ptr <= other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const To *other) const {
  return (To *)_void_ptr >= other;
}
/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (To *other) const {
  return (To *)_void_ptr == other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (To *other) const {
  return (To *)_void_ptr != other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (To *other) const {
  return (To *)_void_ptr > other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (To *other) const {
  return (To *)_void_ptr <= other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (To *other) const {
  return (To *)_void_ptr >= other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (std::nullptr_t) const {
  return _void_ptr == nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (std::nullptr_t) const {
  return _void_ptr != nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (std::nullptr_t) const {
  return _void_ptr != nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (std::nullptr_t) const {
  return _void_ptr == nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (std::nullptr_t) const {
  return true;
}

/**
 * Returns true if both pointers have the same raw pointer value.  For this to
 * be meaningful, neither pointer may have expired, since if one has expired
 * while the other was allocated at the expired pointer's memory address, this
 * comparison will be true even though they didn't refer to the same object.
 * @see owner_before
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)other._void_ptr;
}

/**
 * @see operator ==
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr != (To *)other._void_ptr;
}

/**
 * Defines an ordering between WeakPointerTo based on their raw pointer value.
 * @deprecated Do not use this.  Use owner_before or std::owner_less instead.
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr > (To *)other._void_ptr;
}

/**
 * Defines an ordering between WeakPointerTo based on their raw pointer value.
 * @deprecated Do not use this.  Use owner_before or std::owner_less instead.
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr <= (To *)other._void_ptr;
}

/**
 * Defines an ordering between WeakPointerTo based on their raw pointer value.
 * @deprecated Do not use this.  Use owner_before or std::owner_less instead.
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr >= (To *)other._void_ptr;
}

/**
 * Returns true if both pointers point to the same object.
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const PointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

/**
 * Returns false if both pointers point to the same object.
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const PointerToBase<To> &other) const {
  return (To *)_void_ptr != (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const PointerToBase<To> &other) const {
  return (To *)_void_ptr > (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const PointerToBase<To> &other) const {
  return (To *)_void_ptr <= (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const PointerToBase<To> &other) const {
  return (To *)_void_ptr >= (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (const To *other) const {
  return (To *)_void_ptr < other;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (std::nullptr_t) const {
  return false;
}

/**
 * Defines an ordering between WeakPointerTo based on their raw pointer value.
 * @deprecated Do not use this.  Use owner_before or std::owner_less instead.
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr < (To *)other._void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (const PointerToBase<To> &other) const {
  return (To *)_void_ptr < (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

#endif  // CPPPARSER

/**
 * Defines an ordering that is guaranteed to remain consistent even after the
 * weak pointers have expired.  This may result in two pointers with the same
 * get_orig() value comparing unequal if one of them is a new object that was
 * allocated at the same memory address as the older, expired pointer.
 */
template<class T>
template<class Y>
INLINE bool WeakPointerToBase<T>::
owner_before(const WeakPointerToBase<Y> &other) const noexcept {
  return _weak_ref < other._weak_ref;
}

/**
 * Defines an ordering that is guaranteed to remain consistent even after this
 * weak pointer has expired.  This may result in two pointers with the same
 * get_orig() value comparing unequal if one of them is a new object that was
 * allocated at the same memory address as the older, expired pointer.
 */
template<class T>
template<class Y>
INLINE bool WeakPointerToBase<T>::
owner_before(const PointerToBase<Y> &other) const noexcept {
  // Unfortunately, this may needlessly cause a control block to be allocated,
  // but I do not see a more efficient solution.
  return (other._void_ptr != nullptr) &&
    (_void_ptr == nullptr || _weak_ref < ((const Y *)other._void_ptr)->get_weak_list());
}

/**
 * A convenient way to set the PointerTo object to NULL. (Assignment to a NULL
 * pointer also works, of course.)
 */
template<class T>
INLINE void WeakPointerToBase<T>::
clear() {
  WeakReferenceList *old_ref = (WeakReferenceList *)_weak_ref;
  _void_ptr = nullptr;
  _weak_ref = nullptr;

  // Now remove the old reference.
  if (old_ref != nullptr && !old_ref->unref()) {
    delete old_ref;
  }
}

/**
 * Informs the WeakPointerTo object that its pointer is no longer deleted.
 * This may be used after a WeakPointerTo has deleted a deleted pointer, and
 * then a new pointer has been reallocated.  It's equivalent to simply
 * reassigning the pointer to its new (i.e.  original) value, but has the
 * advantage that it is const, so can be used for WeakPointers used as keys in
 * STL maps and sets.
 */
template<class T>
INLINE void WeakPointerToBase<T>::
refresh() const {
  if (_void_ptr != nullptr) {
    ((WeakPointerToBase<T> *)this)->reassign((To *)_void_ptr);
  }
}

/**
 * A handy function to output PointerTo's as a hex pointer followed by a
 * reference count.
 */
template<class T>
INLINE void WeakPointerToBase<T>::
output(std::ostream &out) const {
  out << _void_ptr;

  WeakReferenceList *weak_ref = this->_weak_ref;
  if (weak_ref != nullptr) {
    weak_ref->_lock.lock();
    if (!weak_ref->was_deleted()) {
      out << ":" << ((To *)_void_ptr)->get_ref_count();
    } else {
      out << ":deleted";
    }
    weak_ref->_lock.unlock();
  } else {
    out << ":invalid";
  }
}
