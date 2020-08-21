/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glBufferContext_src.h
 * @author rdb
 * @date 2016-12-12
 */

#include "pandabase.h"
#include "bufferContext.h"
#include "deletedChain.h"

/**
 * Caches a GeomPrimitive on the GL as a buffer object.
 */
class EXPCL_GL CLP(BufferContext) : public BufferContext, public AdaptiveLruPage {
public:
  INLINE CLP(BufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                            PreparedGraphicsObjects *pgo,
                            TypedWritableReferenceCount *object);
  ALLOC_DELETED_CHAIN(CLP(BufferContext));

  virtual void evict_lru();

  CLP(GraphicsStateGuardian) *_glgsg;

  // This is the GL "name" of the data object.
  GLuint _index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BufferContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "BufferContext",
                  BufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glBufferContext_src.I"
