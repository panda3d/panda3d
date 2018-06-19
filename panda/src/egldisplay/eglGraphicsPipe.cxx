/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsPipe.cxx
 * @author rdb
 * @date 2009-05-21
 */

#include "eglGraphicsBuffer.h"
#include "eglGraphicsPipe.h"
#include "eglGraphicsPixmap.h"
#include "eglGraphicsWindow.h"
#include "eglGraphicsStateGuardian.h"
#include "config_egldisplay.h"
#include "frameBufferProperties.h"

TypeHandle eglGraphicsPipe::_type_handle;

/**
 *
 */
eglGraphicsPipe::
eglGraphicsPipe(const std::string &display) : x11GraphicsPipe(display) {
  _egl_display = eglGetDisplay((NativeDisplayType) _display);
  if (!eglInitialize(_egl_display, nullptr, nullptr)) {
    egldisplay_cat.error()
      << "Couldn't initialize the EGL display: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    egldisplay_cat.error()
      << "Couldn't bind EGL to the OpenGL ES API: "
      << get_egl_error_string(eglGetError()) << "\n";
  }
}

/**
 *
 */
eglGraphicsPipe::
~eglGraphicsPipe() {
  if (_egl_display) {
    if (!eglTerminate(_egl_display)) {
      egldisplay_cat.error() << "Failed to terminate EGL display: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
  }
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string eglGraphicsPipe::
get_interface_name() const {
  return "OpenGL ES";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default eglGraphicsPipe.
 */
PT(GraphicsPipe) eglGraphicsPipe::
pipe_constructor() {
  return new eglGraphicsPipe;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) eglGraphicsPipe::
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

  eglGraphicsStateGuardian *eglgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(eglgsg, gsg, nullptr);
  }

  bool support_rtt;
  support_rtt = false;
  /*
    Currently, no support for eglGraphicsBuffer render-to-texture.
  if (eglgsg) {
     support_rtt =
      eglgsg -> get_supports_render_texture() &&
      support_render_texture;
  }
  */

  // First thing to try: an eglGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }
    return new eglGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Second thing to try: a GLES(2)GraphicsBuffer
  if (retry == 1) {
    if ((host==0)||
  // (!gl_support_fbo)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return nullptr;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_indexed_color() > 0)||
          (fb_prop.get_back_buffers() > 0)||
          (fb_prop.get_accum_bits() > 0)||
          (fb_prop.get_multisamples() > 0)) {
        return nullptr;
      }
    }
    // Early success - if we are sure that this buffer WILL meet specs, we can
    // precertify it.
    if ((eglgsg != 0) &&
        (eglgsg->is_valid()) &&
        (!eglgsg->needs_reset()) &&
        (eglgsg->_supports_framebuffer_object) &&
        (eglgsg->_glDrawBuffers != 0)&&
        (fb_prop.is_basic())) {
      precertify = true;
    }
#ifdef OPENGLES_2
    return new GLES2GraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
#else
    return new GLESGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
#endif
  }

  // Third thing to try: a eglGraphicsBuffer
  if (retry == 2) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return nullptr;
    }

    if (!support_rtt) {
      if (((flags&BF_rtt_cumulative)!=0)||
          ((flags&BF_can_bind_every)!=0)) {
        // If we require Render-to-Texture, but can't be sure we support it,
        // bail.
        return nullptr;
      }
    }

    return new eglGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Fourth thing to try: an eglGraphicsPixmap.
  if (retry == 3) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return nullptr;
    }

    if (((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }

    return new eglGraphicsPixmap(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Nothing else left to try.
  return nullptr;
}
