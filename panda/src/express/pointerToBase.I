// Filename: pointerToBase.I
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
//     Function: PointerToBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
PointerToBase(To *ptr) {
  reassign(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
PointerToBase(const PointerToBase<T> &copy) {
  reassign(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
~PointerToBase() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::reassign
//       Access: Protected
//  Description: This is the main work of the PointerTo family.  When
//               the pointer is reassigned, decrement the old
//               reference count and increment the new one.
////////////////////////////////////////////////////////////////////
template<class T>
void PointerToBase<T>::
reassign(To *ptr) {
  if (ptr != (To *)_void_ptr) {
    // First save the old pointer; we won't delete it until we have
    // assigned the new one.  We do this just in case there are
    // cascading effects from deleting this pointer that might
    // inadvertently delete the new one.  (Don't laugh--it's
    // happened!)
    To *old_ptr = (To *)_void_ptr;

    _void_ptr = (void *)ptr;
    if (ptr != (To *)NULL) {
      ptr->ref();
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

    // Now delete the old pointer.
    if (old_ptr != (To *)NULL) {
      unref_delete(old_ptr);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::reassign
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
reassign(const PointerToBase<To> &copy) {
  reassign((To *)copy._void_ptr);
}

#ifndef CPPPARSER
#ifndef WIN32_VC
////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator == (const To *other) const {
  return (To *)_void_ptr == other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator != (const To *other) const {
  return (To *)_void_ptr != other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator > (const To *other) const {
  return (To *)_void_ptr > other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator <= (const To *other) const {
  return (To *)_void_ptr <= other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator >= (const To *other) const {
  return (To *)_void_ptr >= other;
}
////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator == (To *other) const {
  return (To *)_void_ptr == other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator != (To *other) const {
  return (To *)_void_ptr != other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator > (To *other) const {
  return (To *)_void_ptr > other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator <= (To *other) const {
  return (To *)_void_ptr <= other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator >= (To *other) const {
  return (To *)_void_ptr >= other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Equivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator == (const PointerToBase<To> &other) const {
  return (To *)_void_ptr == (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Nonequivalence operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator != (const PointerToBase<To> &other) const {
  return (To *)_void_ptr != (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator > (const PointerToBase<To> &other) const {
  return (To *)_void_ptr > (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator <= (const PointerToBase<To> &other) const {
  return (To *)_void_ptr <= (To *)other._void_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator >= (const PointerToBase<To> &other) const {
  return (To *)_void_ptr >= (To *)other._void_ptr;
}
#endif  // WIN32_VC

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator < (const To *other) const {
  return (To *)_void_ptr < other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator < (const PointerToBase<To> &other) const {
  return (To *)_void_ptr < (To *)other._void_ptr;
}

#endif  // CPPPARSER



////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::is_null
//       Access: Public
//  Description: Returns true if the PointerTo is a NULL pointer,
//               false otherwise.  (Direct comparison to a NULL
//               pointer also works.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
is_null() const {
  return (_void_ptr == (void *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::clear
//       Access: Public
//  Description: A convenient way to set the PointerTo object to NULL.
//               (Assignment to a NULL pointer also works, of course.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
clear() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::output
//       Access: Public
//  Description: A handy function to output PointerTo's as a hex
//               pointer followed by a reference count.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
output(ostream &out) const {
  out << _void_ptr;
  if (_void_ptr != (void *)NULL) {
    out << ":" << ((To *)_void_ptr)->get_ref_count();
  }
}
