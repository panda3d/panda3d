// Filename: dxIndexBufferContext9.h
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

#ifndef DXINDEXBUFFERCONTEXT9_H
#define DXINDEXBUFFERCONTEXT9_H

#include "pandabase.h"
#include "dxgsg9base.h"
#include "indexBufferContext.h"

////////////////////////////////////////////////////////////////////
//       Class : DXIndexBufferContext9
// Description : Caches a GeomPrimitive in the DirectX device as
//               an index buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXIndexBufferContext9 : public IndexBufferContext {
public:
  DXIndexBufferContext9(GeomPrimitive *data);
  virtual ~DXIndexBufferContext9();

  void free_ibuffer(void);
  void allocate_ibuffer(DXScreenData &scrn);
  void create_ibuffer(DXScreenData &scrn);
  void upload_data();

  IDirect3DIndexBuffer9 *_ibuffer;
  int _managed;
  LruPage *_lru_page;

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
