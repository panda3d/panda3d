// Filename: qpgeomVertexRewriter.h
// Created by:  drose (28Mar05)
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

#ifndef qpGEOMVERTEXREWRITER_H
#define qpGEOMVERTEXREWRITER_H

#include "pandabase.h"
#include "qpgeomVertexReader.h"
#include "qpgeomVertexWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexRewriter
// Description : This object provides the functionality of both a
//               GeomVertexReader and a GeomVertexWriter, combined
//               together into one convenient package.  It is designed
//               for making a single pass over a GeomVertexData
//               object, modifying vertices as it goes.
//
//               Although it doesn't provide any real performance
//               benefit over using a separate reader and writer
//               object, it should probably be used in preference to
//               separate objects, because it makes an effort to
//               manage the reference counts properly between the
//               reader and the writer to avoid accidentally
//               dereferencing either array while recopying.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexRewriter : public qpGeomVertexWriter, public qpGeomVertexReader {
PUBLISHED:
  INLINE qpGeomVertexRewriter(qpGeomVertexData *vertex_data);
  INLINE qpGeomVertexRewriter(qpGeomVertexData *vertex_data,
                              const string &name);
  INLINE qpGeomVertexRewriter(qpGeomVertexData *vertex_data,
                              const InternalName *name);
  INLINE ~qpGeomVertexRewriter();

  INLINE qpGeomVertexData *get_vertex_data() const;

  INLINE bool set_data_type(int data_type);
  INLINE bool set_data_type(const string &name);
  INLINE bool set_data_type(const InternalName *name);
  INLINE bool set_data_type(int array, const qpGeomVertexDataType *data_type);

  INLINE bool has_data_type() const;
  INLINE int get_array() const;
  INLINE const qpGeomVertexDataType *get_data_type() const;

  INLINE void set_vertex(int vertex);

  INLINE int get_start_vertex() const;
  INLINE int get_num_vertices() const;
  INLINE bool is_at_end() const;
};

#include "qpgeomVertexRewriter.I"

#endif
