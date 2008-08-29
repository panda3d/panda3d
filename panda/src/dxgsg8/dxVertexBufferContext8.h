// Filename: dxVertexBufferContext8.h
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

#ifndef DXVERTEXBUFFERCONTEXT8_H
#define DXVERTEXBUFFERCONTEXT8_H

#include "pandabase.h"
#include "dxgsg8base.h"
#include "vertexBufferContext.h"

////////////////////////////////////////////////////////////////////
//       Class : DXVertexBufferContext8
// Description : Caches a GeomVertexArrayData in the DirectX device as
//               a vertex buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXVertexBufferContext8 : public VertexBufferContext {
public:
  DXVertexBufferContext8(PreparedGraphicsObjects *pgo, GeomVertexArrayData *data);
  virtual ~DXVertexBufferContext8();

  virtual void evict_lru();

  void create_vbuffer(DXScreenData &scrn,
                      const GeomVertexArrayDataHandle *reader);
  bool upload_data(const GeomVertexArrayDataHandle *reader, bool force);

  IDirect3DVertexBuffer8 *_vbuffer;
  int _fvf;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexBufferContext::init_type();
    register_type(_type_handle, "DXVertexBufferContext8",
                  VertexBufferContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxVertexBufferContext8.I"

#endif


