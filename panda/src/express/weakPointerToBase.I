// Filename: weakPointerToBase.I
// Created by:  drose (27Sep04)
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
//     Function: WeakPointerToBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(To *ptr) {
  reassign(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(const PointerToBase<T> &copy) {
  reassign(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerToBase<T>::
WeakPointerToBase(const WeakPointerToBase<T> &copy) {
  reassign(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE WeakPointerToBase<T>::
~WeakPointerToBase() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::reassign
//       Access: Protected
//  Description: This is the main work of the PointerTo family.  When
//               the pointer is reassigned, decrement the old
//               reference count and increment the new one.
////////////////////////////////////////////////////////////////////
template<class T>
void WeakPointerToBase<T>::
reassign(To *ptr) {
  if (ptr != (To *)_void_ptr || _ptr_was_deleted) {
    To *old_ptr = (To *)_void_ptr;

    _void_ptr = (void *)ptr;
    if (ptr != (To *)NULL) {
      ptr->weak_ref(this);
#ifdef DO_MEMORY_USAGE
      if (MemoryUsage::get_track_memory_usage()) {
        // Make sure the MemoryUsage record knows what the TypeHandle
        // is, if we know it ourselves.
        TypeHandle type = get_type_handle(To);
        if (type == TypeHandle::none()) {
          do_init_type(To);
          type = get_type_handle(To);
        }
        if (type != TypeHandle::none()) {
          MemoryUsage::update_type(ptr, type);
        }
      }
#endif
    }

    // Now remove the old reference.
    if (old_ptr != (To *)NULL && !_ptr_was_deleted) {
      old_ptr->weak_unref(this);
    }

    _ptr_was_deleted = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::reassign
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void WeakPointerToBase<T>::
reassign(const PointerToBase<To> &copy) {
  // This double-casting is a bit of a cheat to get around the
  // inheritance issue--it's difficult to declare a template class to
  // be a friend.
  reassign((To *)((const WeakPointerToBase<To> *)&copy)->_void_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::reassign
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void WeakPointerToBase<T>::
reassign(const WeakPointerToBase<To> &copy) {
  nassertv(!copy.was_deleted());
  reassign((To *)copy._void_ptr);
}

#ifndef CPPPARSER
#ifndef WIN32_VC
////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const To *other) const {
  return (To *)_void_ptr == other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const To *other) const {
  return (To *)_void_ptr != other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const To *other) const {
  return (To *)_void_ptr > other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const To *other) const {
  return (To *)_void_ptr <= other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const To *other) const {
  return (To *)_void_ptr >= other;
}
////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (To *other) const {
  return (To *)_void_ptr == other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (To *other) const {
  return (To *)_void_ptr != other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (To *other) const {
  return (To *)_void_ptr > other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (To *other) const {
  return (To *)_void_ptr <= other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (To *other) const {
  return (To *)_void_ptr >= other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr != (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr > (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr <= (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr >= (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator == (const PointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator != (const PointerToBase<To> &other) const {
  return (To *)_void_ptr != (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator > (const PointerToBase<To> &other) const {
  return (To *)_void_ptr > (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator <= (const PointerToBase<To> &other) const {
  return (To *)_void_ptr <= (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator >= (const PointerToBase<To> &other) const {
  return (To *)_void_ptr >= (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}
#endif  // WIN32_VC

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (const To *other) const {
  return (To *)_void_ptr < other;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (const WeakPointerToBase<To> &other) const {
  return (To *)_void_ptr < (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
operator < (const PointerToBase<To> &other) const {
  return (To *)_void_ptr < (To *)((WeakPointerToBase<To> *)&other)->_void_ptr;
}

#endif  // CPPPARSER



////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::is_null
//       Access: Public
//  Description: Returns true if the PointerTo is a NULL pointer,
//               false otherwise.  (Direct comparison to a NULL
//               pointer also works.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool WeakPointerToBase<T>::
is_null() const {
  return (_void_ptr == (void *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::clear
//       Access: Public
//  Description: A convenient way to set the PointerTo object to NULL.
//               (Assignment to a NULL pointer also works, of course.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void WeakPointerToBase<T>::
clear() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: WeakPointerToBase::output
//       Access: Public
//  Description: A handy function to output PointerTo's as a hex
//               pointer followed by a reference count.
////////////////////////////////////////////////////////////////////
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
