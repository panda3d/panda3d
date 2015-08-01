// Filename: glUniformBufferContext_src.h
// Created by:  rdb (29Jul15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef OPENGLES

#include "pandabase.h"
#include "bufferContext.h"
#include "deletedChain.h"
#include "shader.h"

////////////////////////////////////////////////////////////////////
//       Class : GLUniformBufferContext
// Description : Caches a GeomPrimitive on the GL as a buffer
//               object.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(UniformBufferContext) : public BufferContext, public ReferenceCount {
public:
  CLP(UniformBufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                            CPT(GeomVertexArrayFormat) layout);
  virtual ~CLP(UniformBufferContext)();

  ALLOC_DELETED_CHAIN(CLP(UniformBufferContext));

  CLP(GraphicsStateGuardian) *_glgsg;

  // This is the GL "name" of the data object.
  GLuint _index;

  void update_data(const ShaderAttrib *attrib);

private:
  CPT(GeomVertexArrayFormat) _layout;
  //Shader::ShaderMatSpecs _mat_spec;
  int _mat_deps;
  int _frame;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BufferContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "UniformBufferContext",
                  BufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glUniformBufferContext_src.I"

#endif  // !OPENGLES
