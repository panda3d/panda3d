// Filename: dxSavedFrameBuffer8.h
// Created by:  drose (06Oct99)
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

#ifndef DXSAVEDFRAMEBUFFER8_H
#define DXSAVEDFRAMEBUFFER8_H

#include "pandabase.h"

#include "savedFrameBuffer.h"
#include "texture.h"
#include "textureContext.h"
#include "pixelBuffer.h"


////////////////////////////////////////////////////////////////////
//   Class : DXSavedFrameBuffer8
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXSavedFrameBuffer8 : public SavedFrameBuffer {
public:
  INLINE DXSavedFrameBuffer8(const RenderBuffer &buffer,
                CPT(DisplayRegion) dr);
  INLINE ~DXSavedFrameBuffer8();

  PT(Texture) _back_rgba;
  PT(PixelBuffer) _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedFrameBuffer::init_type();
    register_type(_type_handle, "DXSavedFrameBuffer8",
          SavedFrameBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "DXSavedFrameBuffer8.I"

#endif

