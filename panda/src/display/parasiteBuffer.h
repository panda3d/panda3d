// Filename: parasiteBuffer.h
// Created by:  drose (27Feb04)
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

#ifndef PARASITEBUFFER_H
#define PARASITEBUFFER_H

#include "pandabase.h"

#include "graphicsOutput.h"
#include "texture.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : ParasiteBuffer
// Description : This is a special GraphicsOutput type that acts a lot
//               like a GraphicsBuffer, effectively allowing rendering
//               to an offscreen buffer, except it does not create any
//               framebuffer space for itself.  Instead, it renders
//               into the framebuffer owned by some other
//               GraphicsOutput.
//
//               The x_size and y_size must therefore fit within the
//               bounds of the source GraphicsOutput.
//
//               Since the framebuffer will be subsequently cleared
//               when the actual owner draws in it later, this only
//               makes sense if we are going to copy the contents of
//               the framebuffer to a texture immediately after we
//               draw it.  Thus, has_texture() is implicitly true for
//               a ParasiteBuffer.
//
//               This class is useful to render offscreen to a texture
//               while preventing the waste of framebuffer memory for
//               API's that are unable to render directly into a
//               texture (and must render into a separate framebuffer
//               first and then copy to texture).  It is also the only
//               way to render to a texture on API's that do not
//               support offscreen rendering.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ParasiteBuffer : public GraphicsOutput {
public:
  ParasiteBuffer(GraphicsOutput *host, const string &name,
                 int x_size, int y_size);
  
PUBLISHED:
  virtual ~ParasiteBuffer();

  virtual bool is_active() const;

public:
  virtual GraphicsOutput *get_host();
  virtual void make_current();
  virtual void auto_resize();

private:
  PT(GraphicsOutput) _host;
  bool _track_host_size;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsOutput::init_type();
    register_type(_type_handle, "ParasiteBuffer",
                  GraphicsOutput::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "parasiteBuffer.I"

#endif
