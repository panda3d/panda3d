// Filename: threadSafePointerToBase.I
// Created by:  drose (28Apr06)
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
//     Function: ThreadSafePointerToBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ThreadSafePointerToBase<T>::
ThreadSafePointerToBase(To *ptr) {
  reassign(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSafePointerToBase::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ThreadSafePointerToBase<T>::
ThreadSafePointerToBase(const ThreadSafePointerToBase<T> &copy) {
  reassign(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSafePointerToBase::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ThreadSafePointerToBase<T>::
~ThreadSafePointerToBase() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSafePointerToBase::reassign
//       Access: Protected
//  Description: This is the main work of the ThreadSafePointerTo family.  When
//               the pointer is reassigned, decrement the old
//               reference count and increment the new one.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void ThreadSafePointerToBase<T>::
reassign(To *ptr) {
  To *old_ptr = (To *)AtomicAdjust::get_ptr(_void_ptr);
  if (ptr == old_ptr) {
    return;
  }

#ifdef HAVE_THREADS
  void *orig_ptr = AtomicAdjust::compare_and_exchange_ptr(_void_ptr, old_ptr, ptr);
  while (orig_ptr != old_ptr) {
    // Some other thread assigned it first.  Try again.
    old_ptr = (To *)AtomicAdjust::get_ptr(_void_ptr);
    if (ptr == old_ptr) {
      return;
    }

    orig_ptr = AtomicAdjust::compare_and_exchange_ptr(_void_ptr, old_ptr, ptr);
  }
#else  // HAVE_THREADS
  _void_ptr = ptr;
#endif  // HAVE_THREADS

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

////////////////////////////////////////////////////////////////////
//     Function: ThreadSafePointerToBase::reassign
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void ThreadSafePointerToBase<T>::
reassign(const ThreadSafePointerToBase<To> &copy) {
  reassign((To *)copy._void_ptr);
}

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: ThreadSafePointerToBase::update_type
//       Access: Protected
//  Description: Ensures that the MemoryUsage record for the pointer
//               has the right type of object, if we know the type
//               ourselves.
////////////////////////////////////////////////////////////////////
template<class T>
void ThreadSafePointerToBase<T>::
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
//     Function: ThreadSafePointerToBase::clear
//       Access: Published
//  Description: A convenient way to set the ThreadSafePointerTo object to NULL.
//               (Assignment to a NULL pointer also works, of course.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void ThreadSafePointerToBase<T>::
clear() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ThreadSafePointerToBase::output
//       Access: Published
//  Description: A handy function to output ThreadSafePointerTo's as a hex
//               pointer followed by a reference count.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void ThreadSafePointerToBase<T>::
output(ostream &out) const {
  out << _void_ptr;
  if (_void_ptr != (void *)NULL) {
    out << ":" << ((To *)_void_ptr)->get_ref_count();
  }
}
