/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wglGraphicsPipe.cxx
 * @author drose
 * @date 2002-12-20
 */

#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include "config_windisplay.h"
#include "wglGraphicsWindow.h"
#include "wglGraphicsBuffer.h"
#include "wglGraphicsStateGuardian.h"

TypeHandle wglGraphicsPipe::_type_handle;
bool    wglGraphicsPipe::_current_valid;
HDC     wglGraphicsPipe::_current_hdc;
HGLRC   wglGraphicsPipe::_current_hglrc;
Thread *wglGraphicsPipe::_current_thread;

/**
 *
 */
wglGraphicsPipe::
wglGraphicsPipe() {
  _current_valid = false;
}

/**
 *
 */
wglGraphicsPipe::
~wglGraphicsPipe() {
}

/**
 * a thin wrapper around wglMakeCurrent to avoid unnecessary OS-call overhead.
 */
bool wglGraphicsPipe::
wgl_make_current(HDC hdc, HGLRC hglrc, PStatCollector *collector) {
  Thread *thread = Thread::get_current_thread();
  if ((_current_valid) &&
      (_current_hdc == hdc) &&
      (_current_hglrc == hglrc) &&
      (_current_thread == thread)) {
    return true;
  }
  _current_valid = true;
  _current_hdc = hdc;
  _current_hglrc = hglrc;
  _current_thread = thread;
  BOOL res;
  if (collector) {
    PStatTimer timer(*collector);
    res = wglMakeCurrent(hdc, hglrc);
  } else {
    res = wglMakeCurrent(hdc, hglrc);
  }
  return (res != 0);
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string wglGraphicsPipe::
get_interface_name() const {
  return "OpenGL";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default wglGraphicsPipe.
 */
PT(GraphicsPipe) wglGraphicsPipe::
pipe_constructor() {
  return new wglGraphicsPipe;
}

/**
 * Creates a new window or buffer on the pipe, if possible.  This routine is
 * only called from GraphicsEngine::make_output.
 */
PT(GraphicsOutput) wglGraphicsPipe::
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

  wglGraphicsStateGuardian *wglgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(wglgsg, gsg, nullptr);
  }

  bool support_rtt;
  support_rtt = false;
  if (wglgsg) {
     support_rtt =
      wglgsg -> get_supports_wgl_render_texture() &&
      support_render_texture;
  }

  // First thing to try: a wglGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)||
        ((flags&BF_can_bind_layered)!=0)) {
      return nullptr;
    }
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_hrgba() > 0)||
          (fb_prop.get_aux_float() > 0)) {
        return nullptr;
      }
    }
    return new wglGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Second thing to try: a GLGraphicsBuffer

  if (retry == 1) {
    if (!gl_support_fbo || host == nullptr ||
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
    if (wglgsg != nullptr && wglgsg->is_valid() && !wglgsg->needs_reset()) {
      if (!wglgsg->_supports_framebuffer_object ||
          wglgsg->_glDrawBuffers == nullptr) {
        return nullptr;
      } else {
        // Early success - if we are sure that this buffer WILL meet specs, we
        // can precertify it.
        precertify = true;
      }
    }
    return new GLGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                flags, gsg, host);
  }

  // Third thing to try: a wglGraphicsBuffer

  if (retry == 2) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_can_bind_layered)!=0)) {
      return nullptr;
    }
    if ((wglgsg != 0) &&
        (wglgsg->is_valid()) &&
        (!wglgsg->needs_reset()) &&
  !wglgsg->_supports_pbuffer) {
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

    // Early failure - if we are sure that this buffer WONT meet specs, we can
    // bail out early.
    if ((flags & BF_fb_props_optional) == 0) {
      if ((fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_rgba() > 0)||
          (fb_prop.get_aux_float() > 0)) {
        return nullptr;
      }
    }
    // Early success - if we are sure that this buffer WILL meet specs, we can
    // precertify the window.
    if ((wglgsg != 0) &&
        (wglgsg->is_valid()) &&
        (!wglgsg->needs_reset()) &&
        (wglgsg->pfnum_supports_pbuffer()) &&
        (wglgsg->get_fb_properties().subsumes(fb_prop))&&
        (wglgsg->get_fb_properties().is_single_buffered())) {
      precertify = true;
    }
    return new wglGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
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
PT(GraphicsStateGuardian) wglGraphicsPipe::
make_callback_gsg(GraphicsEngine *engine) {
  return new wglGraphicsStateGuardian(engine, this, nullptr);
}


/**
 * Returns pfd_flags formatted as a string in a user-friendly way.
 */
std::string wglGraphicsPipe::
format_pfd_flags(DWORD pfd_flags) {
  struct FlagDef {
    DWORD flag;
    const char *name;
  };
  static FlagDef flag_def[] = {
    { PFD_DRAW_TO_WINDOW, "PFD_DRAW_TO_WINDOW" },
    { PFD_DRAW_TO_BITMAP, "PFD_DRAW_TO_BITMAP" },
    { PFD_SUPPORT_GDI, "PFD_SUPPORT_GDI" },
    { PFD_SUPPORT_OPENGL, "PFD_SUPPORT_OPENGL" },
    { PFD_GENERIC_ACCELERATED, "PFD_GENERIC_ACCELERATED" },
    { PFD_GENERIC_FORMAT, "PFD_GENERIC_FORMAT" },
    { PFD_NEED_PALETTE, "PFD_NEED_PALETTE" },
    { PFD_NEED_SYSTEM_PALETTE, "PFD_NEED_SYSTEM_PALETTE" },
    { PFD_DOUBLEBUFFER, "PFD_DOUBLEBUFFER" },
    { PFD_STEREO, "PFD_STEREO" },
    { PFD_SWAP_LAYER_BUFFERS, "PFD_SWAP_LAYER_BUFFERS" },
    { PFD_SWAP_COPY, "PFD_SWAP_COPY" },
    { PFD_SWAP_EXCHANGE, "PFD_SWAP_EXCHANGE" },
  };
  static const int num_flag_defs = sizeof(flag_def) / sizeof(FlagDef);

  std::ostringstream out;

  const char *sep = "";
  bool got_any = false;
  for (int i = 0; i < num_flag_defs; i++) {
    if (pfd_flags & flag_def[i].flag) {
      out << sep << flag_def[i].name;
      pfd_flags &= ~flag_def[i].flag;
      sep = "|";
      got_any = true;
    }
  }

  if (pfd_flags != 0 || !got_any) {
    out << sep << std::hex << "0x" << pfd_flags << std::dec;
  }

  return out.str();
}
