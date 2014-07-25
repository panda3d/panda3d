// Filename: glIndexBufferContext_src.cxx
// Created by:  drose (17Mar05)
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

TypeHandle CLP(IndexBufferContext)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GLIndexBufferContext::evict_lru
//       Access: Public, Virtual
//  Description: Evicts the page from the LRU.  Called internally when
//               the LRU determines that it is full.  May also be
//               called externally when necessary to explicitly evict
//               the page.
//
//               It is legal for this method to either evict the page
//               as requested, do nothing (in which case the eviction
//               will be requested again at the next epoch), or
//               requeue itself on the tail of the queue (in which
//               case the eviction will be requested again much
//               later).
////////////////////////////////////////////////////////////////////
void CLP(IndexBufferContext)::
evict_lru() {
  dequeue_lru();

  // Make sure the buffer is unbound before we delete it.
  if (_glgsg->_current_ibuffer_index == _index) {
    if (GLCAT.is_debug() && gl_debug_buffers) {
      GLCAT.debug()
        << "unbinding index buffer\n";
    }
    _glgsg->_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    _glgsg->_current_ibuffer_index = 0;
  }

  // Free the buffer.
  _glgsg->_glDeleteBuffers(1, &_index);

  // We still need a valid index number, though, in case we want to
  // re-load the buffer later.
  _glgsg->_glGenBuffers(1, &_index);

  update_data_size_bytes(0);
  mark_unloaded();
}
