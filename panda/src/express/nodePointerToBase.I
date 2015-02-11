// Filename: nodePointerToBase.I
// Created by:  drose (07May05)
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
//     Function: NodePointerToBase::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerToBase<T>::
NodePointerToBase(To *ptr) {
  reassign(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerToBase<T>::
NodePointerToBase(const NodePointerToBase<T> &copy) {
  reassign(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::Destructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerToBase<T>::
~NodePointerToBase() {
  reassign((To *)NULL);
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::Move Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE NodePointerToBase<T>::
NodePointerToBase(NodePointerToBase<T> &&from) NOEXCEPT {
  _void_ptr = from._void_ptr;
  from._void_ptr = (void *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::reassign
//       Access: Protected
//  Description: This version of reassign is called when a
//               NodePointerTo is assigned to this Node PointerTo
//               as an rvalue.  In this case, we can steal the
//               reference count from the other PointerTo, without
//               needing to call ref() and unref() unnecessarily.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void NodePointerToBase<T>::
reassign(NodePointerToBase<T> &&from) NOEXCEPT {
  To *old_ptr = (To *)this->_void_ptr;

  this->_void_ptr = from._void_ptr;
  from._void_ptr = NULL;

  // Now delete the old pointer.
  if (old_ptr != (To *)NULL) {
    node_unref_delete(old_ptr);
  }
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::reassign
//       Access: Protected
//  Description: This is the main work of the NodePointerTo family.  When
//               the pointer is reassigned, decrement the old
//               reference count and increment the new one.
////////////////////////////////////////////////////////////////////
template<class T>
void NodePointerToBase<T>::
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
      ptr->node_ref();
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
      node_unref_delete(old_ptr);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::reassign
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void NodePointerToBase<T>::
reassign(const NodePointerToBase<To> &copy) {
  reassign((To *)copy._void_ptr);
}


////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::clear
//       Access: Published
//  Description: A convenient way to set the NodePointerTo object to NULL.
//               (Assignment to a NULL pointer also works, of course.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void NodePointerToBase<T>::
clear() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePointerToBase::output
//       Access: Published
//  Description: A handy function to output NodePointerTo's as a hex
//               pointer followed by a reference count.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void NodePointerToBase<T>::
output(ostream &out) const {
  out << _void_ptr;
  if (_void_ptr != (void *)NULL) {
    out << ":" << ((To *)_void_ptr)->get_node_ref_count() << "/"
        << ((To *)_void_ptr)->get_ref_count();
  }
}
