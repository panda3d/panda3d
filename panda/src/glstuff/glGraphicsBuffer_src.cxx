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
////////////////////////////////////////////////////////////////////

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
  CLP(GraphicsStateGuardian) *glgsg;

  // A FBO doesn't have a back buffer.
  _draw_buffer_type       = RenderBuffer::T_front;
  _screenshot_buffer_type = RenderBuffer::T_front;

  // Initialize these.
  _num_faces = 0;
  memset(_fbo, 0, sizeof(_fbo));
  _fbo_multisample = 0;
  _initial_clear = true;
  _needs_rebuild = true;
  DCAST_INTO_V(glgsg, _gsg);

  if (glgsg->get_supports_framebuffer_multisample() && glgsg->get_supports_framebuffer_blit()) {
    _requested_multisamples = fb_prop.get_multisamples();
  } else {
    _requested_multisamples = 0;
  }

  if (glgsg->get_supports_framebuffer_multisample_coverage_nv() && glgsg->get_supports_framebuffer_blit()) {
    _requested_coverage_samples = fb_prop.get_coverage_samples();
    // Note:  Only 4 and 8 actual samples are supported by the extension, with 8 or 16 coverage samples.
    if ((_requested_coverage_samples <= 8) && (_requested_coverage_samples > 0)) {
      _requested_multisamples = 4;
      _requested_coverage_samples = 8;
    } else if (_requested_coverage_samples > 8) {
      if (_requested_multisamples < 8) {
        _requested_multisamples = 4;
      } else {
        _requested_multisamples = 8;
      }
      _requested_coverage_samples = 16;
    }

  } else {
    _requested_coverage_samples = 0;
  }

  if (_requested_multisamples > glgsg->_max_fb_samples) {
    _requested_multisamples = glgsg->_max_fb_samples;
  }

  _rb_size_x = 0;
  _rb_size_y = 0;
  for (int i=0; i<RTP_COUNT; i++) {
    _rb[i] = 0;
    _tex[i] = 0;
    _rbm[i] = 0;
  }

  _shared_depth_buffer = 0;
  _active_cube_map_index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CLP(GraphicsBuffer)::
