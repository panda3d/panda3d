/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGLGraphicsPipe.mm
 * @author rdb
 * @date 2023-03-20
 */

#include "cocoaGLGraphicsPipe.h"
#include "cocoaGLGraphicsBuffer.h"
#include "cocoaGLGraphicsWindow.h"
#include "cocoaGLGraphicsStateGuardian.h"
#include "config_cocoagldisplay.h"
#include "frameBufferProperties.h"

TypeHandle CocoaGLGraphicsPipe::_type_handle;

/**
 * Takes a CoreGraphics display ID, which defaults to the main display.
 */
CocoaGLGraphicsPipe::
CocoaGLGraphicsPipe(CGDirectDisplayID display) : CocoaGraphicsPipe(display) {
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _is_valid = true;
}

/**
 *
 */
CocoaGLGraphicsPipe::
~CocoaGLGraphicsPipe() {
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string CocoaGLGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default CocoaGLGraphicsPipe.
 */
PT(GraphicsPipe) CocoaGLGraphicsPipe::
pipe_constructor() {
  return new CocoaGLGraphicsPipe;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) CocoaGLGraphicsPipe::
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return nullptr;
  }

  CocoaGLGraphicsStateGuardian *cocoagsg = nullptr;
  if (gsg != nullptr) {
    DCAST_INTO_R(cocoagsg, gsg, nullptr);
  }

  // First thing to try: a CocoaGLGraphicsWindow

  if (retry == 0) {
    if ((flags & BF_require_parasite) != 0 ||
        (flags & BF_refuse_window) != 0 ||
        (flags & BF_resizeable) != 0 ||
        (flags & BF_size_track_host) != 0 ||
        (flags & BF_rtt_cumulative) != 0 ||
        (flags & BF_can_bind_color) != 0 ||
        (flags & BF_can_bind_every) != 0 ||
        (flags & BF_can_bind_layered) != 0) {
      return nullptr;
    }
    return new CocoaGLGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                     flags, gsg, host);
  }

  // Second thing to try: a GLGraphicsBuffer.  This requires a context, so if
  // we don't have a host window, we instead create a CocoaGLGraphicsBuffer,
  // which wraps around GLGraphicsBuffer and manages a context.

  if (retry == 1) {
    if (!gl_support_fbo ||
        (flags & (BF_require_parasite | BF_require_window)) != 0) {
      return nullptr;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if (fb_prop.get_indexed_color() ||
          fb_prop.get_back_buffers() > 0 ||
          fb_prop.get_accum_bits() > 0) {
        return nullptr;
      }
    }
    if (cocoagsg != nullptr && cocoagsg->is_valid() && !cocoagsg->needs_reset()) {
      if (!cocoagsg->_supports_framebuffer_object ||
          cocoagsg->_glDrawBuffers == nullptr) {
        return nullptr;
      }
      else if (fb_prop.is_basic()) {
        // Early success - if we are sure that this buffer WILL meet specs, we
        // can precertify it.
        precertify = true;
      }
    }
    if (host != nullptr && host->get_engine() == engine) {
      return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
    } else {
      return new CocoaGLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                       flags, gsg, nullptr);
    }
  }

  // Nothing else left to try.
  return nullptr;
}

/**
 * This is called when make_output() is used to create a
 * CallbackGraphicsWindow.  If the GraphicsPipe can construct a GSG that's not
 * associated with any particular window object, do so now, assuming the
 * correct graphics context has been set up externally.
 */
PT(GraphicsStateGuardian) CocoaGLGraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return new CocoaGLGraphicsStateGuardian(engine, this, nullptr);
}
