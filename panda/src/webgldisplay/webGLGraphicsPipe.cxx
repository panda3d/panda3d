/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webGLGraphicsPipe.cxx
 * @author rdb
 * @date 2015-04-01
 */

#include "webGLGraphicsPipe.h"
#include "webGLGraphicsWindow.h"
#include "webGLGraphicsStateGuardian.h"
#include "config_webgldisplay.h"
#include "frameBufferProperties.h"

TypeHandle WebGLGraphicsPipe::_type_handle;

/**
 *
 */
WebGLGraphicsPipe::
WebGLGraphicsPipe() {
}

/**
 *
 */
WebGLGraphicsPipe::
~WebGLGraphicsPipe() {
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string WebGLGraphicsPipe::
get_interface_name() const {
  return "WebGL";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default webGLGraphicsPipe.
 */
PT(GraphicsPipe) WebGLGraphicsPipe::
pipe_constructor() {
  return new WebGLGraphicsPipe;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread WebGLGraphicsPipe::
get_preferred_window_thread() const {
  // JavaScript has no threads, so does it matter?
  return PWT_draw;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) WebGLGraphicsPipe::
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
    return NULL;
  }

  WebGLGraphicsStateGuardian *web_gl = 0;
  if (gsg != NULL) {
    DCAST_INTO_R(web_gl, gsg, NULL);
  }

  // First thing to try: a WebGLGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new WebGLGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }

  // Second thing to try: a GLES2GraphicsBuffer
  if (retry == 1) {
    if ((host==0)||
        //(!gl_support_fbo)||
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
    if ((web_gl != 0) &&
        (web_gl->is_valid()) &&
        (!web_gl->needs_reset()) &&
        (fb_prop.is_basic())) {
      precertify = true;
    }
    return new GLES2GraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
  }

  // Nothing else left to try.
  return NULL;
}
