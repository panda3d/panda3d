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
#include "pStatTimer.h"

TypeHandle VertexDataBuffer::_type_handle;

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
      // If we're going to size 0, we don't necessarily need to page
      // in first.  But if we're paged out, discard the page.
      _block = NULL;
        
      if (_resident_data != (unsigned char *)NULL) {
        free(_resident_data);
        _resident_data = NULL;
        get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
      }
      _block = NULL;
      
    } else {
      // Page in if we're currently paged out.
      if (_block != (VertexDataBlock *)NULL || 
          _resident_data == (unsigned char *)NULL) {
        do_page_in();
      }
      
      if (_resident_data == (unsigned char *)NULL) {
        _resident_data = (unsigned char *)malloc(size);
        get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)size);
      } else {
        _resident_data = (unsigned char *)::realloc(_resident_data, size);
        get_class_type().inc_memory_usage(TypeHandle::MC_array, (int)size - (int)_size);
      }
      nassertv(_resident_data != (unsigned char *)NULL);
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

  free(_resident_data);
  _resident_data = NULL;
  get_class_type().dec_memory_usage(TypeHandle::MC_array, (int)_size);
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
  if (_block == (VertexDataBlock *)NULL) {
    // We're already paged in.
    return;
  }

  nassertv(_resident_data == (unsigned char *)NULL);

  _resident_data = (unsigned char *)malloc(_size);
  nassertv(_resident_data != (unsigned char *)NULL);
  get_class_type().inc_memory_usage(TypeHandle::MC_array, _size);

  memcpy(_resident_data, _block->get_pointer(true), _size);
  _block = NULL;
}
