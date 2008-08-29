// Filename: dxVertexBufferContext9.h
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

#ifndef DXVERTEXBUFFERCONTEXT9_H
#define DXVERTEXBUFFERCONTEXT9_H

#include "pandabase.h"
#include "dxgsg9base.h"
#include "vertexBufferContext.h"

////////////////////////////////////////////////////////////////////
//       Class : DXVertexBufferContext9
// Description : Caches a GeomVertexArrayData in the DirectX device as
//               a vertex buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXVertexBufferContext9 : public VertexBufferContext {
public:
  DXVertexBufferContext9(PreparedGraphicsObjects *pgo, GeomVertexArrayData *data, DXScreenData &scrn);
  virtual ~DXVertexBufferContext9();

  virtual void evict_lru();

  void free_vbuffer();
  void allocate_vbuffer(DXScreenData &scrn, const GeomVertexArrayDataHandle *reader);
  void create_vbuffer(DXScreenData &scrn, const GeomVertexArrayDataHandle *reader, string name);
  bool upload_data(const GeomVertexArrayDataHandle *reader, bool force);

  IDirect3DVertexBuffer9 *_vbuffer;
  int _fvf;

  int _managed;
  LruPage *_lru_page;

  VERTEX_ELEMENT_TYPE *_vertex_element_type_array;
  DIRECT_3D_VERTEX_DECLARATION _direct_3d_vertex_declaration;
  CLP(ShaderContext) *_shader_context;

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
