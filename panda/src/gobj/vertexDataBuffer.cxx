// Filename: vertexDataBuffer.cxx
// Created by:  drose (14May07)
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

#include "vertexDataBuffer.h"

TypeHandle VertexDataBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::clean_realloc
//       Access: Public
//  Description: Changes the size of the buffer, preserving its data
//               (except for any data beyond the new end of the
//               buffer, if the buffer is being reduced).  If the
//               buffer is expanded, the new data is uninitialized.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
clean_realloc(size_t size) {
  if (size != _size) {
    if (size == 0) {
      // If we're going to size 0, we don't necessarily need to page
      // in first.
      if (_block != (VertexDataBlock *)NULL) {
        // We're currently paged out.  Discard the page.
        _block = NULL;
      } else {
        // We're currently paged in.  Decrement the global total.
        get_class_type().dec_memory_usage(TypeHandle::MC_array, _size);
      }
        
      if (_resident_data != (unsigned char *)NULL) {
        free(_resident_data);
        _resident_data = NULL;
      }
      _block = NULL;
      
    } else {
      // Page if if we're currently paged out.
      if (_block != (VertexDataBlock *)NULL) {
        page_in();
      }
      
      get_class_type().dec_memory_usage(TypeHandle::MC_array, _size);
      get_class_type().inc_memory_usage(TypeHandle::MC_array, size);
    
      if (_resident_data == (unsigned char *)NULL) {
        _resident_data = (unsigned char *)malloc(size);
      } else {
        _resident_data = (unsigned char *)::realloc(_resident_data, size);
      }
      nassertv(_resident_data != (unsigned char *)NULL);
    }

    _size = size;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::page_out
//       Access: Public
//  Description: Moves the buffer out of independent memory and puts
//               it on a page in the indicated book.  The buffer may
//               still be directly accessible as long as its page
//               remains resident.  Any subsequent attempt to rewrite
//               the buffer will implicitly move it off of the page
//               and back into independent memory.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
page_out(VertexDataBook &book) {
  if (_block != (VertexDataBlock *)NULL || _size == 0) {
    // We're already paged out.
    return;
  }
  nassertv(_resident_data != (unsigned char *)NULL);

  _block = book.alloc(_size);
  memcpy(_block->get_pointer(), _resident_data, _size);
  free(_resident_data);
  _resident_data = NULL;

  get_class_type().dec_memory_usage(TypeHandle::MC_array, _size);
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBuffer::page_in
//       Access: Public
//  Description: Moves the buffer off of its current page and into
//               independent memory.  If the page is not already
//               resident, it is forced resident first.
////////////////////////////////////////////////////////////////////
void VertexDataBuffer::
page_in() {
  if (_block == (VertexDataBlock *)NULL) {
    // We're already paged in.
    return;
  }

  nassertv(_resident_data == (unsigned char *)NULL);

  _resident_data = (unsigned char *)malloc(_size);
  nassertv(_resident_data != (unsigned char *)NULL);
  memcpy(_resident_data, _block->get_pointer(), _size);
  _block = NULL;
  
  get_class_type().inc_memory_usage(TypeHandle::MC_array, _size);
}
