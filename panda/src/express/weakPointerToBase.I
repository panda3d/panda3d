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
 *
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
 *
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
 *
 */
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(const WeakPointerToBase<T> &copy) {
  _void_ptr = copy._void_ptr;

  // Don't bother increasing the weak reference count if the object was
  // already deleted.
  WeakReferenceList *weak_ref = copy._weak_ref;
  if (weak_ref != nullptr && !weak_ref->was_deleted()) {
    _weak_ref = copy._weak_ref;
    _weak_ref->ref();
  }
}

/**
 *
 */
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(WeakPointerToBase<T> &&from) noexcept {
  // Protect against self-move-assignment.
  if (from._void_ptr != this->_void_ptr) {
    WeakReferenceList *old_ref = (To *)this->_weak_ref;

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

    // Don't bother increasing the weak reference count if the object was
    // already deleted.
    WeakReferenceList *weak_ref = copy._weak_ref;
    if (weak_ref != nullptr && !weak_ref->was_deleted()) {
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

#ifndef CPPPARSER
#ifndef WIN32_VC
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
operator == (nullptr_t) const {
  return _void_ptr == nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (nullptr_t) const {
  return _void_ptr != nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (nullptr_t) const {
  return _void_ptr != nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (nullptr_t) const {
  return _void_ptr == nullptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (nullptr_t) const {
  return true;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)other._void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr != (To *)other._void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr > (To *)other._void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr <= (To *)other._void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr >= (To *)other._void_ptr;
}

/**
 *
 */
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const PointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

/**
 *
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
#endif  // WIN32_VC

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
operator < (nullptr_t) const {
  return false;
}

/**
 *
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
output(ostream &out) const {
  out << _void_ptr;
  if (was_deleted()) {
    out << ":deleted";
  } else if (_void_ptr != (void *)NULL) {
    out << ":" << ((To *)_void_ptr)->get_ref_count();
  }
}
