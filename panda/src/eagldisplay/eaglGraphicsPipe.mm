/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file EAGLGraphicsPipe.mm
 * @author D. Lawrence
 * @date 2019-01-03
 */

#include "eaglGraphicsPipe.h"
#include "eaglGraphicsWindow.h"
#include "eaglGraphicsStateGuardian.h"
#include "config_eagldisplay.h"
#include "frameBufferProperties.h"
#include "displayInformation.h"

#import <UIKit/UIKit.h>

TypeHandle EAGLGraphicsPipe::_type_handle;

/**
 *
 */
EAGLGraphicsPipe::
EAGLGraphicsPipe() {
  // TODO: Support more than just GraphicsWindows.
  _supported_types = OT_window;
  _is_valid = true;
}

/**
 *
 */
EAGLGraphicsPipe::
~EAGLGraphicsPipe() {
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string EAGLGraphicsPipe::
get_interface_name() const {
  return "OpenGLES 2";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default EAGLGraphicsPipe.
 */
PT(GraphicsPipe) EAGLGraphicsPipe::
pipe_constructor() {
  return new EAGLGraphicsPipe;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread
EAGLGraphicsPipe::get_preferred_window_thread() const {
  // Anything inside UIKit must be called from the main thread.
  return PWT_app;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) EAGLGraphicsPipe::
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
  
  EAGLGraphicsStateGuardian *eaglgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(eaglgsg, gsg, NULL);
  }
  
  // First thing to try: an EAGLGraphicsWindow.
  
  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)||
        ((flags&BF_can_bind_layered)!=0)) {
      return NULL;
    }
    return new EAGLGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }

  // Second thing to try: a GLGraphicsBuffer.  This requires a context, so if
  // we don't have a host window, we instead create a EAGLGraphicsBuffer,
  // which wraps around GLGraphicsBuffer and manages a context.
  if (retry == 1) {
    if (!gl_support_fbo ||
        (flags & (BF_require_parasite | BF_require_window)) != 0) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if (fb_prop.get_indexed_color() ||
          fb_prop.get_back_buffers() > 0 ||
          fb_prop.get_accum_bits() > 0) {
        return NULL;
      }
    }

    if (host != NULL) {
      return new GLES2GraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
    } else {
      return new EAGLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                     flags, gsg, host);
    }
  }
  
  // Nothing else left to try.
  return NULL;
}
