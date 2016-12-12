/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxVertexBufferContext9.h
 * @author drose
 * @date 2005-03-18
 */

#ifndef DXVERTEXBUFFERCONTEXT9_H
#define DXVERTEXBUFFERCONTEXT9_H

#include "pandabase.h"
#include "dxgsg9base.h"
#include "vertexBufferContext.h"
#include "deletedChain.h"

class DXGraphicsStateGuardian9;

/**
 * Caches a GeomVertexArrayData in the DirectX device as a vertex buffer.
 */
class EXPCL_PANDADX DXVertexBufferContext9 : public VertexBufferContext {
public:
  DXVertexBufferContext9(DXGraphicsStateGuardian9 *dxgsg,
                           PreparedGraphicsObjects *pgo,
                           GeomVertexArrayData *data);
  ALLOC_DELETED_CHAIN(DXVertexBufferContext9);

  virtual void evict_lru();

  LPDIRECT3DVERTEXBUFFER9 _vbuffer;
  int _fvf;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexBufferContext::init_type();
    register_type(_type_handle, "DXVertexBufferContext9",
                  VertexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxVertexBufferContext9.I"

#endif
