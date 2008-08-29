// Filename: dxIndexBufferContext8.h
// Created by:  drose (18Mar05)
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

#ifndef DXINDEXBUFFERCONTEXT8_H
#define DXINDEXBUFFERCONTEXT8_H

#include "pandabase.h"
#include "dxgsg8base.h"
#include "indexBufferContext.h"

////////////////////////////////////////////////////////////////////
//       Class : DXIndexBufferContext8
// Description : Caches a GeomPrimitive in the DirectX device as
//               an index buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXIndexBufferContext8 : public IndexBufferContext {
public:
  DXIndexBufferContext8(PreparedGraphicsObjects *pgo, GeomPrimitive *data);
  virtual ~DXIndexBufferContext8();

  virtual void evict_lru();

  void create_ibuffer(DXScreenData &scrn,
                      const GeomPrimitivePipelineReader *reader);
  bool upload_data(const GeomPrimitivePipelineReader *reader, bool force);

  IDirect3DIndexBuffer8 *_ibuffer;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    IndexBufferContext::init_type();
    register_type(_type_handle, "DXIndexBufferContext8",
                  IndexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxIndexBufferContext8.I"

#endif


