/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copyOnWritePointer.I
 * @author drose
 * @date 2007-04-09
 */

/**
 *
 */
INLINE CopyOnWritePointer::
CopyOnWritePointer(CopyOnWriteObject *object) :
  _cow_object(object)
{
  if (_cow_object != nullptr) {
    _cow_object->cache_ref();
  }
}

/**
 *
 */
INLINE CopyOnWritePointer::
CopyOnWritePointer(const CopyOnWritePointer &copy) :
  _cow_object(copy._cow_object)
{
  if (_cow_object != nullptr) {
    _cow_object->cache_ref();
  }
}

/**
 *
 */
INLINE void CopyOnWritePointer::
operator = (const CopyOnWritePointer &copy) {
  operator = (copy._cow_object);
}

/**
 *
 */
INLINE void CopyOnWritePointer::
operator = (CopyOnWriteObject *object) {
  if (_cow_object != object) {
    if (_cow_object != nullptr) {
      cache_unref_delete(_cow_object);
    }
    _cow_object = object;
    if (_cow_object != nullptr) {
      _cow_object->cache_ref();
    }
  }
}

/**
 *
 */
INLINE CopyOnWritePointer::
~CopyOnWritePointer() {
  if (_cow_object != nullptr) {
    cache_unref_delete(_cow_object);
  }
}

/**
 *
 */
INLINE CopyOnWritePointer::
CopyOnWritePointer(CopyOnWritePointer &&from) noexcept :
  _cow_object(from._cow_object)
{
  // Steal the other's reference count.
  from._cow_object = nullptr;
}

/**
 *
 */
INLINE CopyOnWritePointer::
CopyOnWritePointer(PointerTo<CopyOnWriteObject> &&from) noexcept :
  _cow_object(from.p())
{
  // Steal the other's reference count, but because it is a regular pointer,
  // we do need to include the cache reference count.
  if (_cow_object != nullptr) {
    _cow_object->cache_ref_only();
  }
  from.cheat() = nullptr;
}

/**
 *
 */
INLINE void CopyOnWritePointer::
operator = (CopyOnWritePointer &&from) noexcept {
  // Protect against self-move-assignment.
  if (from._cow_object != _cow_object) {
    CopyOnWriteObject *old_object = _cow_object;
    _cow_object = from._cow_object;
    from._cow_object = nullptr;

    if (old_object != nullptr) {
      cache_unref_delete(old_object);
    }
  }
}

/**
 *
 */
INLINE void CopyOnWritePointer::
operator = (PointerTo<CopyOnWriteObject> &&from) noexcept {
  if (from.p() != _cow_object) {
    CopyOnWriteObject *old_object = _cow_object;

    // Steal the other's reference count, but because it is a regular pointer,
    // we do need to include the cache reference count.
    _cow_object = from.p();
    _cow_object->cache_ref_only();
    from.cheat() = nullptr;

    if (old_object != nullptr) {
      cache_unref_delete(old_object);
    }
  }
}

/**
 *
 */
INLINE bool CopyOnWritePointer::
operator == (const CopyOnWritePointer &other) const {
  return _cow_object == other._cow_object;
}

/**
 *
 */
INLINE bool CopyOnWritePointer::
operator != (const CopyOnWritePointer &other) const {
  return _cow_object != other._cow_object;
}

/**
 *
 */
INLINE bool CopyOnWritePointer::
operator < (const CopyOnWritePointer &other) const {
  return _cow_object < other._cow_object;
}

#ifndef COW_THREADED
/**
 * Returns a pointer locked for read.  Until this pointer dereferences, calls
 * to get_write_pointer() will force a copy.
 *
 * This flavor of the method is written for the non-threaded case.
 */
INLINE const CopyOnWriteObject *CopyOnWritePointer::
get_read_pointer(Thread *current_thread) const {
  return _cow_object;
}
#endif  // COW_THREADED

#ifndef COW_THREADED
/**
 * Returns a pointer locked for write.  If another thread or threads already
 * hold the pointer locked for read, then this will force a copy.
 *
 * Until this pointer dereferences, calls to get_read_pointer() or
 * get_write_pointer() will block.
 *
 * This flavor of the method is written for the non-threaded case.
 */
INLINE CopyOnWriteObject *CopyOnWritePointer::
get_write_pointer() {
  if (_cow_object == nullptr) {
    return nullptr;
  }
  if (_cow_object->get_cache_ref_count() > 1) {
    PT(CopyOnWriteObject) new_object = _cow_object->make_cow_copy();
    cache_unref_delete(_cow_object);
    _cow_object = new_object;
    _cow_object->cache_ref();
  }
  return _cow_object;
}
#endif  // COW_THREADED

/**
 * Returns an unlocked pointer that you can write to.  This should only be
 * used in very narrow circumstances in which you know that no other thread
 * may be accessing the pointer at the same time.
 */
INLINE CopyOnWriteObject *CopyOnWritePointer::
get_unsafe_pointer() {
  return _cow_object;
}

