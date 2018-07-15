/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyXGraphicsPipe.cxx
 * @author drose
 * @date 2008-05-03
 */

#include "pandabase.h"
#ifdef HAVE_X11

#include "tinyXGraphicsPipe.h"
#include "tinyXGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "tinyGraphicsBuffer.h"
#include "config_tinydisplay.h"
#include "frameBufferProperties.h"

TypeHandle TinyXGraphicsPipe::_type_handle;

/**
 *
 */
TinyXGraphicsPipe::
TinyXGraphicsPipe(const std::string &display) : x11GraphicsPipe(display) {
}

/**
 *
 */
TinyXGraphicsPipe::
~TinyXGraphicsPipe() {
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string TinyXGraphicsPipe::
get_interface_name() const {
  return "TinyPanda";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default TinyXGraphicsPipe.
 */
PT(GraphicsPipe) TinyXGraphicsPipe::
pipe_constructor() {
  return new TinyXGraphicsPipe;
}

/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) TinyXGraphicsPipe::
make_output(const std::string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  TinyGraphicsStateGuardian *tinygsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(tinygsg, gsg, nullptr);
  }

  // First thing to try: a TinyXGraphicsWindow

  // We check _is_valid only in this case.  The pipe will be invalid if it
  // can't contact the X server, but that shouldn't prevent the creation of an
  // offscreen buffer.
  if (retry == 0 && _is_valid) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return nullptr;
    }
    return new TinyXGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                   flags, gsg, host);
  }

  // Second thing to try: a TinyGraphicsBuffer

  // No need to check _is_valid here.  We can create an offscreen buffer even
  // if the pipe is not technically valid.
  if (retry == 1) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return nullptr;
    }
    return new TinyGraphicsBuffer(engine, this, name, fb_prop, win_prop, flags, gsg, host);
  }

  // Nothing else left to try.
  return nullptr;
}

#endif  // HAVE_X11
