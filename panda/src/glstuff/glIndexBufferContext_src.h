/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glIndexBufferContext_src.h
 * @author drose
 * @date 2005-03-17
 */

#include "pandabase.h"
#include "indexBufferContext.h"
#include "deletedChain.h"

/**
 * Caches a GeomPrimitive on the GL as a buffer object.
 */
class EXPCL_GL CLP(IndexBufferContext) : public IndexBufferContext {
public:
  INLINE CLP(IndexBufferContext)(CLP(GraphicsStateGuardian) *glgsg,
                                 PreparedGraphicsObjects *pgo,
                                 GeomPrimitive *data);
  ALLOC_DELETED_CHAIN(CLP(IndexBufferContext));

  virtual void evict_lru();

  CLP(GraphicsStateGuardian) *_glgsg;

  // This is the GL "name" of the data object.
  GLuint _index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IndexBufferContext::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "IndexBufferContext",
                  IndexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glIndexBufferContext_src.I"
