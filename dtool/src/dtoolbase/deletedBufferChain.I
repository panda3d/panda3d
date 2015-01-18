// Filename: deletedBufferChain.I
// Created by:  drose (20Jul07)
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
//     Function: DeletedBufferChain::validate
//       Access: Public
//  Description: Returns true if the pointer is valid, false if it has
//               been deleted or if it was never a valid pointer.
//
//               This is only meaningful in debug mode, where
//               USE_DELETEDCHAINFLAG is defined.  If not, this
//               trivially returns true.
////////////////////////////////////////////////////////////////////
INLINE bool DeletedBufferChain::
validate(void *ptr) {
  TAU_PROFILE("bool DeletedBufferChain::validate(void *)", " ", TAU_USER);
  if (ptr == (void *)NULL) {
    return false;
  }

#if defined(USE_DELETEDCHAINFLAG) && defined(USE_DELETED_CHAIN)
  const ObjectNode *obj = buffer_to_node(ptr);
  return AtomicAdjust::get(obj->_flag) == DCF_alive;
#else
  return true;
#endif  // USE_DELETEDCHAINFLAG
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedBufferChain::get_buffer_size
//       Access: Public
//  Description: Returns the size of the buffer that is actually
//               returned at each request.
////////////////////////////////////////////////////////////////////
INLINE size_t DeletedBufferChain::
get_buffer_size() const {
  return _buffer_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedBufferChain::node_to_buffer
//       Access: Private, Static
//  Description: Casts an ObjectNode* to a void* buffer.
////////////////////////////////////////////////////////////////////
INLINE void *DeletedBufferChain::
node_to_buffer(DeletedBufferChain::ObjectNode *node) {
#if defined(USE_DELETEDCHAINFLAG) && defined(USE_DELETED_CHAIN)
  // In development mode, we increment the pointer so that the
  // returned data does not overlap our _flag member.
  return (void *)(((char *)node) + flag_reserved_bytes);
#else
  return (void *)node;
#endif  // NDEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: DeletedBufferChain::buffer_to_node
//       Access: Private, Static
//  Description: Casts a void* buffer to an ObjectNode* .
////////////////////////////////////////////////////////////////////
INLINE DeletedBufferChain::ObjectNode *DeletedBufferChain::
buffer_to_node(void *ptr) {
#if defined(USE_DELETEDCHAINFLAG) && defined(USE_DELETED_CHAIN)
  // In development mode, we decrement the pointer to undo the
  // increment we did above.
  return (ObjectNode *)(((char *)ptr) - flag_reserved_bytes);
#else
  return (ObjectNode *)ptr;
#endif  // NDEBUG
}
