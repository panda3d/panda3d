// Filename: qpgeomVertexWriter.h
// Created by:  drose (25Mar05)
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

#ifndef qpGEOMVERTEXWRITER_H
#define qpGEOMVERTEXWRITER_H

#include "pandabase.h"
#include "qpgeomVertexData.h"
#include "qpgeomVertexColumn.h"
#include "internalName.h"
#include "luse.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexWriter
// Description : This object provides a high-level interface for
//               quickly writing a sequence of numeric values from a
//               vertex table. 
//
//               This object can be used both to replace existing
//               vertices in the table, or to extend the table with
//               new vertices.  The set_data*() family of methods can
//               only be used to replace existing data; it is an error
//               to allow these to run past the end of the data.  The
//               add_data*() family of methods, on the other hand, can
//               be used to replace existing data or add new data; if
//               you call set_vertex() into the middle of existing
//               data the add_data*() methods will behave like the
//               corresponding set_data*(), but if they run past the
//               end of existing data they will quietly add new
//               vertices.
//
//               Like GeomVertexReader, the writer is particularly
//               optimized for writing a single column of data values
//               for a series of vertices, without changing columns
//               between each number.  Although you can also use one
//               GeomVertexWriter to write across the columns if it is
//               convenient, by calling set_column() repeatedly at
//               each vertex, it is faster to write down the columns,
//               and to use a different GeomVertexWriter for each
//               column.
//
//               Note that, like a GeomVertexReader, a
//               GeomVertexWriter does not keep a reference count to
//               the actual vertex data buffer.  This means that it is
//               important not to keep a GeomVertexWriter object
//               around over a long period of time in which the data
//               buffer is likely to be deallocated; it is intended
//               for making a quick pass over the data in one session.
//
//               It also means that you should create any
//               GeomVertexWriters *before* creating GeomVertexReaders
//               on the same data, since the writer itself might cause
//               the vertex buffer to be deallocated.  Better yet, use
//               a GeomVertexRewriter if you are going to create both
//               of them anyway.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexWriter : public qpGeomEnums {
PUBLISHED:
  INLINE qpGeomVertexWriter(qpGeomVertexData *vertex_data);
  INLINE qpGeomVertexWriter(qpGeomVertexData *vertex_data,
                            const string &name);
  INLINE qpGeomVertexWriter(qpGeomVertexData *vertex_data,
                            const InternalName *name);
  INLINE qpGeomVertexWriter(qpGeomVertexArrayData *array_data);
  INLINE qpGeomVertexWriter(qpGeomVertexArrayData *array_data, 
                            int column);
  INLINE ~qpGeomVertexWriter();

  INLINE qpGeomVertexData *get_vertex_data() const;
  INLINE qpGeomVertexArrayData *get_array_data() const;

  INLINE bool set_column(int column);
  INLINE bool set_column(const string &name);
  INLINE bool set_column(const InternalName *name);
  bool set_column(int array, const qpGeomVertexColumn *column);

  INLINE bool has_column() const;
  INLINE int get_array() const;
  INLINE const qpGeomVertexColumn *get_column() const;

  INLINE void set_vertex(int vertex);

  INLINE int get_start_vertex() const;
  INLINE bool is_at_end() const;

  INLINE void set_data1f(float data);
  INLINE void set_data2f(float x, float y);
  INLINE void set_data2f(const LVecBase2f &data);
  INLINE void set_data3f(float x, float y, float z);
  INLINE void set_data3f(const LVecBase3f &data);
  INLINE void set_data4f(float x, float y, float z, float w);
  INLINE void set_data4f(const LVecBase4f &data);

  INLINE void set_data1i(int data);
  INLINE void set_data2i(int a, int b);
  INLINE void set_data2i(const int data[2]);
  INLINE void set_data3i(int a, int b, int c);
  INLINE void set_data3i(const int data[3]);
  INLINE void set_data4i(int a, int b, int c, int d);
  INLINE void set_data4i(const int data[4]);

  INLINE void add_data1f(float data);
  INLINE void add_data2f(float x, float y);
  INLINE void add_data2f(const LVecBase2f &data);
  INLINE void add_data3f(float x, float y, float z);
  INLINE void add_data3f(const LVecBase3f &data);
  INLINE void add_data4f(float x, float y, float z, float w);
  INLINE void add_data4f(const LVecBase4f &data);

  INLINE void add_data1i(int data);
  INLINE void add_data2i(int a, int b);
  INLINE void add_data2i(const int data[2]);
  INLINE void add_data3i(int a, int b, int c);
  INLINE void add_data3i(const int data[3]);
  INLINE void add_data4i(int a, int b, int c, int d);
  INLINE void add_data4i(const int data[4]);

private:
  class Writer;

  void initialize();

  INLINE void set_pointer(int vertex);
  INLINE unsigned char *inc_pointer();
  INLINE unsigned char *inc_add_pointer();

  // It is important that we only store *one* of the following two
  // pointers.  If we are storing a GeomVertexData/array index, we
  // must not keep a pointer to the particular ArrayData we are
  // working on (if we do, it may result in an extra copy of the data
  // due to holding the reference count).
  PT(qpGeomVertexData) _vertex_data;
  int _array;
  PT(qpGeomVertexArrayData) _array_data;

  qpGeomVertexColumn::Packer *_packer;
  int _stride;

  unsigned char *_pointer;
  unsigned char *_pointer_end;

  int _start_vertex;
  int _write_vertex;

#ifndef NDEBUG
  // This is defined just for the benefit of having something non-NULL
  // to return from a nassertr() call.
  static unsigned char empty_buffer[100];
#endif
};

#include "qpgeomVertexWriter.I"

#endif