~CLP(GraphicsBuffer)() {
  // unshare shared depth buffer if any
  this->unshare_depth_buffer();

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
  _active_cube_map_index = -1;

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

  // Figure out the desired size of the  buffer.
  if (mode == FM_render) {
    clear_cube_map_selection();

    {
      // If the set of render-to-textures has recently changed, we
      // need to rebuild bitplanes.
      CDReader cdata(_cycler);
      if (cdata->_textures_seq != _last_textures_seq) {
        _last_textures_seq = cdata->_textures_seq;
        _needs_rebuild = true;
      }
    }
    if (_creation_flags & GraphicsPipe::BF_size_track_host) {
      if ((_host->get_x_size() != _x_size)||
          (_host->get_y_size() != _y_size)) {
        // We also need to rebuild if we need to change size.
        _needs_rebuild = true;
      }
    }
        
    rebuild_bitplanes();
      
    if (_needs_rebuild) {
      // If we still need rebuild, something went wrong with
      // rebuild_bitplanes().
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
    GLCAT.error() << "EXT_framebuffer_object reports non-framebuffer-completeness:\n";
    switch(status) {
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      GLCAT.error() << "FRAMEBUFFER_UNSUPPORTED_EXT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT"; break;
#ifndef OPENGLES_2
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_FORMATS_EXT"; break;
#endif
#ifndef OPENGLES
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT"; break;
#endif
    default:
      GLCAT.error() << "UNKNOWN PROBLEM " << (int)status; break;
    }
    GLCAT.error(false) << " for " << get_name() << "\n";

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

  if (!_needs_rebuild) {
    glgsg->bind_fbo(_fbo[0]);
    return;
  }
  
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
  memset(attach, 0, sizeof(Texture *) * RTP_COUNT);
  
  // Sort the textures list into appropriate slots.
  bool any_cube_maps = false;
  {
    CDReader cdata(_cycler);
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      if (rtm_mode != RTM_bind_or_copy) {
        continue;
      }
      Texture *tex = rt._texture;
      RenderTexturePlane plane = rt._plane;
      
      // If it's a not a 2D texture or a cube map, punt it.
      if ((tex->get_texture_type() != Texture::TT_2d_texture) &&
          (tex->get_texture_type() != Texture::TT_cube_map)) {
        ((CData *)cdata.p())->_textures[i]._rtm_mode = RTM_copy_texture;
        continue;
      }

      // If I can't find an appropriate slot, or if there's
      // already a texture bound to this slot, then punt
      // this texture.
      if (attach[plane]) {
        ((CData *)cdata.p())->_textures[i]._rtm_mode = RTM_copy_texture;
        continue;
      }

      // Assign the texture to this slot.
      attach[plane] = tex;
      if (tex->get_texture_type() == Texture::TT_cube_map) {
        any_cube_maps = true;
      }
    }
  }
  
  // Having both a depth texture and a depth_stencil texture is
  // invalid: depth_stencil implies depth, and we can't bind them
  // both.  Detect that case, normalize it, and complain.
  if (( attach[RTP_depth] != NULL ) && ( attach[RTP_depth_stencil] != NULL )) {
    attach[RTP_depth] = NULL;
    GLCAT.warning() << "Attempt to bind both Depth and DepthStencil bitplanes.\n";
  }

  _num_faces = 1;
  if (any_cube_maps) {
    _num_faces = 6;
  }

  // Now create the FBO's.
  for (int face = 0; face < _num_faces; ++face) {
    // Bind the FBO
    if (_fbo[face] == 0) {
      glgsg->_glGenFramebuffers(1, &_fbo[face]);
      if (_fbo[face] == 0) {
        report_my_gl_errors();
        return;
      }
    }
    glgsg->bind_fbo(_fbo[face]);

    // For all slots, update the slot.
    
    bind_slot(face, rb_resize, attach, RTP_depth_stencil, GL_DEPTH_ATTACHMENT_EXT);
    bind_slot(face, rb_resize, attach, RTP_depth, GL_DEPTH_ATTACHMENT_EXT);
    bind_slot(face, rb_resize, attach, RTP_color, GL_COLOR_ATTACHMENT0_EXT);
#ifndef OPENGLES
    int next = GL_COLOR_ATTACHMENT1_EXT;
    for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
      bind_slot(face, rb_resize, attach, (RenderTexturePlane)(RTP_aux_rgba_0+i), next);
      next += 1;
    }
    for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
      bind_slot(face, rb_resize, attach, (RenderTexturePlane)(RTP_aux_hrgba_0+i), next);
      next += 1;
    }
    for (int i=0; i<_fb_properties.get_aux_float(); i++) {
      bind_slot(face, rb_resize, attach, (RenderTexturePlane)(RTP_aux_float_0+i), next);
      next += 1;
    }
#endif  // OPENGLES

    // Clear if the fbo was just created, regardless of the clear settings per frame.
    if (_initial_clear) {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
  }

#ifndef OPENGLES
  // Setup any required multisample buffers.
  if (_requested_multisamples) {
    if (_fbo_multisample == 0) {
      glgsg->_glGenFramebuffers(1, &_fbo_multisample);
    }
    glgsg->bind_fbo(_fbo_multisample);
    bind_slot_multisample(rb_resize, attach, RTP_depth, GL_DEPTH_ATTACHMENT_EXT);
    bind_slot_multisample(rb_resize, attach, RTP_color, GL_COLOR_ATTACHMENT0_EXT);

    int next = GL_COLOR_ATTACHMENT1_EXT;
    for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
      bind_slot_multisample(rb_resize, attach, (RenderTexturePlane)(RTP_aux_rgba_0+i), next);
      next += 1;
    }
    for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
      bind_slot_multisample(rb_resize, attach, (RenderTexturePlane)(RTP_aux_hrgba_0+i), next);
      next += 1;
    }
    for (int i=0; i<_fb_properties.get_aux_float(); i++) {
      bind_slot_multisample(rb_resize, attach, (RenderTexturePlane)(RTP_aux_float_0+i), next);
      next += 1;
    }
    glEnable(GL_MULTISAMPLE);

    if (_initial_clear) {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
  } else {
    glDisable(GL_MULTISAMPLE);
  }
