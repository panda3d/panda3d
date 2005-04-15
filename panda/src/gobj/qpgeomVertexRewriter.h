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
//               object, modifying rows as it goes.
//
//               Although it doesn't provide any real performance
//               benefit over using a separate reader and writer on
//               the same data, it should probably be used in
//               preference to a separate reader and writer, because
//               it makes an effort to manage the reference counts
//               properly between the reader and the writer to avoid
//               accidentally dereferencing either array while
//               recopying.
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
  INLINE qpGeomVertexRewriter(qpGeomVertexArrayData *array_data);
  INLINE qpGeomVertexRewriter(qpGeomVertexArrayData *array_data, 
                              int column);
  INLINE ~qpGeomVertexRewriter();

  INLINE qpGeomVertexData *get_vertex_data() const;
  INLINE qpGeomVertexArrayData *get_array_data() const;

  INLINE bool set_column(int column);
  INLINE bool set_column(const string &name);
  INLINE bool set_column(const InternalName *name);
  INLINE bool set_column(int array, const qpGeomVertexColumn *column);

  INLINE bool has_column() const;
  INLINE int get_array() const;
  INLINE const qpGeomVertexColumn *get_column() const;

  INLINE void set_row(int row);

  INLINE int get_start_row() const;
  INLINE bool is_at_end() const;
};

#include "qpgeomVertexRewriter.I"

#endif
