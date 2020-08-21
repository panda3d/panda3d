/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parasiteBuffer.h
 * @author drose
 * @date 2004-02-27
 */

#ifndef PARASITEBUFFER_H
#define PARASITEBUFFER_H

#include "pandabase.h"

#include "graphicsOutput.h"
#include "texture.h"
#include "pointerTo.h"

/**
 * This is a special GraphicsOutput type that acts a lot like a
 * GraphicsBuffer, effectively allowing rendering to an offscreen buffer,
 * except it does not create any framebuffer space for itself.  Instead, it
 * renders into the framebuffer owned by some other GraphicsOutput.
 *
 * The x_size and y_size must therefore fit within the bounds of the source
 * GraphicsOutput.
 *
 * Since the framebuffer will be subsequently cleared when the actual owner
 * draws in it later, this only makes sense if we are going to copy the
 * contents of the framebuffer to a texture immediately after we draw it.
 * Thus, has_texture() is implicitly true for a ParasiteBuffer.
 *
 * This class is useful to render offscreen to a texture while preventing the
 * waste of framebuffer memory for API's that are unable to render directly
 * into a texture (and must render into a separate framebuffer first and then
 * copy to texture).  It is also the only way to render to a texture on API's
 * that do not support offscreen rendering.
 */
class EXPCL_PANDA_DISPLAY ParasiteBuffer : public GraphicsOutput {
public:
  ParasiteBuffer(GraphicsOutput *host, const std::string &name,
                 int x_size, int y_size, int flags);

PUBLISHED:
  virtual ~ParasiteBuffer();

  virtual bool is_active() const;
  void set_size(int x, int y);

public:
  void set_size_and_recalc(int x, int y);
  virtual bool flip_ready() const;
  virtual void begin_flip();
  virtual void ready_flip();
  virtual void end_flip();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual GraphicsOutput *get_host();

private:
  int _creation_flags;

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
