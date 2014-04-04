// Filename: wglGraphicsBuffer.cxx
// Created by:  drose (08Feb04)
//
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

#include "wglGraphicsBuffer.h"
#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include "glgsg.h"
#include "pStatTimer.h"

#include <wingdi.h>

TypeHandle wglGraphicsBuffer::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsBuffer::
wglGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _pbuffer = (HPBUFFERARB)0;
  _pbuffer_dc = (HDC)0;
  release_pbuffer();
  
  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsBuffer::
~wglGraphicsBuffer() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  HGLRC context = wglgsg->get_context(_pbuffer_dc);
  if (context == 0) {
    return false;
  }

  if (_fb_properties.is_single_buffered()) {
    wglgsg->_wglReleaseTexImageARB(_pbuffer, WGL_FRONT_LEFT_ARB);
  } else {
    wglgsg->_wglReleaseTexImageARB(_pbuffer, WGL_BACK_LEFT_ARB);
  }

  if (!rebuild_bitplanes()) {
    wglGraphicsPipe::wgl_make_current(0, 0, &_make_current_pcollector);
    return false;
  }
  
  wglGraphicsPipe::wgl_make_current(_pbuffer_dc, context,
                                    &_make_current_pcollector);
  
  if (mode == FM_render) {
    CDLockedReader cdata(_cycler);
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      RenderTexturePlane plane = rt._plane;
      if (rtm_mode == RTM_bind_or_copy && plane != RTP_color) {
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
//     Function: wglGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    copy_to_textures();
    bind_texture_to_pbuffer();
  }
  
  _gsg->end_frame(current_thread);
  
  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::bind_texture_to_pbuffer
//       Access: Private
//  Description: Looks for the appropriate texture,
//               and binds that texture to the pbuffer.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
bind_texture_to_pbuffer() {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_V(wglgsg, _gsg);

  // Find the color texture, if there is one. That one can be bound to
  // the framebuffer.  All others must be marked RTM_copy_to_texture.

  int tex_index = -1;
  CDLockedReader cdata(_cycler);
  for (size_t i = 0; i != cdata->_textures.size(); ++i) {
    const RenderTexture &rt = cdata->_textures[i];
    RenderTexturePlane plane = rt._plane;
    if (plane == RTP_color) {
      tex_index = i;
      break;
    }
  }

  if (tex_index >= 0) {
    const RenderTexture &rt = cdata->_textures[tex_index];
    Texture *tex = rt._texture;
    if ((_pbuffer_bound != 0)&&(_pbuffer_bound != tex)) {
      _pbuffer_bound->release(wglgsg->get_prepared_objects());
      _pbuffer_bound = 0;
    }
    tex->set_size_padded(_x_size, _y_size);
    if (tex->get_match_framebuffer_format()) {
      if (_fb_properties.get_alpha_bits()) {
        tex->set_format(Texture::F_rgba);
      } else {
        tex->set_format(Texture::F_rgb);
      }
    }
    TextureContext *tc = tex->prepare_now(0, _gsg->get_prepared_objects(), _gsg);
    nassertv(tc != (TextureContext *)NULL);
    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
    GLenum target = wglgsg->get_texture_target(tex->get_texture_type());
    if (target == GL_NONE) {
      CDWriter cdataw(_cycler, cdata, false);
      nassertv(cdata->_textures.size() == cdataw->_textures.size());
      cdataw->_textures[tex_index]._rtm_mode = RTM_copy_texture;
      return;
    }
    GLP(BindTexture)(target, gtc->_index);
    if (_fb_properties.is_single_buffered()) {
      wglgsg->_wglBindTexImageARB(_pbuffer, WGL_FRONT_LEFT_ARB);
    } else {
      wglgsg->_wglBindTexImageARB(_pbuffer, WGL_BACK_LEFT_ARB);
    }
    _pbuffer_bound = tex;
  } else {
    if (_pbuffer_bound != 0) {
      _pbuffer_bound->release(wglgsg->get_prepared_objects());
      _pbuffer_bound = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::select_target_tex_page
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map.  This should
//               do whatever needs to be done to switch the buffer to
//               the indicated face.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
select_target_tex_page(int page) {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_V(wglgsg, _gsg);

  nassertv(wglgsg->_wglSetPbufferAttribARB != NULL);

  static const int max_attrib_list = 64;
  int iattrib_list[max_attrib_list];
  int ni = 0;

  iattrib_list[ni++] = WGL_CUBE_MAP_FACE_ARB;
  iattrib_list[ni++] = WGL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + page;

  // Terminate the list.
  nassertv(ni <= max_attrib_list);
  iattrib_list[ni] = 0;

  wglgsg->_wglSetPbufferAttribARB(_pbuffer, iattrib_list);
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
process_events() {
  GraphicsBuffer::process_events();

  MSG msg;
    
  // Handle all the messages on the queue in a row.  Some of these
  // might be for another window, but they will get dispatched
  // appropriately.
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    process_1_event();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::get_supports_render_texture
//       Access: Published, Virtual
//  Description: Returns true if this particular GraphicsOutput can
//               render directly into a texture, or false if it must
//               always copy-to-texture at the end of each frame to
//               achieve this effect.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
get_supports_render_texture() const {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);
  return wglgsg->get_supports_wgl_render_texture();
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    wglGraphicsStateGuardian *wglgsg;
    DCAST_INTO_V(wglgsg, _gsg);

    _gsg.clear();
  }
  
  release_pbuffer();
  
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
open_buffer() {

  // pbuffers don't seem to work correctly in double-buffered
  // mode. Besides, the back buffer is a pointless waste of space.  
  // So always use a single-buffered gsg.
  
  _fb_properties.set_back_buffers(0);
  _draw_buffer_type = RenderBuffer::T_front;
  _screenshot_buffer_type = RenderBuffer::T_front;
  
  // GSG creation/initialization.

  wglGraphicsStateGuardian *wglgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    wglgsg = new wglGraphicsStateGuardian(_engine, _pipe, NULL);
    wglgsg->choose_pixel_format(_fb_properties, true);
    _gsg = wglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a
    // new one that shares with the old gsg.
    DCAST_INTO_R(wglgsg, _gsg, false);
    if ((!wglgsg->get_fb_properties().subsumes(_fb_properties))||
        (!wglgsg->get_fb_properties().is_single_buffered())||
        (!wglgsg->pfnum_supports_pbuffer())) {
      wglgsg = new wglGraphicsStateGuardian(_engine, _pipe, wglgsg);
      wglgsg->choose_pixel_format(_fb_properties, true);
      _gsg = wglgsg;
    }
  }
  
  // Use the temp window to initialize the gsg.
  
  HDC twindow_dc = wglgsg->get_twindow_dc();
  if (twindow_dc == 0) {
    // If we couldn't make a window, we can't get a GL context.
    _gsg = NULL;
    return false;
  }
  HGLRC context = wglgsg->get_context(twindow_dc);
  if (context == 0) {
    _gsg = NULL;
    return false;
  }
  wglGraphicsPipe::wgl_make_current(twindow_dc, context,
                                    &_make_current_pcollector);
  wglgsg->reset_if_new();
  wglgsg->report_my_gl_errors();
  if (!wglgsg->get_fb_properties().verify_hardware_software
      (_fb_properties,wglgsg->get_gl_renderer())) {
    _gsg = NULL;
    return false;
  }
  _fb_properties = wglgsg->get_fb_properties();
  
  // Now that we have fully made a window and used that window to
  // create a rendering context, we can attempt to create a pbuffer.
  // This might fail if the pbuffer extensions are not supported.

  if (!rebuild_bitplanes()) {
    wglGraphicsPipe::wgl_make_current(0, 0, &_make_current_pcollector);
    _gsg = NULL;
    return false;
  }
  
  _is_valid = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::release_pbuffer
//       Access: Private
//  Description: Destroys the pbuffer if it has been created.  The
//               intent is that this may allow it to be recreated
//               with different options.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
release_pbuffer() {
  if (_gsg == 0) {
    return;
  }
  
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_V(wglgsg, _gsg);

  if (_pbuffer_bound != 0) {
    _pbuffer_bound->release(wglgsg->get_prepared_objects());
    _pbuffer_bound = 0;
  }
  wglGraphicsPipe::wgl_make_current(0, 0, NULL);
  if (_pbuffer_dc) {
    wglgsg->_wglReleasePbufferDCARB(_pbuffer, _pbuffer_dc);
  }
  if (_pbuffer) {
    wglgsg->_wglDestroyPbufferARB(_pbuffer);
  }
  _pbuffer = (HPBUFFERARB)0;
  _pbuffer_dc = (HDC)0;
  _pbuffer_mipmap = false;
  _pbuffer_sizex = 0;
  _pbuffer_sizey = 0;
  _pbuffer_type = Texture::TT_2d_texture;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::rebuild_bitplanes
//       Access: Private
//  Description: Once the GL context has been fully realized, attempts
//               to create an offscreen pbuffer if the graphics API
//               supports it.  Returns true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool wglGraphicsBuffer::
rebuild_bitplanes() {
  wglGraphicsStateGuardian *wglgsg;
  DCAST_INTO_R(wglgsg, _gsg, false);

  if (!wglgsg->_supports_pbuffer) {
    wgldisplay_cat.info()
      << "PBuffers not supported by GL implementation.\n";
    return false;
  }

  // Find the texture to bind to the color buffer.
  Texture *bindtexture = NULL;
  for (int i=0; i<count_textures(); i++) {
    if ((get_rtm_mode(i) == RTM_bind_or_copy)&&
        (get_texture(i)->get_format() != Texture::F_depth_stencil)) {
      bindtexture = get_texture(i);
      break;
    }
  }

  // If we already have a pbuffer, and if it's lost, then 
  // force the rebuild.

  if (_pbuffer_dc) {
    int flag = 0;
    wglgsg->_wglQueryPbufferARB(_pbuffer, WGL_PBUFFER_LOST_ARB, &flag);
    if (flag != 0) {
      release_pbuffer();
    }
  }
  
  // Determine what pbuffer attributes are needed
  // for currently-applicable textures.

  if ((_host != 0)&&(_creation_flags & GraphicsPipe::BF_size_track_host)) {
    if ((_host->get_x_size() != _x_size)||
        (_host->get_y_size() != _y_size)) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  }
  int desired_x = _x_size;
  int desired_y = _y_size;
  if ((bindtexture != 0)&&(Texture::get_textures_power_2() != ATS_none)) {
    desired_x = Texture::up_to_power_2(desired_x);
    desired_y = Texture::up_to_power_2(desired_y);
  }
  bool desired_mipmap = false;
  Texture::TextureType desired_type = Texture::TT_2d_texture;
  if (bindtexture != 0) {
    desired_mipmap = bindtexture->uses_mipmaps();
    desired_type = bindtexture->get_texture_type();
  }

  if ((_pbuffer != 0)&&
      (_pbuffer_sizex == desired_x)&&
      (_pbuffer_sizey == desired_y)&&
      (_pbuffer_mipmap == desired_mipmap)&&
      (_pbuffer_type == desired_type)) {
    // the pbuffer we already have is fine. Do not rebuild.
    return true;
  }

  // Release the old pbuffer, if there was one.
  
  release_pbuffer();

  // Allocate the new pbuffer.

  int pfnum = wglgsg->get_pfnum();

  static const int max_attrib_list = 64;
  int iattrib_list[max_attrib_list];
  int ni = 0;
  
  if (_fb_properties.get_alpha_bits()) {
    iattrib_list[ni++] = WGL_TEXTURE_FORMAT_ARB;
    iattrib_list[ni++] = WGL_TEXTURE_RGBA_ARB;
  } else {
    iattrib_list[ni++] = WGL_TEXTURE_FORMAT_ARB;
    iattrib_list[ni++] = WGL_TEXTURE_RGB_ARB;
  }

  if (desired_mipmap) {
    iattrib_list[ni++] = WGL_MIPMAP_TEXTURE_ARB;
    iattrib_list[ni++] = 1;
  }

  switch (desired_type) {
  case Texture::TT_cube_map:
    iattrib_list[ni++] = WGL_TEXTURE_TARGET_ARB;
    iattrib_list[ni++] = WGL_TEXTURE_CUBE_MAP_ARB;
    break;
    
  case Texture::TT_1d_texture:
    iattrib_list[ni++] = WGL_TEXTURE_TARGET_ARB;
    iattrib_list[ni++] = WGL_TEXTURE_1D_ARB;
    break;
    
  default:
    iattrib_list[ni++] = WGL_TEXTURE_TARGET_ARB;
    iattrib_list[ni++] = WGL_TEXTURE_2D_ARB;
  }
  
  // Terminate the list.
  nassertr(ni <= max_attrib_list, false);
  iattrib_list[ni] = 0;

  HDC twindow_dc = wglgsg->get_twindow_dc();
  if (twindow_dc == 0) {
    return false;
  }
  
  HGLRC context = wglgsg->get_context(twindow_dc);
  if (context == 0) {
    return false;
  }
  wglGraphicsPipe::wgl_make_current(twindow_dc, context,
                                    &_make_current_pcollector);

  _pbuffer = wglgsg->_wglCreatePbufferARB(twindow_dc, pfnum, 
                                          desired_x, desired_y, iattrib_list);
  
  if (_pbuffer == 0) {
    wgldisplay_cat.info()
      << "Attempt to create pbuffer failed.\n";
    return false;
  }

  _pbuffer_dc = wglgsg->_wglGetPbufferDCARB(_pbuffer);
  _pbuffer_mipmap = desired_mipmap;
  _pbuffer_type = desired_type;
  _pbuffer_sizex = desired_x;
  _pbuffer_sizey = desired_y;
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsBuffer::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void wglGraphicsBuffer::
process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0)) {
    // WM_QUIT received.  We need a cleaner way to deal with this.
    //    DestroyAllWindows(false);
    exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}


