// Filename: qpgeomVertexIterator.h
// Created by:  drose (10Mar05)
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

#ifndef qpGEOMVERTEXITERATOR_H
#define qpGEOMVERTEXITERATOR_H

#include "pandabase.h"
#include "qpgeomVertexData.h"
#include "qpgeomVertexDataType.h"
#include "internalName.h"
#include "luse.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexIterator
// Description : This is used to read or write the vertices of a
//               GeomVertexData structure, one vertex and data type at
//               a time.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexIterator {
PUBLISHED:
  INLINE qpGeomVertexIterator(qpGeomVertexData *data);
  INLINE qpGeomVertexIterator(qpGeomVertexData *data,
                              const string &name);
  INLINE qpGeomVertexIterator(qpGeomVertexData *data,
                              const InternalName *name);
  INLINE qpGeomVertexIterator(const qpGeomVertexData *data);
  INLINE qpGeomVertexIterator(const qpGeomVertexData *data,
                              const string &name);
  INLINE qpGeomVertexIterator(const qpGeomVertexData *data,
                              const InternalName *name);

  INLINE const qpGeomVertexData *get_data() const;

  INLINE void set_data_type(int data_type);
  INLINE void set_data_type(const string &name);
  INLINE void set_data_type(const InternalName *name);
  INLINE void set_data_type(int array, const qpGeomVertexDataType *data_type);

  INLINE int get_array() const;
  INLINE const qpGeomVertexDataType *get_data_type() const;

  INLINE void set_vertex(int vertex);

  INLINE int get_start_vertex() const;
  INLINE int get_read_vertex() const;
  INLINE int get_write_vertex() const;

  INLINE void set_data1(float data);
  INLINE void set_data2(float x, float y);
  INLINE void set_data2(const LVecBase2f &data);
  INLINE void set_data3(float x, float y, float z);
  INLINE void set_data3(const LVecBase3f &data);
  INLINE void set_data4(float x, float y, float z, float w);
  INLINE void set_data4(const LVecBase4f &data);

  INLINE float get_data1();
  INLINE LVecBase2f get_data2();
  INLINE LVecBase3f get_data3();
  INLINE LVecBase4f get_data4();

private:
  PT(qpGeomVertexData) _data;
  bool _const_data;
  int _array;
  const qpGeomVertexDataType *_data_type;

  int _start_vertex;
  int _read_vertex;
  int _write_vertex;
};

#include "qpgeomVertexIterator.I"

#endif
