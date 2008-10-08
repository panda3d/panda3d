// Filename: vertexDataBuffer.cxx
// Created by:  drose (14May07)
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

#include "vertexDataBuffer.h"
#include "pStatTimer.h"

TypeHandle VertexDataBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
operator = (const VertexDataBuffer &copy) {
  LightMutexHolder holder(_lock);
  LightMutexHolder holder2(copy._lock);

  if (_resident_data != (unsigned char *)NULL) {
    nassertv(_size != 0);
    get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
    PANDA_FREE_ARRAY(_resident_data);
    _resident_data = NULL;
  }
  if (copy._resident_data != (unsigned char *)NULL) {
    nassertv(copy._size != 0);
    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)copy._size);
    _resident_data = (unsigned char *)PANDA_MALLOC_ARRAY(copy._size);
    memcpy(_resident_data, copy._resident_data, copy._size);
  }
  _size = copy._size;
  _block = copy._block;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::do_clean_realloc
//       Access: Private
//  Description: Changes the size of the buffer, preserving its data
//               (except for any data beyond the new end of the
//               buffer, if the buffer is being reduced).  If the
//               buffer is expanded, the new data is uninitialized.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
do_clean_realloc(size_t size) {
  if (size != _size) {
    if (size == 0) {
      do_unclean_realloc(size);
      return;
    }      

    // Page in if we're currently paged out.
    if (_size != 0 && _resident_data == (unsigned char *)NULL) {
      do_page_in();
    }
    
    get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)size - (int)_size);
    if (_size == 0) {
      nassertv(_resident_data == (unsigned char *)NULL);
      _resident_data = (unsigned char *)PANDA_MALLOC_ARRAY(size);
    } else {
      nassertv(_resident_data != (unsigned char *)NULL);
      _resident_data = (unsigned char *)PANDA_REALLOC_ARRAY(_resident_data, size);
    }
    nassertv(_resident_data != (unsigned char *)NULL);
    _size = size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::do_unclean_realloc
//       Access: Private
//  Description: Changes the size of the buffer, without regard to
//               preserving its data.  The buffer may contain random
//               data after this call.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
do_unclean_realloc(size_t size) {
  if (size != _size || _resident_data == (unsigned char *)NULL) {
    // If we're paged out, discard the page.
    _block = NULL;
        
    if (_resident_data != (unsigned char *)NULL) {
      nassertv(_size != 0);

      get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
      PANDA_FREE_ARRAY(_resident_data);
      _resident_data = NULL;
      _size = 0;
    }

    if (size != 0) {
      get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)size);
      nassertv(_resident_data == (unsigned char *)NULL);
      _resident_data = (unsigned char *)PANDA_MALLOC_ARRAY(size);
    }

    _size = size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::do_page_out
//       Access: Private
//  Description: Moves the buffer out of independent memory and puts
//               it on a page in the indicated book.  The buffer may
//               still be directly accessible as long as its page
//               remains resident.  Any subsequent attempt to rewrite
//               the buffer will implicitly move it off of the page
//               and back into independent memory.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
do_page_out(VertexDataBook &book) {
  if (_block != (VertexDataBlock *)NULL || _size == 0) {
    // We're already paged out.
    return;
  }
  nassertv(_resident_data != (unsigned char *)NULL);

  _block = book.alloc(_size);
  nassertv(_block != (VertexDataBlock *)NULL);
  unsigned char *pointer = _block->get_pointer(true);
  nassertv(pointer != (unsigned char *)NULL);
  memcpy(pointer, _resident_data, _size);

  get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
  PANDA_FREE_ARRAY(_resident_data);
  _resident_data = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::do_page_in
//       Access: Private
//  Description: Moves the buffer off of its current page and into
//               independent memory.  If the page is not already
//               resident, it is forced resident first.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
do_page_in() {
  if (_resident_data != (unsigned char *)NULL || _size == 0) {
    // We're already paged in.
    return;
  }

  nassertv(_block != (VertexDataBlock *)NULL);

  get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)_size);
  _resident_data = (unsigned char *)PANDA_MALLOC_ARRAY(_size);
  nassertv(_resident_data != (unsigned char *)NULL);
  
  memcpy(_resident_data, _block->get_pointer(true), _size);
}
