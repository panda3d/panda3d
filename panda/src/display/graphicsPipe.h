// Filename: graphicsPipe.h
// Created by:  mike (09Jan97)
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

#ifndef GRAPHICSPIPE_H
#define GRAPHICSPIPE_H

#include "pandabase.h"

#include "graphicsDevice.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "pmutex.h"

class HardwareChannel;
class GraphicsOutput;
class GraphicsWindow;
class GraphicsBuffer;
class GraphicsStateGuardian;
class FrameBufferProperties;
class Texture;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsPipe
// Description : An object to create GraphicsOutputs that share a
//               particular 3-D API.  Normally, there will only be one
//               GraphicsPipe in an application, although it is
//               possible to have multiple of these at once if there
//               are multiple different API's available in the same
//               machine.
//
//               Often, the GraphicsPipe corresponds to a physical
//               output device, hence the term "pipe", but this is not
//               necessarily the case.
//
//               The GraphicsPipe is used by the GraphicsEngine object
//               to create and destroy windows; it keeps ownership of
//               the windows it creates.
//
//               M. Asad added new/interim functionality where GraphicsPipe
//               now contains a device interface to directx/opengl which
//               will be used to handle multiple windows from same device.
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsPipe : public TypedReferenceCount {
protected:
  GraphicsPipe();
private:
  GraphicsPipe(const GraphicsPipe &copy);
  void operator = (const GraphicsPipe &copy);

PUBLISHED:
  virtual ~GraphicsPipe();

  enum OutputTypes {
    OT_window            = 0x0001,
    OT_fullscreen_window = 0x0002,
    OT_buffer            = 0x0004,
    OT_texture_buffer    = 0x0008,
  };

  INLINE bool is_valid() const;
  INLINE int get_supported_types() const;
  INLINE bool supports_type(int flags) const;

  INLINE int get_display_width() const;
  INLINE int get_display_height() const;

  virtual string get_interface_name() const=0;

public:
  virtual int get_num_hw_channels();
  virtual HardwareChannel *get_hw_channel(GraphicsOutput *window, int index);

  INLINE GraphicsDevice *get_device() const;
  virtual PT(GraphicsDevice) make_device(void *scrn = NULL);

protected:
  // The make_window() and make_gsg() interfaces on GraphicsPipe are
  // protected; don't try to call them directly.  Instead, use
  // the interface on GraphicsEngine to make a new window or gsg.
  virtual PT(GraphicsStateGuardian) make_gsg(const FrameBufferProperties &properties);
  virtual void close_gsg(GraphicsStateGuardian *gsg);
  virtual PT(GraphicsWindow) make_window(GraphicsStateGuardian *gsg);
  virtual PT(GraphicsBuffer) make_buffer(GraphicsStateGuardian *gsg, 
                                         int x_size, int y_size, bool want_texture);

  Mutex _lock;

  bool _is_valid;
  int _supported_types;
  int _display_width;
  int _display_height;
  PT(GraphicsDevice) _device;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsPipe",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  friend class GraphicsEngine;
};

#include "graphicsPipe.I"

#endif /* GRAPHICSPIPE_H */
