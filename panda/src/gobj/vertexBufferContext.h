// Filename: vertexBufferContext.h
// Created by:  drose (17Mar05)
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

#ifndef VERTEXBUFFERCONTEXT_H
#define VERTEXBUFFERCONTEXT_H

#include "pandabase.h"

#include "savedContext.h"
#include "updateSeq.h"
#include "qpgeomVertexArrayData.h"

////////////////////////////////////////////////////////////////////
//       Class : VertexBufferContext
// Description : This is a special class object that holds all the
//               information returned by a particular GSG to indicate
//               the vertex data array's internal context identifier.
//
//               This allows the GSG to cache the vertex data array in
//               whatever way makes sense.  For instance, DirectX can
//               allocate a vertex buffer for the array.  OpenGL can
//               create a buffer object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA VertexBufferContext : public SavedContext {
public:
  INLINE VertexBufferContext(qpGeomVertexArrayData *data);

  INLINE qpGeomVertexArrayData *get_data() const;

  INLINE int get_data_size_bytes() const;
  INLINE bool changed_size() const;
  INLINE bool changed_usage_hint() const;
  INLINE bool was_modified() const;

  INLINE void mark_loaded();

private:
  // This cannot be a PT(qpGeomVertexArrayData), because the data and
  // the GSG both own their VertexBufferContexts!  That would create a
  // circular reference count.
  qpGeomVertexArrayData *_data;
  UpdateSeq _modified;
  int _data_size_bytes;
  qpGeomEnums::UsageHint _usage_hint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "VertexBufferContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
};

#include "vertexBufferContext.I"

#endif

