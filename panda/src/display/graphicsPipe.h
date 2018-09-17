/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsPipe.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef GRAPHICSPIPE_H
#define GRAPHICSPIPE_H

#include "pandabase.h"

#include "graphicsDevice.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "lightMutex.h"

class GraphicsEngine;
class GraphicsOutput;
class GraphicsWindow;
class GraphicsBuffer;
class GraphicsStateGuardian;
class FrameBufferProperties;
class WindowProperties;
class Texture;
class WindowHandle;
class DisplayInformation;

/**
 * An object to create GraphicsOutputs that share a particular 3-D API.
 * Normally, there will only be one GraphicsPipe in an application, although
 * it is possible to have multiple of these at once if there are multiple
 * different API's available in the same machine.
 *
 * Often, the GraphicsPipe corresponds to a physical output device, hence the
 * term "pipe", but this is not necessarily the case.
 *
 * The GraphicsPipe is used by the GraphicsEngine object to create and destroy
 * windows; it keeps ownership of the windows it creates.
 *
 * M. Asad added new/interim functionality where GraphicsPipe now contains a
 * device interface to directx/opengl which will be used to handle multiple
 * windows from same device.
 *
 */
class EXPCL_PANDA_DISPLAY GraphicsPipe : public TypedReferenceCount {
protected:
  GraphicsPipe();
  GraphicsPipe(const GraphicsPipe &copy) = delete;
  GraphicsPipe &operator = (const GraphicsPipe &copy) = delete;

PUBLISHED:
  virtual ~GraphicsPipe();

  enum OutputTypes {
    OT_window            = 0x0001,
    OT_fullscreen_window = 0x0002,
    OT_buffer            = 0x0004,
    OT_texture_buffer    = 0x0008,
  };

  enum BufferCreationFlags {
    // Flags that control what type of output is returned.
    BF_refuse_parasite     = 0x0001,
    BF_require_parasite    = 0x0002,
    BF_refuse_window       = 0x0004,
    BF_require_window      = 0x0008,
    BF_require_callback_window = 0x0010,

    // Miscellaneous control flags.
    BF_can_bind_color      = 0x0040, // Need capability: bind the color bitplane to a tex.
    BF_can_bind_every      = 0x0080, // Need capability: bind all bitplanes to a tex.
    BF_resizeable          = 0x0100, // Buffer should allow set_size.
    BF_size_track_host     = 0x0200, // Buffer should track the host size.
    BF_rtt_cumulative      = 0x0400, // Buffer supports cumulative render-to-texture.
    BF_fb_props_optional   = 0x0800, // FrameBufferProperties can be ignored.
    BF_size_square         = 0x1000, // x_size must equal y_size (e.g. for cube maps)
    BF_size_power_2        = 0x2000, // x_size and y_size must each be a power of two
    BF_can_bind_layered    = 0x4000, // Need capability: support RTM_bind_layered.
  };

  INLINE bool is_valid() const;
  INLINE int get_supported_types() const;
  INLINE bool supports_type(int flags) const;

  INLINE int get_display_width() const;
  INLINE int get_display_height() const;
  MAKE_PROPERTY(display_width, get_display_width);
  MAKE_PROPERTY(display_height, get_display_height);

  DisplayInformation *get_display_information();
  MAKE_PROPERTY(display_information, get_display_information);

  virtual void lookup_cpu_data();

  virtual std::string get_interface_name() const=0;
  MAKE_PROPERTY(interface_name, get_interface_name);

public:
  enum PreferredWindowThread {
    PWT_app,
    PWT_draw
  };
  virtual PreferredWindowThread get_preferred_window_thread() const;

  INLINE GraphicsDevice *get_device() const;
  virtual PT(GraphicsDevice) make_device(void *scrn = nullptr);

  virtual PT(GraphicsStateGuardian) make_callback_gsg(GraphicsEngine *engine);

protected:
  virtual void close_gsg(GraphicsStateGuardian *gsg);

  virtual PT(GraphicsOutput) make_output(const std::string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);

  LightMutex _lock;

  bool _is_valid;
  int _supported_types;
  int _display_width;
  int _display_height;
  PT(GraphicsDevice) _device;

  DisplayInformation *_display_information;

  static const int strip_properties[];

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
