// Filename: glGraphicsBuffer_src.cxx
// Created by:  jyelon (15Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//

TypeHandle CLP(GraphicsBuffer)::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
glGraphicsBuffer::
glGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name,
                  int x_size, int y_size) :
  GraphicsBuffer(pipe, gsg, name, x_size, y_size) 
{
  _pbuffer = (HPBUFFERARB)0;
  _pbuffer_dc = (HDC)0;

  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
glGraphicsBuffer::
~glGraphicsBuffer() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool glGraphicsBuffer::
begin_frame() {
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);

  if (_pbuffer_dc) {
    int flag = 0;
    glgsg->_glQueryPbufferARB(_pbuffer, GL_PBUFFER_LOST_ARB, &flag);
    if (flag != 0) {
      // The pbuffer was lost, due to a mode change or something
      // silly like that.  We must therefore recreate the pbuffer.
      close_buffer();
      if (!open_buffer()) {
        return false;
      }
    }
  }

  return GraphicsBuffer::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsStateGuardian::begin_render_texture
//       Access: Public, Virtual
//  Description: If the GraphicsOutput supports direct render-to-texture,
//               and if any setup needs to be done during begin_frame,
//               then the setup code should go here.  Any textures that
//               can not be rendered to directly should be reflagged
//               as RTM_copy_texture.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
begin_render_texture() {
  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_V(glgsg, _gsg);
  
  if (_gsg->get_properties().is_single_buffered()) {
    glgsg->_glReleaseTexImageARB(_pbuffer, GL_FRONT_LEFT_ARB);
  } else {
    glgsg->_glReleaseTexImageARB(_pbuffer, GL_BACK_LEFT_ARB);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_render_texture
//       Access: Public, Virtual
//  Description: If the GraphicsOutput supports direct render-to-texture,
//               and if any setup needs to be done during end_frame,
//               then the setup code should go here.  Any textures that
//               could not be rendered to directly should be reflagged
//               as RTM_copy_texture.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
end_render_texture() {
  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  // Find the color texture, if there is one. That one can be bound to
  // the framebuffer.  All others must be marked RTM_copy_to_texture.
  int tex_index = -1;
  for (int i=0; i<count_textures(); i++) {
    if (get_rtm_mode(i) == RTM_bind_or_copy) {
      if ((get_texture(i)->get_format() != Texture::F_depth_component)&&
          (get_texture(i)->get_format() != Texture::F_stencil_index)&&
          (tex_index < 0)) {
        tex_index = i;
      } else {
        _textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
  }
  
  if (tex_index >= 0) {
    Texture *tex = get_texture(tex_index);
    TextureContext *tc = tex->prepare_now(_gsg->get_prepared_objects(), _gsg);
    nassertv(tc != (TextureContext *)NULL);
    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
    GLenum target = glgsg->get_texture_target(tex->get_texture_type());
    if (target == GL_NONE) {
      _textures[tex_index]._rtm_mode = RTM_copy_texture;
      return;
    }
    GLP(BindTexture)(target, gtc->_index);
    if (_gsg->get_properties().is_single_buffered()) {
      glgsg->_glBindTexImageARB(_pbuffer, GL_FRONT_LEFT_ARB);
    } else {
      glgsg->_glBindTexImageARB(_pbuffer, GL_BACK_LEFT_ARB);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::select_cube_map
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map.  This should
//               do whatever needs to be done to switch the buffer to
//               the indicated face.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
select_cube_map(int cube_map_index) {
  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  nassertv(glgsg->_glSetPbufferAttribARB != NULL);

  static const int max_attrib_list = 64;
  int iattrib_list[max_attrib_list];
  int ni = 0;

  iattrib_list[ni++] = GL_CUBE_MAP_FACE_ARB;
  iattrib_list[ni++] = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + cube_map_index;

  // Terminate the list.
  nassertv(ni <= max_attrib_list);
  iattrib_list[ni] = 0;

  glgsg->_glSetPbufferAttribARB(_pbuffer, iattrib_list);
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
make_current() {
  PStatTimer timer(_make_current_pcollector);

  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  glMakeCurrent(_pbuffer_dc, glgsg->get_context(_pbuffer_dc));
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
release_gsg() {
  GraphicsBuffer::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
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
//     Function: glGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    glGraphicsStateGuardian *glgsg;
    DCAST_INTO_V(glgsg, _gsg);

    if (_pbuffer_dc) {
      glgsg->_glReleasePbufferDCARB(_pbuffer, _pbuffer_dc);
    }
    if (_pbuffer) {
      glgsg->_glDestroyPbufferARB(_pbuffer);
    }
  }
  _pbuffer_dc = 0;
  _pbuffer = 0;

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool glGraphicsBuffer::
open_buffer() {
  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);

  HDC twindow_dc = glgsg->get_twindow_dc();
  if (twindow_dc == 0) {
    // If we couldn't make a window, we can't get a GL context.
    return false;
  }

  glMakeCurrent(twindow_dc, glgsg->get_context(twindow_dc));
  glgsg->reset_if_new();
  _needs_context = false;

  // Now that we have fully made a window and used that window to
  // create a rendering context, we can attempt to create a pbuffer.
  // This might fail if the pbuffer extensions are not supported.

  if (!make_pbuffer(twindow_dc)) {
    glMakeCurrent(0, 0);
    return false;
  }

  _pbuffer_dc = glgsg->_glGetPbufferDCARB(_pbuffer);
  
  glMakeCurrent(_pbuffer_dc, glgsg->get_context(_pbuffer_dc));
  glgsg->report_my_gl_errors();
  
  _is_valid = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::make_pbuffer
//       Access: Private
//  Description: Once the GL context has been fully realized, attempts
//               to create an offscreen pbuffer if the graphics API
//               supports it.  Returns true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool glGraphicsBuffer::
make_pbuffer(HDC twindow_dc) {
  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);

  if (!glgsg->_supports_pbuffer) {
    gldisplay_cat.info()
      << "PBuffers not supported by GL implementation.\n";
    return false;
  }

  int pbformat = glgsg->get_pfnum();

  if (glgsg->_supports_pixel_format) {
    bool got_pbuffer_format = false;
    bool any_binds = false;
    
    for (int i=0; i<count_textures(); i++) {
      if (get_rtm_mode(i) == RTM_bind_or_copy) {
        any_binds = true;
      }
    }

    if (any_binds && glgsg->_supports_render_texture) {
      // First, try to get a pbuffer format that supports
      // render-to-texture.
      int new_pbformat = choose_pbuffer_format(twindow_dc, true);
      if (new_pbformat != 0) {
        pbformat = new_pbformat;
        got_pbuffer_format = true;
      }
    }

    if (!got_pbuffer_format) {
      // Failing that, just get a matching pbuffer format,
      // and disable RTM_bind_or_copy.
      for (int i=0; i<count_textures(); i++) {
        if (get_rtm_mode(i) == RTM_bind_or_copy) {
          _textures[i]._rtm_mode = RTM_copy_texture;
        }
      }
      int new_pbformat = choose_pbuffer_format(twindow_dc, false);
      if (new_pbformat != 0) {
        pbformat = new_pbformat;
        got_pbuffer_format = true;
      }
    }

    if (gldisplay_cat.is_debug()) {
      FrameBufferProperties properties;
      glGraphicsPipe::get_properties_advanced(properties, glgsg, 
                                               twindow_dc, pbformat);
      gldisplay_cat.debug()
        << "Chose pixfmt #" << pbformat << " for pbuffer = " 
        << properties << "\n";
    }
  }

  static const int max_attrib_list = 64;
  int iattrib_list[max_attrib_list];
  int ni = 0;

  // Find the texture to bind to the color buffer.
  Texture *bindtexture = NULL;
  for (int i=0; i<count_textures(); i++) {
    if ((get_rtm_mode(i) == RTM_bind_or_copy)&&
        (get_texture(i)->get_format() != Texture::F_depth_component)&&
        (get_texture(i)->get_format() != Texture::F_stencil_index)) {
      bindtexture = get_texture(i);
      break;
    }
  }
  
  if (bindtexture != 0) {

    if (_gsg->get_properties().get_frame_buffer_mode() & FrameBufferProperties::FM_alpha) {
      iattrib_list[ni++] = GL_TEXTURE_FORMAT_ARB;
      iattrib_list[ni++] = GL_TEXTURE_RGBA_ARB;
    } else {
      iattrib_list[ni++] = GL_TEXTURE_FORMAT_ARB;
      iattrib_list[ni++] = GL_TEXTURE_RGB_ARB;
    }

    if (bindtexture->uses_mipmaps()) {
      iattrib_list[ni++] = GL_MIPMAP_TEXTURE_ARB;
      iattrib_list[ni++] = 1;
    }

    switch (bindtexture->get_texture_type()) {
    case Texture::TT_cube_map:
      iattrib_list[ni++] = GL_TEXTURE_TARGET_ARB;
      iattrib_list[ni++] = GL_TEXTURE_CUBE_MAP_ARB;
      break;

    case Texture::TT_1d_texture:
      iattrib_list[ni++] = GL_TEXTURE_TARGET_ARB;
      iattrib_list[ni++] = GL_TEXTURE_1D_ARB;
      break;

    default:
      iattrib_list[ni++] = GL_TEXTURE_TARGET_ARB;
      iattrib_list[ni++] = GL_TEXTURE_2D_ARB;
    }
  }  

  // Terminate the list.
  nassertr(ni <= max_attrib_list, false);
  iattrib_list[ni] = 0;
  
  _pbuffer = glgsg->_glCreatePbufferARB(twindow_dc, pbformat, 
                                          _x_size, _y_size, iattrib_list);

  if (_pbuffer == 0) {
    gldisplay_cat.info()
      << "Attempt to create pbuffer failed.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::choose_pbuffer_format
//       Access: Private
//  Description: Select a suitable pixel format that matches the GSG's
//               existing format, and also is appropriate for a pixel
//               buffer.  Returns the selected pfnum if successful, or
//               0 on failure.
////////////////////////////////////////////////////////////////////
int glGraphicsBuffer::
choose_pbuffer_format(HDC twindow_dc, bool draw_to_texture) {
  if (gldisplay_cat.is_debug()) {
    gldisplay_cat.debug()
      << "choose_pbuffer_format(twindow_dc, draw_to_texture = " 
      << draw_to_texture << ")\n";
  }

  glGraphicsStateGuardian *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);

  int pbformat = glgsg->get_pfnum();

  static const int max_attrib_list = 64;
  int iattrib_list[max_attrib_list];
  int ivalue_list[max_attrib_list];
  int ni = 0;
  
  int acceleration_i, pixel_type_i, double_buffer_i, stereo_i,
    red_bits_i, green_bits_i, blue_bits_i, alpha_bits_i, 
    accum_red_bits_i, accum_green_bits_i, accum_blue_bits_i,
    accum_alpha_bits_i, depth_bits_i, 
    stencil_bits_i, sample_buffers_i, multisamples_i;
  
  iattrib_list[acceleration_i = ni++] = GL_ACCELERATION_ARB;
  iattrib_list[pixel_type_i = ni++] = GL_PIXEL_TYPE_ARB;
  iattrib_list[double_buffer_i = ni++] = GL_DOUBLE_BUFFER_ARB;
  iattrib_list[stereo_i = ni++] = GL_STEREO_ARB;
  iattrib_list[red_bits_i = ni++] = GL_RED_BITS_ARB;
  iattrib_list[green_bits_i = ni++] = GL_GREEN_BITS_ARB;
  iattrib_list[blue_bits_i = ni++] = GL_BLUE_BITS_ARB;
  iattrib_list[alpha_bits_i = ni++] = GL_ALPHA_BITS_ARB;
  iattrib_list[accum_red_bits_i = ni++] = GL_ACCUM_RED_BITS_ARB;
  iattrib_list[accum_green_bits_i = ni++] = GL_ACCUM_GREEN_BITS_ARB;
  iattrib_list[accum_blue_bits_i = ni++] = GL_ACCUM_BLUE_BITS_ARB;
  iattrib_list[accum_alpha_bits_i = ni++] = GL_ACCUM_ALPHA_BITS_ARB;
  iattrib_list[depth_bits_i = ni++] = GL_DEPTH_BITS_ARB;
  iattrib_list[stencil_bits_i = ni++] = GL_STENCIL_BITS_ARB;
  
  if (glgsg->_supports_gl_multisample) {
    iattrib_list[sample_buffers_i = ni++] = GL_SAMPLE_BUFFERS_ARB;
    iattrib_list[multisamples_i = ni++] = GL_SAMPLES_ARB;
  }
  
  // Terminate the list.
  nassertr(ni <= max_attrib_list, false);
  
  if (!glgsg->_glGetPixelFormatAttribivARB(twindow_dc, pbformat, 0,
                                             ni, iattrib_list, ivalue_list)) {
    if (gldisplay_cat.is_debug()) {
      gldisplay_cat.debug()
        << "Could not query old format " << pbformat << ".\n";
    }
    return 0;
  }
  
  ni = 0;
  float fattrib_list[max_attrib_list];
  int nf = 0;
  
  // Since we are trying to create a pbuffer, the pixel format we
  // request (and subsequently use) must be "pbuffer capable".
  iattrib_list[ni++] = GL_DRAW_TO_PBUFFER_ARB;
  iattrib_list[ni++] = true;
  iattrib_list[ni++] = GL_SUPPORT_OPENGL_ARB;
  iattrib_list[ni++] = true;

  if (draw_to_texture) {
    // If we want to be able to render-to-texture, request that.
    if (_gsg->get_properties().get_frame_buffer_mode() & FrameBufferProperties::FM_alpha) {
      iattrib_list[ni++] = GL_BIND_TO_TEXTURE_RGBA_ARB;
      iattrib_list[ni++] = true;
    } else {
      iattrib_list[ni++] = GL_BIND_TO_TEXTURE_RGB_ARB;
      iattrib_list[ni++] = true;
    }
  }    
  
  // Match up the framebuffer bits.
  iattrib_list[ni++] = GL_RED_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[red_bits_i];
  iattrib_list[ni++] = GL_GREEN_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[green_bits_i];
  iattrib_list[ni++] = GL_BLUE_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[blue_bits_i];
  iattrib_list[ni++] = GL_ALPHA_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[alpha_bits_i];
  
  iattrib_list[ni++] = GL_ACCUM_RED_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[accum_red_bits_i];
  iattrib_list[ni++] = GL_ACCUM_GREEN_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[accum_green_bits_i];
  iattrib_list[ni++] = GL_ACCUM_BLUE_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[accum_blue_bits_i];
  iattrib_list[ni++] = GL_ACCUM_ALPHA_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[accum_alpha_bits_i];
  
  iattrib_list[ni++] = GL_DEPTH_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[depth_bits_i];
  
  iattrib_list[ni++] = GL_STENCIL_BITS_ARB;
  iattrib_list[ni++] = ivalue_list[stencil_bits_i];
  
  if (glgsg->_supports_gl_multisample) {
    iattrib_list[ni++] = GL_SAMPLE_BUFFERS_ARB;
    iattrib_list[ni++] = ivalue_list[sample_buffers_i];
    iattrib_list[ni++] = GL_SAMPLES_ARB;
    iattrib_list[ni++] = ivalue_list[multisamples_i];
  }
  
  // Match up properties.
  iattrib_list[ni++] = GL_DOUBLE_BUFFER_ARB;
  iattrib_list[ni++] = ivalue_list[double_buffer_i];
  iattrib_list[ni++] = GL_STEREO_ARB;
  iattrib_list[ni++] = ivalue_list[stereo_i];
  
  // Terminate the lists.
  nassertr(ni < max_attrib_list && nf < max_attrib_list, NULL);
  iattrib_list[ni] = 0;
  fattrib_list[nf] = 0;
  
  // Now obtain a list of pixel formats that meet these minimum
  // requirements.
  static const unsigned int max_pformats = 32;
  int pformat[max_pformats];
  memset(pformat, 0, sizeof(pformat));
  unsigned int nformats = 0;
  if (!glgsg->_glChoosePixelFormatARB(twindow_dc, iattrib_list, fattrib_list,
                                        max_pformats, pformat, &nformats)
      || nformats == 0) {
    if (gldisplay_cat.is_debug()) {
      gldisplay_cat.debug()
        << "No formats meet the criteria.\n";
    }
    return 0;
  }
  
  nformats = min(nformats, max_pformats);
  
  if (gldisplay_cat.is_debug()) {
    gldisplay_cat.debug()
      << "Found " << nformats << " pbuffer formats: [";
    for (unsigned int i = 0; i < nformats; i++) {
      gldisplay_cat.debug(false)
        << " " << pformat[i];
    }
    gldisplay_cat.debug(false)
      << " ]\n";
  }
  
  // If one of the options is the original pixfmt, keep it.
  bool found_pbformat = false;
  for (unsigned int i = 0; i < nformats && !found_pbformat; i++) {
    if (pformat[i] == pbformat) {
      found_pbformat = true;
    }
  }
  
  if (!found_pbformat) {
    // Otherwise, pick any of them.
    pbformat = pformat[0];
  }

  return pbformat;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void glGraphicsBuffer::
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


