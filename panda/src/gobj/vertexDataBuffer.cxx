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

PStatCollector VertexDataBuffer::_vdata_reread_pcollector("*:Vertex Data:Reread");
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
    _source_file.clear();

    if (size == 0) {
      // If we're going to size 0, we don't necessarily need to page
      // in first.  But if we're paged out, discard the page.
      _block = NULL;
        
      if (_resident_data != (unsigned char *)NULL) {
        free(_resident_data);
        _resident_data = NULL;
        get_class_type().dec_memory_usage(TypeHandle::MC_array, _size);
      }
      _block = NULL;
      
    } else {
      // Page in if we're currently paged out.
      if (_block != (VertexDataBlock *)NULL || 
          _resident_data == (unsigned char *)NULL) {
        page_in();
      }
      
      if (_resident_data == (unsigned char *)NULL) {
        _resident_data = (unsigned char *)malloc(size);
        get_class_type().inc_memory_usage(TypeHandle::MC_array, size);
      } else {
        _resident_data = (unsigned char *)::realloc(_resident_data, size);
        get_class_type().dec_memory_usage(TypeHandle::MC_array, _size);
        get_class_type().inc_memory_usage(TypeHandle::MC_array, size);
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

  if (_source_file == (VirtualFile *)NULL) {
    // We only need to allocate a block if we don't have a source
    // file.
    _block = book.alloc(_size);
    nassertv(_block != (VertexDataBlock *)NULL);
    unsigned char *pointer = _block->get_pointer();
    nassertv(pointer != (unsigned char *)NULL);
    memcpy(pointer, _resident_data, _size);
  }

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
  if (_source_file != (VirtualFile *)NULL && _resident_data == (unsigned char *)NULL) {
    // Re-read the data from its original source.
    PStatTimer timer(_vdata_reread_pcollector);

    _resident_data = (unsigned char *)malloc(_size);
    nassertv(_resident_data != (unsigned char *)NULL);
    get_class_type().inc_memory_usage(TypeHandle::MC_array, _size);

    istream *in = _source_file->open_read_file(true);
    if (in == (istream *)NULL) {
      gobj_cat.error()
        << "Error reopening " << _source_file->get_filename()
        << " to reread vertex data.\n";
    } else {
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "rereading " << _size << " bytes from " 
          << _source_file->get_filename() << ", position " 
          << _source_pos << "\n";
      }

      in->seekg(_source_pos);

      in->read((char *)_resident_data, _size);
      if (in->fail() || in->eof()) {
        gobj_cat.error()
          << "Error rereading " << _size << " bytes from " 
          << _source_file->get_filename() << ", position " 
          << _source_pos << "\n";
      }

      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      vfs->close_read_file(in);
    }
    return;
  }

  if (_block == (VertexDataBlock *)NULL) {
    // We're already paged in.
    return;
  }

  nassertv(_resident_data == (unsigned char *)NULL);

  _resident_data = (unsigned char *)malloc(_size);
  nassertv(_resident_data != (unsigned char *)NULL);
  get_class_type().inc_memory_usage(TypeHandle::MC_array, _size);

  memcpy(_resident_data, _block->get_pointer(), _size);
  _block = NULL;
}