#endif  // OPENGLES

  _initial_clear = false;

#ifndef OPENGLES
  if ((_fb_properties.get_rgb_color() > 0) ||
      (_fb_properties.get_aux_hrgba() > 0)) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  } else {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
#endif

  report_my_gl_errors();

  if (!check_fbo()) {
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << get_name() << " check_fbo() returns false\n";
    }
    return;
  }

  _needs_rebuild = false;
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::bind_slot
//       Access: Private
//  Description: Attaches either a texture or a renderbuffer to the
//               specified bitplane.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
bind_slot(int face, bool rb_resize, Texture **attach, RenderTexturePlane slot, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

#ifdef OPENGLES
  GLuint glFormat = GL_RGBA4;
#else
  GLuint glFormat = GL_RGBA;
  switch (slot) {
    case RTP_aux_rgba_0:
    case RTP_aux_rgba_1:
    case RTP_aux_rgba_2:
    case RTP_aux_rgba_3:
      glFormat = GL_RGBA;
      break;
    case RTP_aux_hrgba_0:
    case RTP_aux_hrgba_1:
    case RTP_aux_hrgba_2:
    case RTP_aux_hrgba_3:
      glFormat = GL_RGBA16F_ARB;
      break;
  };
#endif

  Texture *tex = attach[slot];
  if (tex) {
    bool is_cube_map = (tex->get_texture_type() == Texture::TT_cube_map);

    // Bind the texture to the slot.
    tex->set_x_size(_rb_size_x);
    tex->set_y_size(_rb_size_y);
    tex->set_pad_size(_rb_size_x - _x_size, _rb_size_y - _y_size);

    TextureContext *tc = tex->prepare_now(0, glgsg->get_prepared_objects(), glgsg);
    nassertv(tc != (TextureContext *)NULL);
    CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
    glgsg->update_texture(tc, true);

    _use_depth_stencil = false;
    if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
      if ( _gsg->get_supports_depth_stencil() && tex->get_format() == Texture::F_depth_stencil ) {
        tex->set_component_type(Texture::T_unsigned_int_24_8);
        _use_depth_stencil = true;
      }
#ifndef OPENGLES
      GLclampf priority = 1.0f;
      glPrioritizeTextures(1, &gtc->_index, &priority);
#endif
      if (!is_cube_map) {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                       GL_TEXTURE_2D, gtc->_index, 0);
      } else {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                       gtc->_index, 0);
      }
      if (_use_depth_stencil) {
        if (!is_cube_map) {
          glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                         GL_TEXTURE_2D, gtc->_index, 0);
        } else {
          glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                         GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                         gtc->_index, 0);
        }
      }
    } else {
#ifdef OPENGLES
      tex->set_format(Texture::F_rgba4);
#else
      if (glFormat == GL_RGBA16F_ARB) {
        tex->set_format(Texture::F_rgba16);
      } else if (glFormat == GL_RGBA32F_ARB) {
        tex->set_format(Texture::F_rgba32);
      } else {
        tex->set_format(Texture::F_rgba);
      }
#endif

#ifndef OPENGLES
      GLclampf priority = 1.0f;
      glPrioritizeTextures(1, &gtc->_index, &priority);
#endif
      glgsg->update_texture(tc, true);
      if (!is_cube_map) {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachpoint,
                                       GL_TEXTURE_2D, gtc->_index, 0);
      } else {
        glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachpoint,
                                       GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                                       gtc->_index, 0);
      }
    }

    _tex[slot] = tex;

    // If there was a renderbuffer bound to this slot, delete it.
    if (_rb[slot] != 0) {
      glgsg->_glDeleteRenderbuffers(1, &(_rb[slot]));
      _rb[slot] = 0;
    }

  } else {

    // Disconnect from any texture that was previously bound to this slot.
    _tex[slot] = 0;

#ifndef OPENGLES_2
    // Check for the tricky case of a depth_stencil buffer:
    // If we think we need one, but we have a texture bound in the depth slot,
    // then we DON'T want to create a depth_stencil buffer here (because depth is
    // a subset of depth_stencil).
    if (( slot == RTP_depth_stencil ) && ( _gsg->get_supports_depth_stencil() != false ) &&
        ( attach[RTP_depth] != NULL )) {
        return;
    }
#endif

    // If there's no renderbuffer for this slot, create one.
    if (_rb[slot] == 0) {
      glgsg->_glGenRenderbuffers(1, &(_rb[slot]));
    }

    // Allocate and bind the renderbuffer.
    glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rb[slot]);
    if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
