// Filename: glVertexBufferContext_src.h
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

#include "pandabase.h"
#include "vertexBufferContext.h"

////////////////////////////////////////////////////////////////////
//       Class : GLVertexBufferContext
// Description : Caches a GeomVertexArrayData on the GL as a buffer
//               object.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(VertexBufferContext) : public VertexBufferContext {
public:
  INLINE CLP(VertexBufferContext)(qpGeomVertexArrayData *data);

  // This is the GL "name" of the data object.
  GLuint _index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexBufferContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "VertexBufferContext",
                  VertexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glVertexBufferContext_src.I"

