// Filename: glSamplerContext_src.h
// Created by:  rdb (11Dec14)
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
#include "samplerContext.h"
#include "deletedChain.h"

class CLP(GraphicsStateGuardian);

////////////////////////////////////////////////////////////////////
//       Class : GLSamplerContext
// Description : This class represents a sampler object, which
//               contains a set of sampler parameters used when
//               sampling a texture.
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(SamplerContext) : public SamplerContext {
public:
  INLINE CLP(SamplerContext)(CLP(GraphicsStateGuardian) *glgsg,
                             const SamplerState &sampler);
  ALLOC_DELETED_CHAIN(CLP(SamplerContext));

  virtual ~CLP(SamplerContext)();
  virtual void evict_lru();
  void reset_data();

  // This is the GL "name" of the sampler object.
  GLuint _index;

  CLP(GraphicsStateGuardian) *_glgsg;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SamplerContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "SamplerContext",
                  SamplerContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // OPENGLES
