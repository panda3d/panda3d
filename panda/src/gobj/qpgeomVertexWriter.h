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
class EXPCL_PANDA qpGeomVertexWriter {
PUBLISHED:
  INLINE qpGeomVertexWriter(qpGeomVertexData *vertex_data);
  INLINE qpGeomVertexWriter(qpGeomVertexData *vertex_data,
                            const string &name);
  INLINE qpGeomVertexWriter(qpGeomVertexData *vertex_data,
                            const InternalName *name);
  INLINE ~qpGeomVertexWriter();

  INLINE qpGeomVertexData *get_vertex_data() const;

  INLINE bool set_column(int column);
  INLINE bool set_column(const string &name);
  INLINE bool set_column(const InternalName *name);
  bool set_column(int array, const qpGeomVertexColumn *column);

  INLINE bool has_column() const;
  INLINE int get_array() const;
  INLINE const qpGeomVertexColumn *get_column() const;

  INLINE void set_vertex(int vertex);

  INLINE int get_start_vertex() const;
  INLINE int get_write_vertex() const;
  INLINE int get_num_vertices() const;
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

  INLINE void set_pointer(int vertex);
  INLINE unsigned char *inc_pointer();
  INLINE unsigned char *inc_add_pointer();
  Writer *make_writer() const;

  PT(qpGeomVertexData) _vertex_data;
  int _array;
  const qpGeomVertexColumn *_column;
  int _stride;

  unsigned char *_pointer;

  int _start_vertex;
  int _write_vertex;
  int _num_vertices;

  Writer *_writer;

  // This nested class provides the implementation for unpacking data
  // in a very general way, but also provides the hooks for
  // implementing the common, very direct code paths (for instance,
  // 3-component float32 to LVecBase3f) as quickly as possible.
  class Writer {
  public:
    virtual ~Writer();
    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);
    
    virtual void set_data1i(unsigned char *pointer, int a);
    virtual void set_data2i(unsigned char *pointer, int a, int b);
    virtual void set_data3i(unsigned char *pointer, int a, int b, int c);
    virtual void set_data4i(unsigned char *pointer, int a, int b, int c, int d);

    INLINE unsigned int maybe_scale_color(float data);
    INLINE void maybe_scale_color(const LVecBase2f &data);
    INLINE void maybe_scale_color(const LVecBase3f &data);
    INLINE void maybe_scale_color(const LVecBase4f &data);

    const qpGeomVertexColumn *_column;
    unsigned int _a, _b, _c, _d;
  };

  // This is a specialization on the generic Writer that handles
  // points, which are special because the fourth component, if
  // present in the data but not specified by the caller, is
  // implicitly 1.0; and if it is not present in the data but is
  // specified, we have to divide by it.
  class Writer_point : public Writer {
  public:
    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &data);
  };

  // This is similar to Writer_point, in that the fourth component
  // (alpha) is implicitly 1.0 if unspecified, but we never divide by
  // alpha.
  class Writer_color : public Writer {
  public:
    virtual void set_data1f(unsigned char *pointer, float data);
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &data);
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &data);
  };


  // These are the specializations on the generic Writer that handle
  // the direct code paths.

  class Writer_float32_3 : public Writer {
  public:
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &value);
  };

  class Writer_point_float32_2 : public Writer_point {
  public:
    virtual void set_data2f(unsigned char *pointer, const LVecBase2f &value);
  };

  class Writer_point_float32_3 : public Writer_point {
  public:
    virtual void set_data3f(unsigned char *pointer, const LVecBase3f &value);
  };

  class Writer_point_float32_4 : public Writer_point {
  public:
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Writer_argb_packed : public Writer_color {
  public:
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Writer_rgba_uint8_4 : public Writer_color {
  public:
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Writer_rgba_float32_4 : public Writer_color {
  public:
    virtual void set_data4f(unsigned char *pointer, const LVecBase4f &value);
  };

  class Writer_uint16_1 : public Writer {
  public:
    virtual void set_data1i(unsigned char *pointer, int value);
  };
};

#include "qpgeomVertexWriter.I"

#endif
