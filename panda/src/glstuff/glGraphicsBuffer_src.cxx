// Filename: glGraphicsBuffer_src.cxx
// Created by:  jyelon (15Jan06)
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

TypeHandle CLP(GraphicsBuffer)::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsBuffer)::
CLP(GraphicsBuffer)(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  // An FBO doesn't have a back buffer.
  _draw_buffer_type       = RenderBuffer::T_front;
  _screenshot_buffer_type = RenderBuffer::T_front;

  // Initialize these.
  _fbo = 0;
  _rb_size_x = 0;
  _rb_size_y = 0;
  _cube_face_active = 0;
  for (int i=0; i<RTP_COUNT; i++) {
    _rb[i] = 0;
    _tex[i] = 0;
  }

  for (int f = 0; f < 6; f++) {
    _cubemap_fbo [f] = 0;
  }

  _shared_depth_buffer = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsBuffer)::
~CLP(GraphicsBuffer)() {
  // unshare shared depth buffer if any
  this -> unshare_depth_buffer();  

  // unshare all buffers that are sharing this object's depth buffer
  {
    CLP(GraphicsBuffer) *graphics_buffer;
    list <CLP(GraphicsBuffer) *>::iterator graphics_buffer_iterator;

    graphics_buffer_iterator = _shared_depth_buffer_list.begin( );
    while (graphics_buffer_iterator != _shared_depth_buffer_list.end( )) {
      graphics_buffer = (*graphics_buffer_iterator);
      if (graphics_buffer) {      
        // this call removes the entry from the list
        graphics_buffer -> unshare_depth_buffer();
      }      
      graphics_buffer_iterator = _shared_depth_buffer_list.begin( );
    }
  }
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
bool CLP(GraphicsBuffer)::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);

  check_host_valid();
  
  if (!_is_valid) {
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << get_name() << " is not valid\n";
    }
    return false;
  }

  if (!_host->begin_frame(FM_parasite, current_thread)) {
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << get_name() << "'s host is not ready\n";
    }
    return false;
  }
  
  // Figure out the desired size of the buffer.
  if (mode == FM_render) {
    rebuild_bitplanes();
    clear_cube_map_selection();
    if (!check_fbo()) {
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << get_name() << " check_fbo() returns false\n";
      }
      return false;
    }
  }

  _gsg->set_current_properties(&get_fb_properties());
  report_my_gl_errors();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::check_fbo