#ifndef OPENGLES_2
      if ( _gsg->get_supports_depth_stencil() != false ) {
        if ( slot == RTP_depth_stencil ) {
          glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL_EXT,
                                        _rb_size_x, _rb_size_y);
          GLint depth_size = 0;
          glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &depth_size);
          _fb_properties.set_depth_bits(depth_size);
          GLint stencil_size = 0;
          glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_STENCIL_SIZE_EXT, &stencil_size);
          _fb_properties.set_stencil_bits(stencil_size);

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
          return;

        } else if ( slot == RTP_depth ) {
          // This is the uber-tricky case, where we DON'T want to bind a depth buffer
          // if there's already any form of depth_stencil buffer bound (because depth_stencil
          // is a superset that includes depth).
          if (( _rb[RTP_depth_stencil] != 0 ) || ( attach[RTP_depth_stencil] != NULL )) {
            return;
          }

        }
      }

      // We'll bail out before here if we set a depth_stencil buffer,
      // or figure out that we're GOING to set a depth_stencil buffer.

      // If we get here, we're using the simple fallback case.
#endif
#ifdef OPENGLES
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16,
                                    _rb_size_x, _rb_size_y);
#else
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
                                    _rb_size_x, _rb_size_y);
#endif

      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

      GLuint rb;

      rb = _rb[slot];
      if (_shared_depth_buffer) {
        rb = _shared_depth_buffer -> _rb[slot];
      }

      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, rb);
    } else {
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, glFormat,
                                    _rb_size_x, _rb_size_y);
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, attachpoint,
                                        GL_RENDERBUFFER_EXT, _rb[slot]);
    }
  }

  report_my_gl_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::bind_slot_multisample
//       Access: Private
//  Description: Attaches incoming Texture or renderbuffer to the
//               required bitplanes for the 2 FBOs comprising a
//               multisample graphics buffer.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
bind_slot_multisample(bool rb_resize, Texture **attach, RenderTexturePlane slot, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  if ((_rbm[slot] != 0)&&(!rb_resize)) {
    return;
  }
  if (_rbm[slot] != 0) {
    glgsg->_glDeleteRenderbuffers(1, &(_rbm[slot]));
    _rbm[slot] = 0;
  }
  glgsg->_glBindFramebuffer(GL_FRAMEBUFFER_EXT, _fbo_multisample);
  glgsg->_glGenRenderbuffers(1, &(_rbm[slot]));
  // Allocate and bind the renderbuffer.
  Texture *tex = attach[slot]; // If there is a texture map, use it's format as needed.

  if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
#ifndef OPENGLES_2
    if (_gsg->get_supports_depth_stencil() && _use_depth_stencil) {
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rbm[slot]);
      if (_requested_coverage_samples) {
        glgsg->_glRenderbufferStorageMultisampleCoverage(GL_RENDERBUFFER_EXT, _requested_coverage_samples,
                                                         _requested_multisamples, GL_DEPTH_STENCIL_EXT,
                                                         _rb_size_x, _rb_size_y);
      } else {
        glgsg->_glRenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, _requested_multisamples, GL_DEPTH_STENCIL_EXT,
                                                 _rb_size_x, _rb_size_y);
      }
#ifndef OPENGLES
      GLint givenSamples = -1;
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &givenSamples);
#endif
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
      glgsg->_glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, _rbm[slot]);
      glgsg->_glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, _rbm[slot]);
    } else {
#endif
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rbm[slot]);
      GLuint format = GL_DEPTH_COMPONENT;
      if (tex) {
        if (tex->get_format() == Texture::F_depth_component16)
            format = GL_DEPTH_COMPONENT16;
        if (tex->get_format() == Texture::F_depth_component24)
            format = GL_DEPTH_COMPONENT24;
        if (tex->get_format() == Texture::F_depth_component32)
            format = GL_DEPTH_COMPONENT32;
      }
      if (_requested_coverage_samples) {
        glgsg->_glRenderbufferStorageMultisampleCoverage(GL_RENDERBUFFER_EXT, _requested_coverage_samples,
                                                         _requested_multisamples, format,
                                                         _rb_size_x, _rb_size_y);
      } else {
        glgsg->_glRenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, _requested_multisamples, format,
                                                 _rb_size_x, _rb_size_y);
