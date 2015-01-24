// Filename: pointerToBase.I
// Created by:  drose (27Sep04)
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

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Move Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
PointerToBase(PointerToBase<T> &&from) NOEXCEPT {
  _void_ptr = from._void_ptr;
  from._void_ptr = (void *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::reassign
//       Access: Protected
//  Description: This version of reassign is called when a PointerTo
//               is assigned to this PointerTo as an rvalue.  In
//               this case, we can steal the reference count from
//               the other PointerTo, without needing to call ref()
//               and unref() unnecessarily.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
reassign(PointerToBase<T> &&from) NOEXCEPT {
  To *old_ptr = (To *)this->_void_ptr;

  this->_void_ptr = from._void_ptr;
  from._void_ptr = NULL;

  // Now delete the old pointer.
  if (old_ptr != (To *)NULL) {
    unref_delete(old_ptr);
  }
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::reassign
//       Access: Protected
//  Description: This is the main work of the PointerTo family.  When
//               the pointer is reassigned, decrement the old
//               reference count and increment the new one.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
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
        update_type(ptr);
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

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::update_type
//       Access: Protected
//  Description: Ensures that the MemoryUsage record for the pointer
//               has the right type of object, if we know the type
//               ourselves.
////////////////////////////////////////////////////////////////////
template<class T>
void PointerToBase<T>::
update_type(To *ptr) {
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


////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::clear
//       Access: Published
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
//       Access: Published
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
