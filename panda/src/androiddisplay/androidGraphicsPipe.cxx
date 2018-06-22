/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file androidGraphicsPipe.cxx
 * @author rdb
 * @date 2013-01-11
 */

// #include "androidGraphicsBuffer.h"
#include "androidGraphicsPipe.h"
// #include "androidGraphicsPixmap.h"
#include "androidGraphicsWindow.h"
#include "androidGraphicsStateGuardian.h"
#include "config_androiddisplay.h"
#include "frameBufferProperties.h"

TypeHandle AndroidGraphicsPipe::_type_handle;

/**
 *
 */
AndroidGraphicsPipe::
AndroidGraphicsPipe() {
  _is_valid = false;
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _egl_display = nullptr;

  _display_width = 0;
  _display_height = 0;

  _egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (!eglInitialize(_egl_display, nullptr, nullptr)) {
    androiddisplay_cat.error()
      << "Couldn't initialize the EGL display: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    androiddisplay_cat.error()
      << "Couldn't bind EGL to the OpenGL ES API: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  _is_valid = true;
}

/**
 *
 */
AndroidGraphicsPipe::
~AndroidGraphicsPipe() {
  if (_egl_display) {
    if (!eglTerminate(_egl_display)) {
      androiddisplay_cat.error() << "Failed to terminate EGL display: "
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
std::string AndroidGraphicsPipe::
get_interface_name() const {
  return "OpenGL ES";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default AndroidGraphicsPipe.
 */
PT(GraphicsPipe) AndroidGraphicsPipe::
pipe_constructor() {
  return new AndroidGraphicsPipe;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread
AndroidGraphicsPipe::get_preferred_window_thread() const {
  // Most of the Android NDK window functions can be called from any thread.
  // Since we're creating the context at open_window time, let's choose
  // "draw".
  return PWT_app;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) AndroidGraphicsPipe::
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

  AndroidGraphicsStateGuardian *androidgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(androidgsg, gsg, nullptr);
  }

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
    return new AndroidGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                     flags, gsg, host);
  }

  // Second thing to try: a GLES(2)GraphicsBuffer
  /*if (retry == 1) {
    if ((host==0)||
  // (!gl_support_fbo)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_indexed_color() > 0)||
          (fb_prop.get_back_buffers() > 0)||
          (fb_prop.get_accum_bits() > 0)||
          (fb_prop.get_multisamples() > 0)) {
        return NULL;
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
      return NULL;
    }

    if (!support_rtt) {
      if (((flags&BF_rtt_cumulative)!=0)||
          ((flags&BF_can_bind_every)!=0)) {
        // If we require Render-to-Texture, but can't be sure we support it,
        // bail.
        return NULL;
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
      return NULL;
    }

    if (((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }

    return new eglGraphicsPixmap(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }*/

  // Nothing else left to try.
  return nullptr;
}