//       Access: Private
//  Description: Calls 'glCheckFramebufferStatus'.  On error, 
//               prints out an appropriate error message and unbinds
//               the fbo.  Returns true for OK or false for error.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsBuffer)::
check_fbo() {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);

  GLenum status = glgsg->_glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
  if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
    GLCAT.error() << "EXT_framebuffer_object reports non-framebuffer-completeness.\n";
    switch(status) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n"; break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      GLCAT.error() << "FRAMEBUFFER_UNSUPPORTED_EXT\n"; break;
    default:
      GLCAT.error() << "OTHER PROBLEM\n"; break;
    }
    
    glgsg->bind_fbo(0);
    report_my_gl_errors();
    return false;
  }
  report_my_gl_errors();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::rebuild_bitplanes
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               to allocate/reallocate the fbo and all the associated
//               renderbuffers, just before rendering a frame.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
rebuild_bitplanes() {
  
  check_host_valid();
  if (_gsg == 0) {
    return;
  }
  
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  // Bind the FBO

  Texture *tex = get_texture(0);
  if (tex == 0) {
    return;
  }
  
  if (tex->get_texture_type() != Texture::TT_cube_map) {
    if (_fbo == 0) {
      glgsg->_glGenFramebuffers(1, &_fbo);
      if (_fbo == 0) {
        report_my_gl_errors();
        return;
      }
    }
    glgsg->bind_fbo(_fbo);

    // Calculate bitplane size.  This can be larger than the buffer.

    if (_creation_flags & GraphicsPipe::BF_size_track_host) {
      if ((_host->get_x_size() != _x_size)||
          (_host->get_y_size() != _y_size)) {
        set_size_and_recalc(_host->get_x_size(),
                            _host->get_y_size());
      }
    }
    int bitplane_x = _x_size;
    int bitplane_y = _y_size;
    if (Texture::get_textures_power_2() != ATS_none) {
      bitplane_x = Texture::up_to_power_2(bitplane_x);
      bitplane_y = Texture::up_to_power_2(bitplane_y);
    }
    bool rb_resize = false;
    if ((bitplane_x != _rb_size_x)||
        (bitplane_y != _rb_size_y)) {
      _rb_size_x = bitplane_x;
      _rb_size_y = bitplane_y;
      rb_resize = true;
    }

    // These variables indicate what should be bound to each bitplane.

    Texture *attach[RTP_COUNT];
    attach[RTP_color] = 0;
    attach[RTP_depth] = 0;
    attach[RTP_depth_stencil] = 0;
    for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
      attach[RTP_aux_rgba_0+i] = 0;
    }
    for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
      attach[RTP_aux_hrgba_0+i] = 0;
    }
    for (int i=0; i<_fb_properties.get_aux_float(); i++) {
      attach[RTP_aux_float_0+i] = 0;
    }

    // Sort the textures list into appropriate slots.

    for (int i=0; i<count_textures(); i++) {
      if (get_rtm_mode(i) != RTM_bind_or_copy) {
        continue;
      }
      Texture *tex = get_texture(i);
      RenderTexturePlane plane = get_texture_plane(i);

      // If it's a not a 2D texture or a cube map, punt it.
      if ((tex->get_texture_type() != Texture::TT_2d_texture)&&
          (tex->get_texture_type() != Texture::TT_cube_map)) {
        _textures[i]._rtm_mode = RTM_copy_texture;
        continue;
      }

      // If I can't find an appropriate slot, or if there's
      // already a texture bound to this slot, then punt
      // this texture.  

      if (attach[plane]) {
        _textures[i]._rtm_mode = RTM_copy_texture;
        continue;
      }
      // Assign the texture to this slot.
      attach[plane] = tex;
    }

    // For all slots, update the slot.

    bind_slot(rb_resize, attach, RTP_depth, GL_DEPTH_ATTACHMENT_EXT);
    bind_slot(rb_resize, attach, RTP_depth_stencil, GL_DEPTH_ATTACHMENT_EXT);
    bind_slot(rb_resize, attach, RTP_color, GL_COLOR_ATTACHMENT0_EXT);
    int next = GL_COLOR_ATTACHMENT1_EXT;
    for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
      bind_slot(rb_resize, attach, (RenderTexturePlane)(RTP_aux_rgba_0+i), next);
      next += 1;
    }
    for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
      bind_slot(rb_resize, attach, (RenderTexturePlane)(RTP_aux_hrgba_0+i), next);
      next += 1;
    }
    for (int i=0; i<_fb_properties.get_aux_float(); i++) {
      bind_slot(rb_resize, attach, (RenderTexturePlane)(RTP_aux_float_0+i), next);
      next += 1;
    }
  }
  else {
    // make an FBO for each cubemap face
    int update;
    
    update = false;
    for (int f = 0; f < 6; f++) {    
      if (_cubemap_fbo [f] == 0) {
        glgsg->_glGenFramebuffers(1, &_cubemap_fbo [f]);
        update = true;
        if (_cubemap_fbo [f] == 0) {
          report_my_gl_errors();
          return;
        }    
      }
    }
    
    if (update) {    
      int color_attachment = GL_COLOR_ATTACHMENT0_EXT;

      for (int i=0; i<count_textures(); i++) {
        // Do we really need the following lines?
        // Uncommenting them seems to break stuff.
        //if (get_rtm_mode(i) != RTM_bind_or_copy) {
        //  continue;
        //}

        Texture *tex = get_texture(i);
        TextureContext *tc = tex->prepare_now(glgsg->get_prepared_objects(), glgsg);
        nassertv(tc != (TextureContext *)NULL);
        CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
        glgsg->update_texture(tc, true);

        RenderTexturePlane plane = get_texture_plane(i);

        switch (plane)
        {            
          case RTP_depth:
          case RTP_depth_stencil:
            // also case RTP_depth_stencil 
            for (int f = 0; f < 6; f++) {    
              glgsg -> bind_fbo(_cubemap_fbo [f]);
              glgsg -> _glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + f,
                                             gtc->_index, 0);
            }
            break;

          case RTP_color:
          case RTP_aux_rgba_0:
          case RTP_aux_rgba_1:
          case RTP_aux_rgba_2:
          case RTP_aux_rgba_3:
          case RTP_aux_hrgba_0:
          case RTP_aux_hrgba_1:
          case RTP_aux_hrgba_2:
          case RTP_aux_hrgba_3:
          case RTP_aux_float_0:
          case RTP_aux_float_1:
          case RTP_aux_float_2:
          case RTP_aux_float_3:
            for (int f = 0; f < 6; f++) {    
              glgsg -> bind_fbo(_cubemap_fbo [f]);
              glgsg -> _glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, color_attachment,
                                             GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + f,
                                             gtc->_index, 0);
            }
            color_attachment++;
            break;
            
          default:
            break;
        }
      }
    }
    
    glgsg -> bind_fbo(_cubemap_fbo [0]);
  }
  
  _cube_face_active = 0;
  
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::bind_slot
//       Access: Private
//  Description: Attaches either a texture or a renderbuffer to the
//               specified bitplane.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
bind_slot(bool rb_resize, Texture **attach, RenderTexturePlane slot, GLenum attachpoint) {
  
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  Texture *tex = attach[slot];
  if (tex) {
    // If the texture is already bound to the slot, and it's
    // the right size, then no update of this slot is needed.
    if ((_tex[slot] == tex)&&
        (tex->get_x_size() == _rb_size_x)&&
        (tex->get_y_size() == _rb_size_y)) {
      tex->set_pad_size(_rb_size_x - _x_size, _rb_size_y - _y_size);
      return;
    }
    
    // Bind the texture to the slot.
    tex->set_x_size(_rb_size_x);
    tex->set_y_size(_rb_size_y);
    tex->set_pad_size(_rb_size_x - _x_size, _rb_size_y - _y_size);
    if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
      tex->set_format(Texture::F_depth_stencil);
      TextureContext *tc = tex->prepare_now(glgsg->get_prepared_objects(), glgsg);
      nassertv(tc != (TextureContext *)NULL);
      CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
      glgsg->update_texture(tc, true);
      if (tex->get_texture_type() == Texture::TT_2d_texture) {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                       GL_TEXTURE_2D, gtc->_index, 0);
      } else {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
                                       gtc->_index, 0);
      }
      if (_gsg->get_supports_depth_stencil()) {
        if (tex->get_texture_type() == Texture::TT_2d_texture) {
          glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                         GL_TEXTURE_2D, gtc->_index, 0);
        } else {
          glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                         GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
                                         gtc->_index, 0);
        }
      }
    } else {
//      tex->set_format(Texture::F_rgba);
      TextureContext *tc = tex->prepare_now(glgsg->get_prepared_objects(), glgsg);
      nassertv(tc != (TextureContext *)NULL);
      CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
      glgsg->update_texture(tc, true);
      if (tex->get_texture_type() == Texture::TT_2d_texture) {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachpoint,
                                       GL_TEXTURE_2D, gtc->_index, 0);
      } else {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachpoint,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
                                       gtc->_index, 0);
      }
    }
    
    _tex[slot] = tex;
    _attach_point[slot] = attachpoint;
    
    // If there was a renderbuffer bound to this slot, delete it.
    if (_rb[slot] != 0) {
      glgsg->_glDeleteRenderbuffers(1, &(_rb[slot]));
      _rb[slot] = 0;
    }
    
  } else {
    
    // If a renderbuffer is already attached to the slot, and it's
    // the right size, then no update of this slot is needed.
    if (_shared_depth_buffer == 0 && (_rb[slot] != 0)&&(!rb_resize)) {
      return;
    }
    
    // If there's no renderbuffer for this slot, create one.
    if (_rb[slot] == 0) {
      glgsg->_glGenRenderbuffers(1, &(_rb[slot]));
    }
    
    // Allocate and bind the renderbuffer.
    glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rb[slot]);
    if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
      if (_gsg->get_supports_depth_stencil() && slot == RTP_depth_stencil) {
        glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT,
                                      _rb_size_x, _rb_size_y);
        glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

        GLuint rb;

        rb = _rb[slot];
        if (_shared_depth_buffer) {
          rb = _shared_depth_buffer -> _rb[slot];
        }
        
        glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                          GL_RENDERBUFFER_EXT, rb);

        glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                          GL_RENDERBUFFER_EXT, rb);
      } else {
        glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
                                      _rb_size_x, _rb_size_y);
        glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

        GLuint rb;

        rb = _rb[slot];
        if (_shared_depth_buffer) {
          rb = _shared_depth_buffer -> _rb[slot];
        }

        glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                          GL_RENDERBUFFER_EXT, rb);
      }
    } else {
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGBA8_EXT,
                                    _rb_size_x, _rb_size_y);
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, attachpoint,
                                        GL_RENDERBUFFER_EXT, _rb[slot]);
    }
    
    // Toss any texture that was connected to the slot.
    _tex[slot] = 0;
    _attach_point[slot] = attachpoint;
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::generate_mipmaps
//       Access: Private
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  
//               If we've just rendered into level zero of a mipmapped
//               texture, then all subsequent mipmap levels will now
//               be calculated.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
generate_mipmaps() {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  for (int slot=0; slot<RTP_COUNT; slot++) {
    Texture *tex = _tex[slot];
    if ((tex != 0) && (tex->uses_mipmaps())) {
      glgsg->_state_texture = 0;
      TextureContext *tc = tex->prepare_now(glgsg->get_prepared_objects(), glgsg);
      nassertv(tc != (TextureContext *)NULL);
      CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
      glgsg->update_texture(tc, true);
      GLenum target = glgsg->get_texture_target(tex->get_texture_type());
      GLP(BindTexture)(target, gtc->_index);
      glgsg->_glGenerateMipmap(target);
      GLP(BindTexture)(target, 0);
    }
  }
  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    copy_to_textures();
  }

  // Unbind the FBO
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);
  glgsg->bind_fbo(0);
  
  if (mode == FM_render) {
    generate_mipmaps();
  }

  _host->end_frame(FM_parasite, current_thread);

  if (mode == FM_render) {
    trigger_flip();
    if (_one_shot) {
      prepare_for_deletion();
    }
    clear_cube_map_selection();
  }
  report_my_gl_errors();
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
void CLP(GraphicsBuffer)::
select_cube_map(int cube_map_index) {
  if (cube_map_index == _cube_face_active) {
    return;
  }
  _cube_face_active = cube_map_index;

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  glgsg->bind_fbo(_cubemap_fbo [cube_map_index]);  

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsBuffer)::
open_buffer() {

  // Double check that we have a host
  nassertr(_host != 0, false);

  // Count total color buffers.
  int totalcolor = 1 +
    _fb_properties.get_aux_rgba() +
    _fb_properties.get_aux_hrgba() +
    _fb_properties.get_aux_float();

  // Check for support of relevant extensions.
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);
  if ((!glgsg->_supports_framebuffer_object)||
      (glgsg->_glDrawBuffers == 0)||
      (glgsg->_max_draw_buffers < totalcolor)) {
    return false;
  }
  
  // Describe the framebuffer properties of the FBO.  
  // 
  // The rule is that the properties should be as close
  // as possible to those requested, subject to the limits
  // of the implementation.  This particular implementation
  // is fairly limited.  But that's okay, we just have to
  // tell the truth about what we actually provide by setting
  // the _fb_properties accurately.

  _fb_properties.set_depth_bits(1);
  _fb_properties.set_color_bits(1);
  _fb_properties.set_alpha_bits(1);
  if (_gsg->get_supports_depth_stencil()) {
    _fb_properties.set_stencil_bits(1);
  } else {
    _fb_properties.set_stencil_bits(0);
  }
  _fb_properties.set_accum_bits(0);
  _fb_properties.set_multisamples(0);
  _fb_properties.set_back_buffers(0);
  _fb_properties.set_indexed_color(0);
  _fb_properties.set_rgb_color(1);
  _fb_properties.set_stereo(0);
  _fb_properties.set_force_hardware(_host->get_fb_properties().get_force_hardware());
  _fb_properties.set_force_software(_host->get_fb_properties().get_force_software());
  
  _is_valid = true;
  report_my_gl_errors();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
