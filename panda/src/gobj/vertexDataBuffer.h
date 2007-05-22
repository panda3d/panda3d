// Filename: vertexDataBuffer.h
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

#ifndef VERTEXDATABUFFER_H
#define VERTEXDATABUFFER_H

#include "pandabase.h"
#include "vertexDataBook.h"
#include "pointerTo.h"
#include "virtualFile.h"
#include "pStatCollector.h"

////////////////////////////////////////////////////////////////////
//       Class : VertexDataBuffer
// Description : A block of bytes that stores the actual raw vertex
//               data referenced by a GeomVertexArrayData object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexDataBuffer {
public:
  INLINE VertexDataBuffer();
  INLINE VertexDataBuffer(size_t size);
  INLINE VertexDataBuffer(const VertexDataBuffer &copy);
  INLINE void operator = (const VertexDataBuffer &copy);
  INLINE ~VertexDataBuffer();

  INLINE const unsigned char *get_read_pointer() const;
  INLINE unsigned char *get_write_pointer();

  INLINE size_t get_size() const;
  void clean_realloc(size_t size);
  INLINE void unclean_realloc(size_t size);
  INLINE void clear();

  INLINE void swap(VertexDataBuffer &other);

  void page_out(VertexDataBook &book);
  void page_in();

  INLINE void set_file(VirtualFile *source_file, streampos source_pos);

private:
  unsigned char *_resident_data;
  size_t _size;
  PT(VertexDataBlock) _block;
  PT(VirtualFile) _source_file;
  streampos _source_pos;

  static PStatCollector _vdata_reread_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "VertexDataBuffer");
  }

private:
  static TypeHandle _type_handle;
};

#include "vertexDataBuffer.I"

#endif