#ifndef OPENGLES_2
      }
#endif
#ifndef OPENGLES
      GLint givenSamples = -1;
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &givenSamples);
#endif
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
      glgsg->_glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, _rbm[slot]);
    }
  } else {
    Texture *Tex = attach[slot];
    GLuint glFormat = GL_RGBA;
#ifndef OPENGLES
    switch (slot) {
      case RTP_aux_rgba_0:
      case RTP_aux_rgba_1:
      case RTP_aux_rgba_2:
      case RTP_aux_rgba_3:
        glFormat = GL_RGBA;
        break;
      case RTP_aux_hrgba_0:
      case RTP_aux_hrgba_1:
      case RTP_aux_hrgba_2:
      case RTP_aux_hrgba_3:
        glFormat = GL_RGBA16F_ARB;
        break;
    };
#endif
    glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rbm[slot]);
    if (_requested_coverage_samples) {
      glgsg->_glRenderbufferStorageMultisampleCoverage(GL_RENDERBUFFER_EXT, _requested_coverage_samples,
                                    _requested_multisamples, glFormat, _rb_size_x, _rb_size_y);
    } else {
      glgsg->_glRenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, _requested_multisamples, glFormat,
                                               _rb_size_x, _rb_size_y);
    }
#ifndef OPENGLES
    GLint givenSamples = -1;
    glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_SAMPLES_EXT, &givenSamples);
