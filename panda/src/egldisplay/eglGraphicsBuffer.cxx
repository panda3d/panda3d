/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsBuffer.cxx
 * @author rdb
 * @date 2009-06-13
 */

#include "eglGraphicsBuffer.h"
#include "eglGraphicsStateGuardian.h"
#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"

#include "graphicsPipe.h"
#include "pStatTimer.h"

TypeHandle eglGraphicsBuffer::_type_handle;

/**
 *
 */
eglGraphicsBuffer::
eglGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const std::string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_V(egl_pipe, _pipe);
  _pbuffer = EGL_NO_SURFACE;

  // Since the pbuffer never gets flipped, we get screenshots from the same
  // buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

/**
 *
 */
eglGraphicsBuffer::
~eglGraphicsBuffer() {
  nassertv(_pbuffer == EGL_NO_SURFACE);
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool eglGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  eglGraphicsStateGuardian *eglgsg;
  DCAST_INTO_R(eglgsg, _gsg, false);
  if (!eglMakeCurrent(eglgsg->_egl_display, _pbuffer, _pbuffer, eglgsg->_context)) {
    egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  eglgsg->reset_if_new();

  if (mode == FM_render) {
    CDLockedReader cdata(_cycler);
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      if (rtm_mode == RTM_bind_or_copy) {
        CDWriter cdataw(_cycler, cdata, false);
        nassertr(cdata->_textures.size() == cdataw->_textures.size(), false);
        cdataw->_textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void eglGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != nullptr);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

/**
 * Closes the buffer right now.  Called from the window thread.
 */
void eglGraphicsBuffer::
close_buffer() {
  if (_gsg != nullptr) {
    eglGraphicsStateGuardian *eglgsg;
    DCAST_INTO_V(eglgsg, _gsg);
    if (!eglMakeCurrent(eglgsg->_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
      egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
    _gsg.clear();

    if (_pbuffer != EGL_NO_SURFACE) {
      if (!eglDestroySurface(_egl_display, _pbuffer)) {
        egldisplay_cat.error() << "Failed to destroy surface: "
          << get_egl_error_string(eglGetError()) << "\n";
      }
      _pbuffer = EGL_NO_SURFACE;
    }
  }

  _is_valid = false;
}

/**
 * Opens the buffer right now.  Called from the window thread.  Returns true
 * if the buffer is successfully opened, or false if there was a problem.
 */
bool eglGraphicsBuffer::
open_buffer() {
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_R(egl_pipe, _pipe, false);

  // GSG CreationInitialization
  eglGraphicsStateGuardian *eglgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    eglgsg = new eglGraphicsStateGuardian(_engine, _pipe, nullptr);
    eglgsg->choose_pixel_format(_fb_properties, egl_pipe->get_display(), egl_pipe->get_screen(), true, false);
    _gsg = eglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(eglgsg, _gsg, false);
    if (!eglgsg->get_fb_properties().subsumes(_fb_properties)) {
      eglgsg = new eglGraphicsStateGuardian(_engine, _pipe, eglgsg);
      eglgsg->choose_pixel_format(_fb_properties, egl_pipe->get_display(), egl_pipe->get_screen(), true, false);
      _gsg = eglgsg;
    }
  }

  if (eglgsg->_fbconfig == None) {
    // If we didn't use an fbconfig to create the GSG, we can't create a
    // PBuffer.
    return false;
  }

  int attrib_list[] = {
    EGL_WIDTH, _size.get_x(),
    EGL_HEIGHT, _size.get_y(),
    EGL_NONE
  };

  _pbuffer = eglCreatePbufferSurface(eglgsg->_egl_display, eglgsg->_fbconfig, attrib_list);

  if (_pbuffer == EGL_NO_SURFACE) {
    egldisplay_cat.error()
      << "Failed to create EGL pbuffer surface: "
      << get_egl_error_string(eglGetError()) << "\n";
    return false;
  }

  if (!eglMakeCurrent(eglgsg->_egl_display, _pbuffer, _pbuffer, eglgsg->_context)) {
    egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
      << get_egl_error_string(eglGetError()) << "\n";
  }
  eglgsg->reset_if_new();
  if (!eglgsg->is_valid()) {
    close_buffer();
    return false;
  }
  if (!eglgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, eglgsg->get_gl_renderer())) {
    close_buffer();
    return false;
  }
  _fb_properties = eglgsg->get_fb_properties();

  _is_valid = true;
  return true;
}
