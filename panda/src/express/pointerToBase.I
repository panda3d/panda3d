/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerToBase.I
 * @author drose
 * @date 2004-09-27
 */

/**
 *
 */
template<class T>
INLINE PointerToBase<T>::
PointerToBase(To *ptr) {
  _void_ptr = (void *)ptr;
  if (ptr != (To *)NULL) {
    ptr->ref();
#ifdef DO_MEMORY_USAGE
    update_type(ptr);
#endif
  }
}

/**
 *
 */
template<class T>
INLINE PointerToBase<T>::
PointerToBase(const PointerToBase<T> &copy) {
  _void_ptr = copy._void_ptr;
  if (_void_ptr != NULL) {
    To *ptr = (To *)_void_ptr;
    ptr->ref();
  }
}

/**
 *
 */
template<class T>
INLINE PointerToBase<T>::
~PointerToBase() {
  if (_void_ptr != NULL) {
    unref_delete((To *)_void_ptr);
    _void_ptr = NULL;
  }
}

/**
 *
 */
template<class T>
INLINE PointerToBase<T>::
PointerToBase(PointerToBase<T> &&from) noexcept {
  _void_ptr = from._void_ptr;
  from._void_ptr = (void *)NULL;
}

/**
 * This version of reassign is called when a PointerTo is assigned to this
 * PointerTo as an rvalue.  In this case, we can steal the reference count
 * from the other PointerTo, without needing to call ref() and unref()
 * unnecessarily.
 */
template<class T>
INLINE void PointerToBase<T>::
reassign(PointerToBase<T> &&from) noexcept {
  // Protect against self-move-assignment.
  if (from._void_ptr != this->_void_ptr) {
    To *old_ptr = (To *)this->_void_ptr;

    this->_void_ptr = from._void_ptr;
    from._void_ptr = NULL;

    // Now delete the old pointer.
    if (old_ptr != (To *)NULL) {
      unref_delete(old_ptr);
    }
  }
}

/**
 * This is the main work of the PointerTo family.  When the pointer is
 * reassigned, decrement the old reference count and increment the new one.
 */
template<class T>
INLINE void PointerToBase<T>::
reassign(To *ptr) {
  if (ptr != (To *)_void_ptr) {
    // First save the old pointer; we won't delete it until we have assigned
    // the new one.  We do this just in case there are cascading effects from
    // deleting this pointer that might inadvertently delete the new one.
    // (Don't laugh--it's happened!)
    To *old_ptr = (To *)_void_ptr;

    _void_ptr = (void *)ptr;
    if (ptr != nullptr) {
      ptr->ref();
#ifdef DO_MEMORY_USAGE
      update_type(ptr);
#endif
    }

    // Now delete the old pointer.
    if (old_ptr != nullptr) {
      unref_delete(old_ptr);
    }
  }
}

/**
 *
 */
template<class T>
INLINE void PointerToBase<T>::
reassign(const PointerToBase<To> &copy) {
  if (copy._void_ptr != _void_ptr) {
    // First save the old pointer; we won't delete it until we have assigned
    // the new one.  We do this just in case there are cascading effects from
    // deleting this pointer that might inadvertently delete the new one.
    // (Don't laugh--it's happened!)
    To *old_ptr = (To *)_void_ptr;
    To *new_ptr = (To *)copy._void_ptr;

    _void_ptr = copy._void_ptr;
    if (new_ptr != nullptr) {
      new_ptr->ref();
    }

    // Now delete the old pointer.
    if (old_ptr != nullptr) {
      unref_delete(old_ptr);
    }
  }
}

/**
 * Ensures that the MemoryUsage record for the pointer has the right type of
 * object, if we know the type ourselves.
 */
template<class T>
INLINE void PointerToBase<T>::
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
 * A convenient way to set the PointerTo object to NULL. (Assignment to a NULL
 * pointer also works, of course.)
 */
template<class T>
ALWAYS_INLINE void PointerToBase<T>::
clear() {
  reassign((To *)NULL);
}

/**
 * A handy function to output PointerTo's as a hex pointer followed by a
 * reference count.
 */
template<class T>
INLINE void PointerToBase<T>::
output(ostream &out) const {
  out << _void_ptr;
  if (_void_ptr != (void *)NULL) {
    out << ":" << ((To *)_void_ptr)->get_ref_count();
  }
}