close_buffer() {

  check_host_valid();
  
  _active = false;
  if (_gsg == 0) {
    return;
  }
  
  // Get the glgsg.
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  report_my_gl_errors();
  // Delete the renderbuffers.
  for (int i=0; i<RTP_COUNT; i++) {
    if (_rb[i] != 0) {
      glgsg->_glDeleteRenderbuffers(1, &(_rb[i]));
      _rb[i] = 0;
    }
    _tex[i] = 0;
  }
  _rb_size_x = 0;
  _rb_size_y = 0;
  report_my_gl_errors();
  
  // Delete the FBO itself.
  if (_fbo != 0) {
    glgsg->_glDeleteFramebuffers(1, &_fbo);
    _fbo = 0;
  }
  report_my_gl_errors();

  for (int f = 0; f < 6; f++) {
    if (_cubemap_fbo [f]) {
      glgsg->_glDeleteFramebuffers(1, &_cubemap_fbo [f]);
      _cubemap_fbo [f] = 0;
      report_my_gl_errors();
    }
  }
  
  // Release the Gsg
  _gsg.clear();

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::share_depth_buffer
//       Access: Published 
//  Description: Will attempt to use the depth buffer of the input
//               graphics_output. The buffer sizes must be exactly 
//               the same. 
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsBuffer)::
share_depth_buffer(GraphicsOutput *graphics_output) {

  bool state;
  CLP(GraphicsBuffer) *input_graphics_output;
  
  state = false;
  input_graphics_output = DCAST (CLP(GraphicsBuffer), graphics_output);
  if (this != input_graphics_output && input_graphics_output) {

    state = true;
    this -> unshare_depth_buffer();
    
    // check buffer sizes
    if (this -> get_x_size() != input_graphics_output -> get_x_size()) {    
      GLCAT.error() << "share_depth_buffer: non matching width \n";
      state = false;    
    }

    if (this -> get_y_size() != input_graphics_output -> get_y_size()) {     
      GLCAT.error() << "share_depth_buffer: non matching height \n";
      state = false;    
    }

    if (state) {    
      // let the input GraphicsOutput know that there is an object 
      // sharing its depth buffer      
      input_graphics_output -> register_shared_depth_buffer(this);
      _shared_depth_buffer = input_graphics_output;
      state = true;
    }
  }
  
  report_my_gl_errors();
  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::unshare_depth_buffer
//       Access: Published 
//  Description: Discontinue sharing the depth buffer.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
unshare_depth_buffer() {
  if (_shared_depth_buffer) {
    // let the GraphicsOutput know that this object is no longer
    // sharing its depth buffer
    _shared_depth_buffer -> unregister_shared_depth_buffer(this);  
    _shared_depth_buffer = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::register_shared_depth_buffer
//       Access: Public
//  Description: Register/save who is sharing the depth buffer.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
register_shared_depth_buffer(GraphicsOutput *graphics_output) {
  CLP(GraphicsBuffer) *input_graphics_output;
  
  input_graphics_output = DCAST (CLP(GraphicsBuffer), graphics_output);
  if (input_graphics_output) {
    // add to list  
    _shared_depth_buffer_list.push_back(input_graphics_output);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::unregister_shared_depth_buffer
//       Access: Public
//  Description: Unregister who is sharing the depth buffer.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
unregister_shared_depth_buffer(GraphicsOutput *graphics_output) {
  CLP(GraphicsBuffer) *input_graphics_output;
  
  input_graphics_output = DCAST (CLP(GraphicsBuffer), graphics_output);
  if (input_graphics_output) {
    // remove from list  
    _shared_depth_buffer_list.remove(input_graphics_output);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::unregister_shared_depth_buffer
//       Access: Public
//  Description: Unregister who is sharing the depth buffer.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
report_my_errors(int line, const char *file) {
  if (_gsg == 0) {
    GLenum error_code = glGetError();
    if (error_code != GL_NO_ERROR) {
      GLCAT.error() << file << ", line " << line << ": GL error " << error_code << "\n";
    }
  } else {
    CLP(GraphicsStateGuardian) *glgsg;
    DCAST_INTO_V(glgsg, _gsg);
    glgsg->report_my_errors(line, file);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::check_host_valid
//       Access: Public
//  Description: If the host window has been closed, then
//               this buffer is dead too.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
check_host_valid() {
  if ((_host == 0)||(!_host->is_valid())) {
    _is_valid = false;
    _active = false;
    _gsg.clear();
    _host.clear();
  }
}
