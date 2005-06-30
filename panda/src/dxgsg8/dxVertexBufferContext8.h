// Filename: dxVertexBufferContext8.h
// Created by:  drose (18Mar05)
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
  DXVertexBufferContext8(GeomVertexArrayData *data);
  virtual ~DXVertexBufferContext8();

  void create_vbuffer(DXScreenData &scrn);
  void upload_data();

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


