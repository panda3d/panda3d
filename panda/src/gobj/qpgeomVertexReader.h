// Filename: qpgeomVertexReader.h
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

#ifndef qpGEOMVERTEXREADER_H
#define qpGEOMVERTEXREADER_H

#include "pandabase.h"
#include "qpgeomVertexData.h"
#include "qpgeomVertexColumn.h"
#include "internalName.h"
#include "luse.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexReader
// Description : This object provides a high-level interface for
//               quickly reading a sequence of numeric values from a
//               vertex table. 
//
//               It is particularly optimized for reading a single
//               column of data values for a series of vertices,
//               without changing columns between each number.
//               Although you can also use one GeomVertexReader to
//               read across the columns if it is convenient, by
//               calling set_column() repeatedly at each vertex, it is
//               faster to read down the columns, and to use a
//               different GeomVertexReader for each column.
//
//               Note that a GeomVertexReader does not keep a
//               reference count to the actual vertex data buffer (it
//               grabs the current data buffer from the GeomVertexData
//               whenever set_column() is called).  This means that it
//               is important not to keep a GeomVertexReader object
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
class EXPCL_PANDA qpGeomVertexReader {
PUBLISHED:
  INLINE qpGeomVertexReader(const qpGeomVertexData *vertex_data);
  INLINE qpGeomVertexReader(const qpGeomVertexData *vertex_data,
                            const string &name);
  INLINE qpGeomVertexReader(const qpGeomVertexData *vertex_data,
                            const InternalName *name);
  INLINE ~qpGeomVertexReader();

  INLINE const qpGeomVertexData *get_vertex_data() const;

  INLINE bool set_column(int column);
  INLINE bool set_column(const string &name);
  INLINE bool set_column(const InternalName *name);
  bool set_column(int array, const qpGeomVertexColumn *column);

  INLINE bool has_column() const;
  INLINE int get_array() const;
  INLINE const qpGeomVertexColumn *get_column() const;

  INLINE void set_vertex(int vertex);

  INLINE int get_start_vertex() const;
  INLINE int get_read_vertex() const;
  INLINE int get_num_vertices() const;
  INLINE bool is_at_end() const;

  INLINE float get_data1f();
  INLINE const LVecBase2f &get_data2f();
  INLINE const LVecBase3f &get_data3f();
  INLINE const LVecBase4f &get_data4f();

  INLINE int get_data1i();
  INLINE const int *get_data2i();
  INLINE const int *get_data3i();
  INLINE const int *get_data4i();

private:
  class Reader;

  INLINE void set_pointer(int vertex);
  INLINE const unsigned char *inc_pointer();
  Reader *make_reader() const;

  CPT(qpGeomVertexData) _vertex_data;
  int _array;
  const qpGeomVertexColumn *_column;
  int _stride;

  const unsigned char *_pointer;

  int _start_vertex;
  int _read_vertex;
  int _num_vertices;

  Reader *_reader;

  // This nested class provides the implementation for unpacking data
  // in a very general way, but also provides the hooks for
  // implementing the common, very direct code paths (for instance,
  // 3-component float32 to LVecBase3f) as quickly as possible.
  class Reader {
  public:
    virtual ~Reader();
    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
    virtual int get_data1i(const unsigned char *pointer);
    virtual const int *get_data2i(const unsigned char *pointer);
    virtual const int *get_data3i(const unsigned char *pointer);
    virtual const int *get_data4i(const unsigned char *pointer);

    INLINE float maybe_scale_color(unsigned int value);
    INLINE void maybe_scale_color(unsigned int a, unsigned int b);
    INLINE void maybe_scale_color(unsigned int a, unsigned int b,
                                  unsigned int c);
    INLINE void maybe_scale_color(unsigned int a, unsigned int b,
                                  unsigned int c, unsigned int d);

    const qpGeomVertexColumn *_column;
    LVecBase2f _v2;
    LVecBase3f _v3;
    LVecBase4f _v4;
    int _i[4];
  };

  // This is a specialization on the generic Reader that handles
  // points, which are special because the fourth component, if not
  // present in the data, is implicitly 1.0; and if it is present,
  // than any three-component or smaller return is implicitly divided
  // by the fourth component.
  class Reader_point : public Reader {
  public:
    virtual float get_data1f(const unsigned char *pointer);
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  // This is similar to Reader_point, in that the fourth component is
  // implicitly 1.0 if it is not present in the data, but we never
  // divide by alpha.
  class Reader_color : public Reader {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };


  // These are the specializations on the generic Reader that handle
  // the direct code paths.

  class Reader_float32_3 : public Reader {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
  };

  class Reader_point_float32_2 : public Reader_point {
  public:
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
  };

  class Reader_point_float32_3 : public Reader_point {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
  };

  class Reader_point_float32_4 : public Reader_point {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Reader_nativefloat_3 : public Reader {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
  };

  class Reader_point_nativefloat_2 : public Reader_point {
  public:
    virtual const LVecBase2f &get_data2f(const unsigned char *pointer);
  };

  class Reader_point_nativefloat_3 : public Reader_point {
  public:
    virtual const LVecBase3f &get_data3f(const unsigned char *pointer);
  };

  class Reader_point_nativefloat_4 : public Reader_point {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Reader_argb_packed : public Reader_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Reader_rgba_uint8_4 : public Reader_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Reader_rgba_float32_4 : public Reader_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Reader_rgba_nativefloat_4 : public Reader_color {
  public:
    virtual const LVecBase4f &get_data4f(const unsigned char *pointer);
  };

  class Reader_uint16_1 : public Reader {
  public:
    virtual int get_data1i(const unsigned char *pointer);
  };
};

#include "qpgeomVertexReader.I"

#endif
