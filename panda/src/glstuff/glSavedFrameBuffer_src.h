// Filename: glSavedFrameBuffer_src.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "savedFrameBuffer.h"
#include "texture.h"
#include "textureContext.h"
#include "pixelBuffer.h"


////////////////////////////////////////////////////////////////////
//       Class : GLSavedFrameBuffer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_GL CLP(SavedFrameBuffer) : public SavedFrameBuffer {
public:
  INLINE CLP(SavedFrameBuffer)(const RenderBuffer &buffer,
                            CPT(DisplayRegion) dr);
  INLINE ~CLP(SavedFrameBuffer)();

  PT(Texture) _back_rgba;
  PT(PixelBuffer) _depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedFrameBuffer::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "SavedFrameBuffer",
                  SavedFrameBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glSavedFrameBuffer_src.I"

