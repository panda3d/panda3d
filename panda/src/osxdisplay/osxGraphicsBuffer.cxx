////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "osxGraphicsBuffer.h"
#include "osxGraphicsStateGuardian.h"
#include "config_osxdisplay.h"
#include "osxGraphicsPipe.h"

#include "graphicsPipe.h"
#include "glgsg.h"
#include "pStatTimer.h"

TypeHandle osxGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsBuffer::
osxGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  osxGraphicsPipe *osx_pipe;
  DCAST_INTO_V(osx_pipe, _pipe);

  _pbuffer = NULL;
 
  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsBuffer::
~osxGraphicsBuffer() {
  nassertv(_pbuffer == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool osxGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector);

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }
  nassertr(_pbuffer != NULL, false);

  osxGraphicsStateGuardian *osxgsg;
  DCAST_INTO_R(osxgsg, _gsg, false);
  if (!aglSetPBuffer(osxgsg->get_context(), _pbuffer, 0, 0, 0)) {
    report_agl_error("aglSetPBuffer");
    return false;
  }

  if (!aglSetCurrentContext(osxgsg->get_context())) {
    report_agl_error("aglSetCurrentContext");
    return false;
  }

  osxgsg->reset_if_new();

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

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void osxGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void osxGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    //    aglSetPBuffer(osxgsg->get_context(), _pbuffer, 0, 0, 0);
    _gsg.clear();
  }
  if (_pbuffer != NULL) {
    aglDestroyPBuffer(_pbuffer);
    _pbuffer = NULL;
  }
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the buffer right now.  Called from the window
//               thread.  Returns true if the buffer is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool osxGraphicsBuffer::
open_buffer() {
  if (_gsg == 0) {
    _gsg = new osxGraphicsStateGuardian(_engine, _pipe, NULL);
  }

  if (_pbuffer == NULL) {
    GLenum target = GL_TEXTURE_RECTANGLE_ARB;
    if (_size[0] == Texture::up_to_power_2(_size[0]) &&
        _size[1] == Texture::up_to_power_2(_size[1])) {
      // It's a power-of-two size, so we can use GL_TEXTURE_2D as the
      // target.  Dunno, but maybe this will be more likely to work on
      // some hardware.
      target = GL_TEXTURE_2D;
    }
    if (!aglCreatePBuffer(_size.get_x(), _size.get_y(), target, GL_RGBA, 0, &_pbuffer)) {
      report_agl_error("aglCreatePBuffer");
      close_buffer();
      return false;
    }
  }

  osxGraphicsStateGuardian *osxgsg;
  DCAST_INTO_R(osxgsg, _gsg, false);

  OSStatus stat = osxgsg->build_gl(false, true, _fb_properties);
  if (stat != noErr) {
    return false;
  }

  if (!aglSetPBuffer(osxgsg->get_context(), _pbuffer, 0, 0, 0)) {
    report_agl_error("aglSetPBuffer");
    close_buffer();
    return false;
  }

  if (!aglSetCurrentContext(osxgsg->get_context())) {
    report_agl_error("aglSetCurrentContext");
    return false;
  }

  osxgsg->reset_if_new();
  if (!osxgsg->is_valid()) {
    close_buffer();
    return false;
  }

  /*
  if (!osxgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, osxgsg->get_gl_renderer())) {
    close_buffer();
    return false;
  }
  _fb_properties = osxgsg->get_fb_properties();
  */
  
  _is_valid = true;
  return true;
}

