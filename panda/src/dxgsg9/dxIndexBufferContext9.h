/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxIndexBufferContext9.h
 * @author drose
 * @date 2005-03-18
 */

#ifndef DXINDEXBUFFERCONTEXT9_H
#define DXINDEXBUFFERCONTEXT9_H

#include "pandabase.h"
#include "dxgsg9base.h"
#include "indexBufferContext.h"

/**
 * Caches a GeomPrimitive in the DirectX device as an index buffer.
 */
class EXPCL_PANDADX DXIndexBufferContext9 : public IndexBufferContext {
public:
  DXIndexBufferContext9(PreparedGraphicsObjects *pgo, GeomPrimitive *data);
  virtual ~DXIndexBufferContext9();

  virtual void evict_lru();

  void free_ibuffer();
  void allocate_ibuffer(DXScreenData &scrn, const GeomPrimitivePipelineReader *reader);
  void create_ibuffer(DXScreenData &scrn, const GeomPrimitivePipelineReader *reader);
  bool upload_data(const GeomPrimitivePipelineReader *reader, bool force);

  IDirect3DIndexBuffer9 *_ibuffer;
  int _managed;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IndexBufferContext::init_type();
    register_type(_type_handle, "DXIndexBufferContext9",
                  IndexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxIndexBufferContext9.I"

#endif
