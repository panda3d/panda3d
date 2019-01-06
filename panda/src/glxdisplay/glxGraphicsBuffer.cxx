/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glxGraphicsBuffer.cxx
 * @author drose
 * @date 2004-02-09
 */

#include "glxGraphicsBuffer.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"

#include "graphicsPipe.h"
#include "glgsg.h"
#include "pStatTimer.h"

TypeHandle glxGraphicsBuffer::_type_handle;

/**
 *
 */
glxGraphicsBuffer::
glxGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const std::string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  _display = glx_pipe->get_display();
  _pbuffer = None;

  // Since the pbuffer never gets flipped, we get screenshots from the same
  // buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

/**
 *
 */
glxGraphicsBuffer::
~glxGraphicsBuffer() {
  nassertv(_pbuffer == None);
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool glxGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == nullptr ||
      _pbuffer == None) {
    return false;
  }

  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_R(glxgsg, _gsg, false);
  {
    LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);
    glXMakeCurrent(_display, _pbuffer, glxgsg->_context);
  }

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  glxgsg->reset_if_new();

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
void glxGraphicsBuffer::
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
void glxGraphicsBuffer::
close_buffer() {
  if (_gsg != nullptr) {
    LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);
    glXMakeCurrent(_display, None, nullptr);

    if (_pbuffer != None) {
      glxGraphicsStateGuardian *glxgsg;
      DCAST_INTO_V(glxgsg, _gsg);
      glxgsg->_glXDestroyPbuffer(_display, _pbuffer);
      _pbuffer = None;
    }

    _gsg.clear();
  }

  _pbuffer = None;
  _is_valid = false;
}

/**
 * Opens the buffer right now.  Called from the window thread.  Returns true
 * if the buffer is successfully opened, or false if there was a problem.
 */
bool glxGraphicsBuffer::
open_buffer() {
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_R(glx_pipe, _pipe, false);

  // GSG CreationInitialization
  glxGraphicsStateGuardian *glxgsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    glxgsg = new glxGraphicsStateGuardian(_engine, _pipe, nullptr);
    glxgsg->choose_pixel_format(_fb_properties, glx_pipe->get_display(), glx_pipe->get_screen(), true, false);
    _gsg = glxgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(glxgsg, _gsg, false);

    if (!glxgsg->_context_has_pbuffer ||
        !glxgsg->get_fb_properties().subsumes(_fb_properties)) {
      // We need a new pixel format, and hence a new GSG.
      glxgsg = new glxGraphicsStateGuardian(_engine, _pipe, glxgsg);
      glxgsg->choose_pixel_format(_fb_properties, glx_pipe->get_display(), glx_pipe->get_screen(), true, false);
      _gsg = glxgsg;
    }
  }

  if (glxgsg->_fbconfig == None || !glxgsg->_context_has_pbuffer) {
    // If we didn't use an fbconfig to create the GSG, or it doesn't support
    // buffers, we can't create a PBuffer.
    return false;
  }

  nassertr(glxgsg->_supports_pbuffer, false);

  LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);

  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n = 0;

  if (glxgsg->_uses_sgix_pbuffer) {
    // The SGI version passed in the size in the parameter list.
    nassertr(n < max_attrib_list, false);
    attrib_list[n] = (int)None;
    _pbuffer = glxgsg->_glXCreateGLXPbufferSGIX(glxgsg->_display, glxgsg->_fbconfig,
                                                get_x_size(), get_y_size(), attrib_list);
  } else {
    // The official GLX 1.3 version passes in the size in the attrib list.
    attrib_list[n++] = GLX_PBUFFER_WIDTH;
    attrib_list[n++] = get_x_size();
    attrib_list[n++] = GLX_PBUFFER_HEIGHT;
    attrib_list[n++] = get_y_size();

    nassertr(n < max_attrib_list, false);
    attrib_list[n] = (int)None;
    _pbuffer = glxgsg->_glXCreatePbuffer(glxgsg->_display, glxgsg->_fbconfig,
                                         attrib_list);
  }

  if (_pbuffer == None) {
    glxdisplay_cat.error()
      << "failed to create GLX pbuffer.\n";
    return false;
  }

  glXMakeCurrent(_display, _pbuffer, glxgsg->_context);
  glxgsg->reset_if_new();
  if (!glxgsg->is_valid()) {
    close_buffer();
    return false;
  }
  if (!glxgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, glxgsg->get_gl_renderer())) {
    close_buffer();
    return false;
  }
  _fb_properties = glxgsg->get_fb_properties();

  _is_valid = true;
  return true;
}