/**
 * Returns true if the CopyOnWritePointer contains a NULL pointer, false
 * otherwise.
 */
bool CopyOnWritePointer::
is_null() const {
  return (_cow_object == nullptr);
}

/**
 * Sets the pointer to NULL.
 */
void CopyOnWritePointer::
clear() {
  if (_cow_object != nullptr) {
    cache_unref_delete(_cow_object);
  }
  _cow_object = nullptr;
}

/**
 * Does some easy checks to make sure that the reference count isn't
 * completely bogus.  Returns true if ok, false otherwise.
 */
INLINE bool CopyOnWritePointer::
test_ref_count_integrity() const {
  nassertr(_cow_object != nullptr, false);
  return _cow_object->test_ref_count_integrity();
}

/**
 * Does some easy checks to make sure that the reference count isn't zero, or
 * completely bogus.  Returns true if ok, false otherwise.
 */
INLINE bool CopyOnWritePointer::
test_ref_count_nonzero() const {
  nassertr(_cow_object != nullptr, false);
  return _cow_object->test_ref_count_nonzero();
}

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(To *object) : CopyOnWritePointer(object) {
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(const CopyOnWritePointerTo<T> &copy) :
  CopyOnWritePointer(copy)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (const CopyOnWritePointerTo<T> &copy) {
  CopyOnWritePointer::operator = (copy);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (To *object) {
  CopyOnWritePointer::operator = (object);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(CopyOnWritePointerTo<T> &&from) noexcept :
  CopyOnWritePointer((CopyOnWritePointer &&)from)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(PointerTo<T> &&from) noexcept {
  // Steal the other's reference count, but because it is a regular pointer,
  // we do need to include the cache reference count.
  _cow_object = from.p();
  if (_cow_object != nullptr) {
    _cow_object->cache_ref_only();
  }
  from.cheat() = nullptr;
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (CopyOnWritePointerTo<T> &&from) noexcept {
  CopyOnWritePointer::operator = ((CopyOnWritePointer &&)from);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 *
 */
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (PointerTo<T> &&from) noexcept {
  if (from.p() != _cow_object) {
    CopyOnWriteObject *old_object = _cow_object;

    // Steal the other's reference count, but because it is a regular pointer,
    // we do need to include the cache reference count.
    _cow_object = from.p();
    _cow_object->cache_ref_only();
    from.cheat() = nullptr;

    if (old_object != nullptr) {
      cache_unref_delete(old_object);
    }
  }
}
#endif  // CPPPARSER

#ifndef CPPPARSER
#ifdef COW_THREADED
/**
 * See CopyOnWritePointer::get_read_pointer().
 */
template<class T>
INLINE CPT(typename CopyOnWritePointerTo<T>::To) CopyOnWritePointerTo<T>::
get_read_pointer(Thread *current_thread) const {
  // This is necessary because we don't currently have a way to cast between
  // two compatible PointerTo types without losing the reference count.
  CPT(typename CopyOnWritePointerTo<T>::To) to;
  CPT(CopyOnWriteObject) from = CopyOnWritePointer::get_read_pointer(current_thread);
  to.cheat() = (const To *)from.p();
  from.cheat() = nullptr;
  return to;
}
#else  // COW_THREADED
/**
 * See CopyOnWritePointer::get_read_pointer().
 */
template<class T>
INLINE const typename CopyOnWritePointerTo<T>::To *CopyOnWritePointerTo<T>::
get_read_pointer(Thread *current_thread) const {
  return (const To *)CopyOnWritePointer::get_read_pointer(current_thread);
}
#endif  // COW_THREADED
#endif  // CPPPARSER

#ifndef CPPPARSER
#ifdef COW_THREADED
/**
 * See CopyOnWritePointer::get_write_pointer().
 */
template<class T>
INLINE PT(typename CopyOnWritePointerTo<T>::To) CopyOnWritePointerTo<T>::
get_write_pointer() {
  // This is necessary because we don't currently have a way to cast between
  // two compatible PointerTo types without losing the reference count.
  PT(typename CopyOnWritePointerTo<T>::To) to;
  PT(CopyOnWriteObject) from = CopyOnWritePointer::get_write_pointer();
  to.cheat() = (To *)from.p();
  from.cheat() = nullptr;
  return to;
}
#else  // COW_THREADED
/**
 * See CopyOnWritePointer::get_write_pointer().
 */
template<class T>
INLINE typename CopyOnWritePointerTo<T>::To *CopyOnWritePointerTo<T>::
get_write_pointer() {
  return (To *)CopyOnWritePointer::get_write_pointer();
}
#endif  // COW_THREADED
#endif  // CPPPARSER

#ifndef CPPPARSER
/**
 * See CopyOnWritePointer::get_unsafe_pointer().
 */
template<class T>
INLINE typename CopyOnWritePointerTo<T>::To *CopyOnWritePointerTo<T>::
get_unsafe_pointer() {
  return (To *)(CopyOnWritePointer::get_unsafe_pointer());
}
#endif  // CPPPARSER
