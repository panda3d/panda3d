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
#include "qpgeomVertexDataType.h"
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
//               end of existing data they will quietly add new data.
//
//               Like GeomVertexReader, the writer is particularly
//               optimized for writing a column of data values for a
//               series of vertices, without changing data types
//               between each number.  Although you can use one
//               GeomVertexWriter to write one complete row at a time,
//               by calling set_data_type() repeatedly for each
//               vertex, it is faster to use a different
//               GeomVertexWriter for each data type.
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

  INLINE bool set_data_type(int data_type);
  INLINE bool set_data_type(const string &name);
  INLINE bool set_data_type(const InternalName *name);
  bool set_data_type(int array, const qpGeomVertexDataType *data_type);

  INLINE bool has_data_type() const;
  INLINE int get_array() const;
  INLINE const qpGeomVertexDataType *get_data_type() const;

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

  INLINE void add_data1f(float data);
  INLINE void add_data2f(float x, float y);
  INLINE void add_data2f(const LVecBase2f &data);
  INLINE void add_data3f(float x, float y, float z);
  INLINE void add_data3f(const LVecBase3f &data);
  INLINE void add_data4f(float x, float y, float z, float w);
  INLINE void add_data4f(const LVecBase4f &data);

  INLINE void add_data1i(int data);

private:
  class Writer;

  INLINE void set_pointer(int vertex);
  INLINE unsigned char *inc_pointer();
  INLINE unsigned char *inc_add_pointer();
  Writer *make_writer() const;

  PT(qpGeomVertexData) _vertex_data;
  int _array;
  const qpGeomVertexDataType *_data_type;
  int _stride;

  PTA_uchar _data;
  unsigned char *_pointer;

  int _start_vertex;
  int _write_vertex;
  int _num_vertices;

  Writer *_writer;

  // This union is handy for packing an NT_packed_8888 value.
  typedef union {
    unsigned char _b[4];
    PN_uint32 _i;
  } packed_8888;

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
    
    virtual void set_data1i(unsigned char *pointer, int data);

    INLINE unsigned int maybe_scale_color(float data);
    INLINE void maybe_scale_color(const LVecBase2f &data);
    INLINE void maybe_scale_color(const LVecBase3f &data);
    INLINE void maybe_scale_color(const LVecBase4f &data);

    const qpGeomVertexDataType *_data_type;
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

  class Writer_argb_packed_8888 : public Writer_color {
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