#endif
    glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
    glgsg->_glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER_EXT, attachpoint,
                                      GL_RENDERBUFFER_EXT, _rbm[slot]);
  }
  glgsg->report_my_gl_errors();
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
      TextureContext *tc = tex->prepare_now(0, glgsg->get_prepared_objects(), glgsg);
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

  // Resolve Multisample rendering if using it.
  if (_requested_multisamples && _fbo_multisample) {
    resolve_multisamples();
  }

  if (mode == FM_render) {
    copy_to_textures();
  }

  // Unbind the FBO
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);
  glgsg->bind_fbo(0);
  _active_cube_map_index = -1;

  if (mode == FM_render) {
    generate_mipmaps();
  }

  _host->end_frame(FM_parasite, current_thread);

  if (mode == FM_render) {
    trigger_flip();
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
  nassertv(cube_map_index >=0 && cube_map_index < _num_faces);

  if (_active_cube_map_index != -1) {
    // Resolve the multisample rendering for the previous face.
    if (_requested_multisamples && _fbo_multisample) {
      resolve_multisamples();
    }
  }

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  glgsg->bind_fbo(_fbo[cube_map_index]);
  _active_cube_map_index = cube_map_index;

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
  report_my_gl_errors();

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
  if (!glgsg->_supports_framebuffer_object) {
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

  _fb_properties.set_color_bits(1);
  _fb_properties.set_alpha_bits(_host->get_fb_properties().get_alpha_bits());
  if (_gsg->get_supports_depth_stencil()) {
    _fb_properties.set_stencil_bits(1);
  } else {
    _fb_properties.set_stencil_bits(0);
  }
  _fb_properties.set_accum_bits(0);
  _fb_properties.set_multisamples(_host->get_fb_properties().get_multisamples());
  _fb_properties.set_back_buffers(0);
  _fb_properties.set_indexed_color(0);
  _fb_properties.set_rgb_color(1);
  _fb_properties.set_stereo(0);
  _fb_properties.set_force_hardware(_host->get_fb_properties().get_force_hardware());
  _fb_properties.set_force_software(_host->get_fb_properties().get_force_software());

  _is_valid = true;
  _needs_rebuild = true;
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
  report_my_gl_errors();

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
  // Delete the renderbuffers.
  for (int i=0; i<RTP_COUNT; i++) {
    if (_rbm[i] != 0) {
      glgsg->_glDeleteRenderbuffers(1, &(_rbm[i]));
      _rb[i] = 0;
    }
    _tex[i] = 0;
  }
  _rb_size_x = 0;
  _rb_size_y = 0;
  report_my_gl_errors();

  // Delete the FBO itself.
  for (int face = 0; face < _num_faces; ++face) {
    glgsg->_glDeleteFramebuffers(1, &_fbo[face]);
  }
  _num_faces = 0;

  report_my_gl_errors();

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

    // Check multisample compatibility.
    if ( this->get_multisample_count() != input_graphics_output->get_multisample_count() ) {
      GLCAT.error() << "share_depth_buffer: non matching multisamples \n";
      state = false;
    }

    if ( this->get_coverage_sample_count() != input_graphics_output->get_coverage_sample_count() ) {
      GLCAT.error() << "share_depth_buffer: non matching multisamples \n";
      state = false;
    }

    if (state) {
      // let the input GraphicsOutput know that there is an object
      // sharing its depth buffer
      input_graphics_output -> register_shared_depth_buffer(this);
      _shared_depth_buffer = input_graphics_output;
      state = true;
    }
    _needs_rebuild = true;
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
    _needs_rebuild = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::get_supports_render_texture
//       Access: Published, Virtual
//  Description: Returns true if this particular GraphicsOutput can
//               render directly into a texture, or false if it must
//               always copy-to-texture at the end of each frame to
//               achieve this effect.
////////////////////////////////////////////////////////////////////
bool CLP(GraphicsBuffer)::
get_supports_render_texture() const {
  // FBO-based buffers, by their nature, can always bind-to-texture.
  return true;
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
      GLCAT.error() << file << ", line " << line << ": GL error " << (int)error_code << "\n";
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
    _gsg.clear();
    _host.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::resolve_multisamples
//       Access: Private
//  Description: After the frame has been rendered into the
//               multisample buffer, filters it down into the final
//               render buffer.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
resolve_multisamples() {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  glgsg->report_my_gl_errors();
  GLuint fbo = _fbo[0];
  if (_active_cube_map_index != -1) {
    fbo = _fbo[_active_cube_map_index];
  }
  glgsg->_glBindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, fbo );
  glgsg->_glBindFramebuffer( GL_READ_FRAMEBUFFER_EXT, _fbo_multisample );
  
  // If the depth buffer is shared, resolve it only on the last to render FBO.
  int do_depth_blit = 0;
  if (_shared_depth_buffer) {
    CLP(GraphicsBuffer) *graphics_buffer = NULL;
    CLP(GraphicsBuffer) *highest_sort_graphics_buffer = NULL;
    list <CLP(GraphicsBuffer) *>::iterator graphics_buffer_iterator;
    
    int max_sort_order = 0;
    for (graphics_buffer_iterator = _shared_depth_buffer_list.begin();
         graphics_buffer_iterator != _shared_depth_buffer_list.end();
         graphics_buffer_iterator++) {
      graphics_buffer = (*graphics_buffer_iterator);
      if (graphics_buffer) {
        // this call removes the entry from the list
        if ( graphics_buffer->get_sort() >= max_sort_order ) {
          max_sort_order = graphics_buffer->get_sort();
          highest_sort_graphics_buffer = graphics_buffer;
        }
      }
    }
    if (max_sort_order == this->get_sort()) {
      do_depth_blit = 1;
    }
  } else {
    do_depth_blit = 1;
  }
  if (do_depth_blit) {
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                              GL_NEAREST);
  } else {
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
  }
#ifndef OPENGLES
  // Now handle the other color buffers.
  int next = GL_COLOR_ATTACHMENT1_EXT;
  for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
    glReadBuffer( next );
    glDrawBuffer( next );
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
  for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
    glReadBuffer( next );
    glDrawBuffer( next );
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
  for (int i=0; i<_fb_properties.get_aux_float(); i++) {
    glReadBuffer( next );
    glDrawBuffer( next );
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
  
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
#endif
  report_my_gl_errors();

#ifndef OPENGLES
  if ((_fb_properties.get_rgb_color() > 0) ||
      (_fb_properties.get_aux_hrgba() > 0)) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  } else {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
#endif
  report_my_gl_errors();
}
