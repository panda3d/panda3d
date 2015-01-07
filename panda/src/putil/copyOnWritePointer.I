// Filename: copyOnWritePointer.I
// Created by:  drose (09Apr07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWritePointer::
CopyOnWritePointer(CopyOnWriteObject *object) :
  _cow_object(object)
{
  if (_cow_object != (CopyOnWriteObject *)NULL) {
    _cow_object->cache_ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWritePointer::
CopyOnWritePointer(const CopyOnWritePointer &copy) :
  _cow_object(copy._cow_object)
{
  if (_cow_object != (CopyOnWriteObject *)NULL) {
    _cow_object->cache_ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CopyOnWritePointer::
operator = (const CopyOnWritePointer &copy) {
  operator = (copy._cow_object);
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CopyOnWritePointer::
operator = (CopyOnWriteObject *object) {
  if (_cow_object != object) {
    if (_cow_object != (CopyOnWriteObject *)NULL) {
      cache_unref_delete(_cow_object);
    }
    _cow_object = object;
    if (_cow_object != (CopyOnWriteObject *)NULL) {
      _cow_object->cache_ref();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWritePointer::
~CopyOnWritePointer() {
  if (_cow_object != (CopyOnWriteObject *)NULL) {
    cache_unref_delete(_cow_object);
  }
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWritePointer::
CopyOnWritePointer(CopyOnWritePointer &&move) NOEXCEPT :
  _cow_object(move._cow_object)
{
  // Steal the other's reference count.
  move._cow_object = (CopyOnWriteObject *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CopyOnWritePointer::
operator = (CopyOnWritePointer &&move) NOEXCEPT {
  CopyOnWriteObject *old_object = _cow_object;
  _cow_object = move._cow_object;
  move._cow_object = NULL;

  if (old_object != (CopyOnWriteObject *)NULL) {
    cache_unref_delete(old_object);
  }
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::operator == 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CopyOnWritePointer::
operator == (const CopyOnWritePointer &other) const {
  return _cow_object == other._cow_object;
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::operator != 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CopyOnWritePointer::
operator != (const CopyOnWritePointer &other) const {
  return _cow_object != other._cow_object;
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::operator < 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CopyOnWritePointer::
operator < (const CopyOnWritePointer &other) const {
  return _cow_object < other._cow_object;
}

#ifndef COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::get_read_pointer
//       Access: Public
//  Description: Returns a pointer locked for read.  Until this
//               pointer dereferences, calls to get_write_pointer()
//               will force a copy.
//
//               This flavor of the method is written for the
//               non-threaded case.
////////////////////////////////////////////////////////////////////
INLINE const CopyOnWriteObject *CopyOnWritePointer::
get_read_pointer() const {
  return _cow_object;
}
#endif  // COW_THREADED

#ifndef COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::get_write_pointer
//       Access: Public
//  Description: Returns a pointer locked for write.  If another
//               thread or threads already hold the pointer locked for
//               read, then this will force a copy.
//
//               Until this pointer dereferences, calls to
//               get_read_pointer() or get_write_pointer() will block.
//
//               This flavor of the method is written for the
//               non-threaded case.
////////////////////////////////////////////////////////////////////
INLINE CopyOnWriteObject *CopyOnWritePointer::
get_write_pointer() {
  if (_cow_object == (CopyOnWriteObject *)NULL) {
    return NULL;
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

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::get_unsafe_pointer
//       Access: Public
//  Description: Returns an unlocked pointer that you can write to.
//               This should only be used in very narrow circumstances
//               in which you know that no other thread may be
//               accessing the pointer at the same time.
////////////////////////////////////////////////////////////////////
INLINE CopyOnWriteObject *CopyOnWritePointer::
get_unsafe_pointer() {
  return _cow_object;
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::is_null
//       Access: Public
//  Description: Returns true if the CopyOnWritePointer contains a
//               NULL pointer, false otherwise.
////////////////////////////////////////////////////////////////////
bool CopyOnWritePointer::
is_null() const {
  return (_cow_object == (CopyOnWriteObject *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::clear
//       Access: Public
//  Description: Sets the pointer to NULL.
////////////////////////////////////////////////////////////////////
void CopyOnWritePointer::
clear() {
  if (_cow_object != (CopyOnWriteObject *)NULL) {
    cache_unref_delete(_cow_object);
  }
  _cow_object = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::test_ref_count_integrity
//       Access: Published
//  Description: Does some easy checks to make sure that the reference
//               count isn't completely bogus.  Returns true if ok,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CopyOnWritePointer::
test_ref_count_integrity() const {
  nassertr(_cow_object != (CopyOnWriteObject *)NULL, false);
  return _cow_object->test_ref_count_integrity();
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::test_ref_count_nonzero
//       Access: Published
//  Description: Does some easy checks to make sure that the reference
//               count isn't zero, or completely bogus.  Returns true
//               if ok, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CopyOnWritePointer::
test_ref_count_nonzero() const {
  nassertr(_cow_object != (CopyOnWriteObject *)NULL, false);
  return _cow_object->test_ref_count_nonzero();
}

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(To *object) : CopyOnWritePointer(object) {
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(const CopyOnWritePointerTo<T> &copy) :
  CopyOnWritePointer(copy)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (const CopyOnWritePointerTo<T> &copy) {
  CopyOnWritePointer::operator = (copy);
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointer::Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (To *object) {
  CopyOnWritePointer::operator = (object);
}
#endif  // CPPPARSER

#ifdef USE_MOVE_SEMANTICS
#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE CopyOnWritePointerTo<T>::
CopyOnWritePointerTo(CopyOnWritePointerTo<T> &&move) NOEXCEPT :
  CopyOnWritePointer((CopyOnWritePointer &&)move)
{
}
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::Move Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void CopyOnWritePointerTo<T>::
operator = (CopyOnWritePointerTo<T> &&move) NOEXCEPT {
  CopyOnWritePointer::operator = ((CopyOnWritePointer &&)move);
}
#endif  // CPPPARSER
#endif  // USE_MOVE_SEMANTICS

#ifndef CPPPARSER
#ifdef COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::get_read_pointer
//       Access: Public
//  Description: See CopyOnWritePointer::get_read_pointer().
////////////////////////////////////////////////////////////////////
template<class T>
INLINE CPT(TYPENAME CopyOnWritePointerTo<T>::To) CopyOnWritePointerTo<T>::
get_read_pointer() const {
  return (const To *)(CopyOnWritePointer::get_read_pointer().p());
}
#else  // COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::get_read_pointer
//       Access: Public
//  Description: See CopyOnWritePointer::get_read_pointer().
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const TYPENAME CopyOnWritePointerTo<T>::To *CopyOnWritePointerTo<T>::
get_read_pointer() const {
  return (const To *)CopyOnWritePointer::get_read_pointer();
}
#endif  // COW_THREADED
#endif  // CPPPARSER

#ifndef CPPPARSER
#ifdef COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::get_write_pointer
//       Access: Public
//  Description: See CopyOnWritePointer::get_write_pointer().
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PT(TYPENAME CopyOnWritePointerTo<T>::To) CopyOnWritePointerTo<T>::
get_write_pointer() {
  return (To *)(CopyOnWritePointer::get_write_pointer().p());
}
#else  // COW_THREADED
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::get_write_pointer
//       Access: Public
//  Description: See CopyOnWritePointer::get_write_pointer().
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME CopyOnWritePointerTo<T>::To *CopyOnWritePointerTo<T>::
get_write_pointer() {
  return (To *)CopyOnWritePointer::get_write_pointer();
}
#endif  // COW_THREADED
#endif  // CPPPARSER

#ifndef CPPPARSER
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWritePointerTo::get_unsafe_pointer
//       Access: Public
//  Description: See CopyOnWritePointer::get_unsafe_pointer().
////////////////////////////////////////////////////////////////////
template<class T>
INLINE TYPENAME CopyOnWritePointerTo<T>::To *CopyOnWritePointerTo<T>::
get_unsafe_pointer() {
  return (To *)(CopyOnWritePointer::get_unsafe_pointer());
}
#endif  // CPPPARSER
