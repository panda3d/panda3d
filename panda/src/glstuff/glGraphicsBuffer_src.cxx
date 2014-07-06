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
  _rb_size_z = 0;
  for (int i=0; i<RTP_COUNT; i++) {
    _rb[i] = 0;
    _tex[i] = 0;
    _rbm[i] = 0;
  }

  _shared_depth_buffer = 0;
  _bound_tex_page = -1;
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

    graphics_buffer_iterator = _shared_depth_buffer_list.begin();
    while (graphics_buffer_iterator != _shared_depth_buffer_list.end()) {
      graphics_buffer = (*graphics_buffer_iterator);
      if (graphics_buffer) {
        // this call removes the entry from the list
        graphics_buffer->unshare_depth_buffer();
      }
      graphics_buffer_iterator = _shared_depth_buffer_list.begin();
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
  _bound_tex_page = -1;

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
      GLCAT.error() << "FRAMEBUFFER_UNSUPPORTED"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_DIMENSIONS"; break;
#ifndef OPENGLES_2
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_FORMATS"; break;
#endif
#ifndef OPENGLES
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_ARB:
      GLCAT.error() << "FRAMEBUFFER_INCOMPLETE_LAYER_COUNT"; break;
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
    if (_fbo.size() > 0) {
      glgsg->bind_fbo(_fbo[0]);
    } else {
      glgsg->bind_fbo(0);
    }
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
  _rb_size_z = 1;

  int num_fbos = 1;

  // These variables indicate what should be bound to each bitplane.
  Texture *attach[RTP_COUNT];
  memset(attach, 0, sizeof(Texture *) * RTP_COUNT);

  // Sort the textures list into appropriate slots.
  {
    CDReader cdata(_cycler);

    // Determine whether this will be a layered or a regular FBO.
    // If layered, the number of _rb_size_z will be higher than 1.
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      Texture *tex = rt._texture;

      if (rtm_mode == RTM_bind_layered) {
        _rb_size_z = max(_rb_size_z, tex->get_z_size());
      }
    }

    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      RenderTexturePlane plane = rt._plane;
      Texture *tex = rt._texture;

      if (rtm_mode == RTM_bind_layered) {
        if (tex->get_z_size() != _rb_size_z) {
          GLCAT.warning()
           << "All textures attached to layered FBO should have the same layer count!\n";
        }

        // Assign the texture to this slot.
        attach[plane] = tex;
        continue;
      }

      if (rtm_mode != RTM_bind_or_copy) {
        continue;
      }

      // If we can't bind this type of texture, punt it.
      if ((tex->get_texture_type() != Texture::TT_2d_texture) &&
          (tex->get_texture_type() != Texture::TT_3d_texture) &&
          (tex->get_texture_type() != Texture::TT_2d_texture_array) &&
          (tex->get_texture_type() != Texture::TT_cube_map)) {
        ((CData *)cdata.p())->_textures[i]._rtm_mode = RTM_copy_texture;
        continue;
      }

      if (_rb_size_z > 1 && tex->get_texture_type() == Texture::TT_2d_texture) {
        // We can't bind a 2D texture to a layered FBO.  If the user happened
        // to request RTM_bind_layered for a 2D texture, that's just silly,
        // and we can't render to anything but the first layer anyway.
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

      if (tex->get_z_size() > 1) {
        num_fbos = max(num_fbos, tex->get_z_size());
      }

      // Assign the texture to this slot.
      attach[plane] = tex;
    }
  }

  // Decide whether we should use a depth stencil or just a regular depth attachment.
  // If nothing was attached to either RTP_depth_stencil or RTP_depth, we use a
  // depth-stencil renderbuffer.
  _use_depth_stencil = false;
  if (_gsg->get_supports_depth_stencil()) {
    if (attach[RTP_depth_stencil]) {
      // This is the obvious case, of course.
      _use_depth_stencil = true;

    } else if (attach[RTP_depth]) {
      // We won't use a depth stencil texture as the user
      // explicitly bound something to RTP_depth.
      _use_depth_stencil = false;

    } else if (_fb_properties.get_depth_bits() > 24) {
      // We can't give more than 24 depth bits with a depth-stencil buffer.
      _use_depth_stencil = false;

    } else if (_fb_properties.get_stencil_bits() > 0) {
      // The user requested stencil bits.  Let's take the hint.
      _use_depth_stencil = true;

    } else if (_fb_properties.get_depth_bits() > 0) {
      // Let's use a depth stencil buffer by default, if a depth
      // buffer was requested.
      _use_depth_stencil = true;
    }
  }

  // Knowing this, we can already be a tiny bit
  // more accurate about the framebuffer properties.
  if (_use_depth_stencil) {
    _fb_properties.set_depth_bits(24);
    _fb_properties.set_stencil_bits(8);
  } else {
    _fb_properties.set_stencil_bits(0);
  }

  // Having both a depth texture and a depth_stencil texture is
  // invalid: depth_stencil implies depth, and we can't bind them
  // both.  Detect that case, normalize it, and complain.
  if (_use_depth_stencil && attach[RTP_depth] && attach[RTP_depth_stencil]) {
    attach[RTP_depth] = NULL;
    GLCAT.warning() << "Attempt to bind both RTP_depth and RTP_depth_stencil bitplanes.\n";
  }

  // Now create the FBO's.
  _have_any_color = false;
  _fbo.reserve(num_fbos);
  for (int layer = 0; layer < num_fbos; ++layer) {
    if (layer >= _fbo.size()) {
      _fbo.push_back(0);
    }

    // Bind the FBO
    if (_fbo[layer] == 0) {
      glgsg->_glGenFramebuffers(1, &_fbo[layer]);
      if (_fbo[layer] == 0) {
        report_my_gl_errors();
        return;
      }
    }
    glgsg->bind_fbo(_fbo[layer]);

    // For all slots, update the slot.
    if (_use_depth_stencil) {
      bind_slot(layer, rb_resize, attach, RTP_depth_stencil, GL_DEPTH_ATTACHMENT_EXT);
    } else if (attach[RTP_depth] || _fb_properties.get_depth_bits() > 0) {
      bind_slot(layer, rb_resize, attach, RTP_depth, GL_DEPTH_ATTACHMENT_EXT);
    }

    int next = GL_COLOR_ATTACHMENT0_EXT;
    if (attach[RTP_color] || _fb_properties.get_color_bits() > 0) {
      bind_slot(layer, rb_resize, attach, RTP_color, next++);

      if (_fb_properties.is_stereo()) {
        // The texture has already been initialized, so bind it straight away.
        if (attach[RTP_color] != NULL) {
          attach_tex(layer, 1, attach[RTP_color], next++);
        } else {
          //XXX hack: I needed a slot to use, and we don't currently use RTP_stencil
          // which is treated as a color attachment below, so this fits the bill.
          bind_slot(layer, rb_resize, attach, RTP_stencil, next++);
        }
      }
      _have_any_color = true;
    }

#ifndef OPENGLES
    for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
      bind_slot(layer, rb_resize, attach, (RenderTexturePlane)(RTP_aux_rgba_0+i), next++);
      _have_any_color = true;
    }
    for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
      bind_slot(layer, rb_resize, attach, (RenderTexturePlane)(RTP_aux_hrgba_0+i), next++);
      _have_any_color = true;
    }
    for (int i=0; i<_fb_properties.get_aux_float(); i++) {
      bind_slot(layer, rb_resize, attach, (RenderTexturePlane)(RTP_aux_float_0+i), next++);
      _have_any_color = true;
    }
#endif  // OPENGLES

    // Clear if the fbo was just created, regardless of the clear settings per frame.
    if (_initial_clear) {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
  }

#ifndef OPENGLES
  // Setup any required multisample buffers.  Does not work for layered buffers.
  if (_requested_multisamples && _rb_size_z == 1) {
    if (_fbo_multisample == 0) {
      glgsg->_glGenFramebuffers(1, &_fbo_multisample);
    }
    glgsg->bind_fbo(_fbo_multisample);

    if (_use_depth_stencil || attach[RTP_depth] || _fb_properties.get_depth_bits() > 0) {
      bind_slot_multisample(rb_resize, attach, RTP_depth, GL_DEPTH_ATTACHMENT_EXT);
    }

    int next = GL_COLOR_ATTACHMENT0_EXT;
    if (attach[RTP_color] || _fb_properties.get_color_bits() > 0) {
      bind_slot_multisample(rb_resize, attach, RTP_color, next++);
      if (_fb_properties.is_stereo()) {
        //TODO: figure out how multisample is supposed to work with stereo buffers.
      }
    }

    for (int i=0; i<_fb_properties.get_aux_rgba(); i++) {
      bind_slot_multisample(rb_resize, attach, (RenderTexturePlane)(RTP_aux_rgba_0+i), next++);
    }
    for (int i=0; i<_fb_properties.get_aux_hrgba(); i++) {
      bind_slot_multisample(rb_resize, attach, (RenderTexturePlane)(RTP_aux_hrgba_0+i), next++);
    }
    for (int i=0; i<_fb_properties.get_aux_float(); i++) {
      bind_slot_multisample(rb_resize, attach, (RenderTexturePlane)(RTP_aux_float_0+i), next++);
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

  if (!_have_any_color) {
    _fb_properties.set_color_bits(0);
    _fb_properties.set_alpha_bits(0);
  }

  _initial_clear = false;
  report_my_gl_errors();

#ifndef OPENGLES
  if (_have_any_color) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  } else {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
#endif

  _needs_rebuild = false;
  report_my_gl_errors();

  if (!check_fbo()) {
    if (GLCAT.is_debug()) {
      GLCAT.debug()
        << get_name() << " check_fbo() returns false\n";
    }
    return;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::bind_slot
//       Access: Private
//  Description: Attaches either a texture or a renderbuffer to the
//               specified bitplane.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
bind_slot(int layer, bool rb_resize, Texture **attach, RenderTexturePlane slot, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  Texture *tex = attach[slot];
  _tex[slot] = tex;

  if (tex && layer >= tex->get_z_size()) {
    // If the requested layer index exceeds the number of layers
    // in the texture, we will not bind this layer.
    tex = NULL;
  }

  if (!tex && _rb_size_z > 1) {
    // Since there is no such thing as a layered renderbuffer (to my knowledge),
    // we have to create a dummy texture to render to if no texture was provided.
    tex = new Texture();

    if (_rb_size_z > 1) {
      // Apparently, it doesn't make a difference whether we use setup_cube_map
      // or setup_2d_texture_array here, since it's the same internal storage.
      tex->setup_2d_texture_array(_rb_size_z);
    } else {
      tex->setup_2d_texture();
    }
  }

  if (tex) {
    // Bind the texture to the slot.
    tex->set_x_size(_rb_size_x);
    tex->set_y_size(_rb_size_y);
    if (tex->get_texture_type() != Texture::TT_cube_map && _rb_size_z > 1) {
      tex->set_z_size(_rb_size_z);
    }
    tex->set_pad_size(_rb_size_x - _x_size, _rb_size_y - _y_size);

    // Adjust the texture format based on the requested framebuffer settings.
    switch (slot) {
    case RTP_depth:
      if (_fb_properties.get_depth_bits() > 24) {
        tex->set_format(Texture::F_depth_component32);
      } else if (_fb_properties.get_depth_bits() > 16) {
        tex->set_format(Texture::F_depth_component24);
      } else if (_fb_properties.get_depth_bits() > 8) {
        tex->set_format(Texture::F_depth_component16);
      } else {
        tex->set_format(Texture::F_depth_component);
      }
      break;
    case RTP_depth_stencil:
      tex->set_format(Texture::F_depth_stencil);
      tex->set_component_type(Texture::T_unsigned_int_24_8);
      break;
    case RTP_aux_hrgba_0:
    case RTP_aux_hrgba_1:
    case RTP_aux_hrgba_2:
    case RTP_aux_hrgba_3:
      tex->set_format(Texture::F_rgba16);
      tex->set_component_type(Texture::T_float);
      break;
    case RTP_aux_float_0:
    case RTP_aux_float_1:
    case RTP_aux_float_2:
    case RTP_aux_float_3:
      tex->set_format(Texture::F_rgba32);
      tex->set_component_type(Texture::T_float);
      break;
    default:
      if (_fb_properties.get_color_bits() > 48) {
        tex->set_format(Texture::F_rgba32);
        // Currently a float format.  Should change.
        tex->set_component_type(Texture::T_float);
      } else if (_fb_properties.get_color_bits() > 24) {
        tex->set_format(Texture::F_rgba16);
        // Currently a float format.  Should change.
        tex->set_component_type(Texture::T_float);
      } else {
        tex->set_format(Texture::F_rgba);
      }
    }

#ifndef OPENGLES
    GLenum target = glgsg->get_texture_target(tex->get_texture_type());
    if (target == GL_TEXTURE_CUBE_MAP) {
      target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
    }
#endif

    if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
      GLCAT.debug() << "Binding texture " << *tex << " to depth attachment.\n";

      attach_tex(layer, 0, tex, GL_DEPTH_ATTACHMENT_EXT);

#ifndef OPENGLES
      GLint depth_size = 0;
      GLP(GetTexLevelParameteriv)(target, 0, GL_TEXTURE_DEPTH_SIZE, &depth_size);
      _fb_properties.set_depth_bits(depth_size);
#endif

      if (slot == RTP_depth_stencil) {
        GLCAT.debug() << "Binding texture " << *tex << " to stencil attachment.\n";

        attach_tex(layer, 0, tex, GL_STENCIL_ATTACHMENT_EXT);

#ifndef OPENGLES
        GLint stencil_size = 0;
        GLP(GetTexLevelParameteriv)(target, 0, GL_TEXTURE_STENCIL_SIZE, &stencil_size);
        _fb_properties.set_stencil_bits(stencil_size);
#endif
      }

    } else {
      GLCAT.debug() << "Binding texture " << *tex << " to color attachment.\n";

      attach_tex(layer, 0, tex, attachpoint);

#ifndef OPENGLES
      if (attachpoint == GL_COLOR_ATTACHMENT0_EXT) {
        GLint red_size = 0, green_size = 0, blue_size = 0, alpha_size = 0;
        GLP(GetTexLevelParameteriv)(target, 0, GL_TEXTURE_RED_SIZE, &red_size);
        GLP(GetTexLevelParameteriv)(target, 0, GL_TEXTURE_GREEN_SIZE, &green_size);
        GLP(GetTexLevelParameteriv)(target, 0, GL_TEXTURE_BLUE_SIZE, &blue_size);
        GLP(GetTexLevelParameteriv)(target, 0, GL_TEXTURE_ALPHA_SIZE, &alpha_size);

        _fb_properties.set_color_bits(red_size + green_size + blue_size);
        _fb_properties.set_alpha_bits(alpha_size);
      }
#endif
    }

    // If there was a renderbuffer bound to this slot, delete it.
    if (_rb[slot] != 0) {
      glgsg->_glDeleteRenderbuffers(1, &(_rb[slot]));
      _rb[slot] = 0;
    }

    report_my_gl_errors();

  } else {
    // No texture to bind.  Instead, create a renderbuffer.
    // Choose a suitable renderbuffer format based on the requirements.
#ifdef OPENGLES
    GLuint gl_format = GL_RGBA4;
    switch (slot) {
    case RTP_depth_stencil:
      gl_format = GL_DEPTH_STENCIL_OES;
      break;
    case RTP_depth:
      if (_fb_properties.get_depth_bits() > 24 && glgsg->_supports_depth32) {
        gl_format = GL_DEPTH_COMPONENT32_OES;
      } else if (_fb_properties.get_depth_bits() > 16 && glgsg->_supports_depth24) {
        gl_format = GL_DEPTH_COMPONENT24_OES;
      } else {
        gl_format = GL_DEPTH_COMPONENT16;
      }
      break;
    //case RTP_stencil:
    //  gl_format = GL_STENCIL_INDEX8;
    //  break
    default:
      if (_fb_properties.get_alpha_bits() == 0) {
        if (_fb_properties.get_color_bits() <= 16) {
          gl_format = GL_RGB565_OES;
        } else if (_fb_properties.get_color_bits() <= 24) {
          gl_format = GL_RGB8_OES;
        } else {
          gl_format = GL_RGB10_EXT;
        }
      } else if (_fb_properties.get_color_bits() == 0) {
        gl_format = GL_ALPHA8_EXT;
      } else if (_fb_properties.get_color_bits() <= 12
              && _fb_properties.get_alpha_bits() <= 4) {
        gl_format = GL_RGBA4_OES;
      } else if (_fb_properties.get_color_bits() <= 15
              && _fb_properties.get_alpha_bits() == 1) {
        gl_format = GL_RGB5_A1_OES;
      } else if (_fb_properties.get_color_bits() <= 30
              && _fb_properties.get_alpha_bits() <= 2) {
        gl_format = GL_RGB10_A2_EXT;
      } else {
        gl_format = GL_RGBA8_OES;
      }
    }
#else
    GLuint gl_format = GL_RGBA;
    switch (slot) {
      case RTP_depth_stencil:
        gl_format = GL_DEPTH_STENCIL_EXT;
        break;
      case RTP_depth:
        if (_fb_properties.get_depth_bits() > 24) {
          gl_format = GL_DEPTH_COMPONENT32;
        } else if (_fb_properties.get_depth_bits() > 16) {
          gl_format = GL_DEPTH_COMPONENT24;
        } else if (_fb_properties.get_depth_bits() > 8) {
          gl_format = GL_DEPTH_COMPONENT16;
        } else {
          gl_format = GL_DEPTH_COMPONENT;
        }
        break;
      case RTP_aux_rgba_0:
      case RTP_aux_rgba_1:
      case RTP_aux_rgba_2:
      case RTP_aux_rgba_3:
        gl_format = GL_RGBA;
        break;
      case RTP_aux_hrgba_0:
      case RTP_aux_hrgba_1:
      case RTP_aux_hrgba_2:
      case RTP_aux_hrgba_3:
        gl_format = GL_RGBA16F_ARB;
        break;
      case RTP_aux_float_0:
      case RTP_aux_float_1:
      case RTP_aux_float_2:
      case RTP_aux_float_3:
        gl_format = GL_RGBA32F_ARB;
        break;
      default:
        if (_fb_properties.get_alpha_bits() == 0) {
          gl_format = GL_RGB;
        }
    };
#endif

    // If there's no renderbuffer for this slot, create one.
    if (_rb[slot] == 0) {
      glgsg->_glGenRenderbuffers(1, &(_rb[slot]));
    }

    // Allocate and bind the renderbuffer.
    glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rb[slot]);

    if (slot == RTP_depth_stencil) {
      GLCAT.debug() << "Creating depth stencil renderbuffer.\n";
      // Allocate renderbuffer storage for depth stencil.
      GLint depth_size = 0, stencil_size = 0;
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &depth_size);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_STENCIL_SIZE_EXT, &stencil_size);
      _fb_properties.set_depth_bits(depth_size);
      _fb_properties.set_stencil_bits(stencil_size);

      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

      GLuint rb = _rb[slot];
      if (_shared_depth_buffer) {
        rb = _shared_depth_buffer->_rb[slot];
      }

      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, rb);

      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, rb);

      report_my_gl_errors();

    } else if (slot == RTP_depth) {
      GLCAT.debug() << "Creating depth renderbuffer.\n";
      // Allocate renderbuffer storage for regular depth.
      GLint depth_size = 0;
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &depth_size);
      _fb_properties.set_depth_bits(depth_size);

      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

      GLuint rb = _rb[slot];
      if (_shared_depth_buffer) {
        rb = _shared_depth_buffer->_rb[slot];
      }

      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, rb);

      report_my_gl_errors();

    } else {
      GLCAT.debug() << "Creating color renderbuffer.\n";
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);

      if (attachpoint == GL_COLOR_ATTACHMENT0_EXT) {
        GLint red_size = 0, green_size = 0, blue_size = 0, alpha_size = 0;
        glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_RED_SIZE_EXT, &red_size);
        glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_GREEN_SIZE_EXT, &green_size);
        glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_BLUE_SIZE_EXT, &blue_size);
        glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_ALPHA_SIZE_EXT, &alpha_size);
        _fb_properties.set_color_bits(red_size + green_size + blue_size);
        _fb_properties.set_alpha_bits(alpha_size);
      }
      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, attachpoint,
                                        GL_RENDERBUFFER_EXT, _rb[slot]);

      report_my_gl_errors();
    }
  }
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
    if (_use_depth_stencil) {
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
        switch (tex->get_format()) {
          case Texture::F_depth_component16:
            format = GL_DEPTH_COMPONENT16;
            break;
          case Texture::F_depth_component24:
            format = GL_DEPTH_COMPONENT24;
            break;
          case Texture::F_depth_component32:
            format = GL_DEPTH_COMPONENT32;
            break;
        }
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
    GLuint gl_format = GL_RGBA;
#ifndef OPENGLES
    switch (slot) {
      case RTP_aux_rgba_0:
      case RTP_aux_rgba_1:
      case RTP_aux_rgba_2:
      case RTP_aux_rgba_3:
        gl_format = GL_RGBA;
        break;
      case RTP_aux_hrgba_0:
      case RTP_aux_hrgba_1:
      case RTP_aux_hrgba_2:
      case RTP_aux_hrgba_3:
        gl_format = GL_RGBA16F_ARB;
        break;
      case RTP_aux_float_0:
      case RTP_aux_float_1:
      case RTP_aux_float_2:
      case RTP_aux_float_3:
        gl_format = GL_RGBA32F_ARB;
        break;
    };
#endif
    glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, _rbm[slot]);
    if (_requested_coverage_samples) {
      glgsg->_glRenderbufferStorageMultisampleCoverage(GL_RENDERBUFFER_EXT, _requested_coverage_samples,
                                    _requested_multisamples, gl_format, _rb_size_x, _rb_size_y);
    } else {
      glgsg->_glRenderbufferStorageMultisample(GL_RENDERBUFFER_EXT, _requested_multisamples, gl_format,
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
//     Function: glGraphicsBuffer::attach_tex
//       Access: Private
//  Description: This function attaches the given texture to the
//               given attachment point.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
attach_tex(int layer, int view, Texture *attach, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  if (view >= attach->get_num_views()) {
    attach->set_num_views(view + 1);
  }

  // Create the OpenGL texture object.
  TextureContext *tc = attach->prepare_now(view, glgsg->get_prepared_objects(), glgsg);
  nassertv(tc != (TextureContext *)NULL);
  CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);
  glgsg->update_texture(tc, true);

#ifndef OPENGLES
  GLclampf priority = 1.0f;
  glPrioritizeTextures(1, &gtc->_index, &priority);
#endif

#ifndef OPENGLES
  if (_rb_size_z != 1) {
    // Bind all of the layers of the texture.
    glgsg->_glFramebufferTexture(GL_FRAMEBUFFER_EXT, attachpoint,
                                 gtc->_index, 0);
    return;
  }
#endif

  GLenum target = glgsg->get_texture_target(attach->get_texture_type());
  if (target == GL_TEXTURE_CUBE_MAP) {
    target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
  }

  switch (target) {
#ifndef OPENGLES_1
  case GL_TEXTURE_3D:
    glgsg->_glFramebufferTexture3D(GL_FRAMEBUFFER_EXT, attachpoint,
                                   target, gtc->_index, 0, layer);
    break;
#endif
#ifndef OPENGLES
  case GL_TEXTURE_2D_ARRAY_EXT:
    glgsg->_glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT, attachpoint,
                                      gtc->_index, 0, layer);
    break;
#endif
  default:
    glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachpoint,
                                   target, gtc->_index, 0);
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
  _bound_tex_page = -1;

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
//     Function: glGraphicsBuffer::set_size
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
set_size(int x, int y) {
  if (_x_size != x || _y_size != y) {
    _needs_rebuild = true;
  }

  set_size_and_recalc(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: glGraphicsBuffer::select_target_tex_page
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map, or any other
//               multi-page texture.  This should do whatever needs
//               to be done to switch the buffer to the indicated page.
////////////////////////////////////////////////////////////////////
void CLP(GraphicsBuffer)::
select_target_tex_page(int page) {
  nassertv(page >= 0 && page < _fbo.size());

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_V(glgsg, _gsg);

  bool switched_page = (_bound_tex_page != page);

  if (switched_page) {
    if (_bound_tex_page != -1) {
      // Resolve the multisample rendering for the previous face.
      if (_requested_multisamples && _fbo_multisample) {
        resolve_multisamples();
      }
    }
    
    glgsg->bind_fbo(_fbo[page]);
    _bound_tex_page = page;
  }

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
  int totalcolor =
   (_fb_properties.is_stereo() ? 2 : 1) +
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
  // Unfortunately, we can't possibly predict which formats
  // the implementation will allow us to use at this point, so
  // we'll just have to make some guesses and parrot the rest
  // of the properties back to the user.
  // When we actually attach the textures, we'll update the
  // properties more appropriately.
  // This is probably safe, as we can usually bind just about
  // any supported texture format to the FBO.

  // Rounding the depth bits is not spectacular, but at least we're
  // telling the user *something* about what we're going to get.

  // A lot of code seems to depend on being able to get a
  // color buffer by just setting the rgb_color bit.
  if (_fb_properties.get_color_bits() == 0 &&
      _fb_properties.get_rgb_color() > 0) {
    _fb_properties.set_color_bits(1);
  }

  // Actually, let's always get a colour buffer for now until we
  // figure out why Intel HD Graphics cards complain otherwise.
  if (_fb_properties.get_color_bits() == 0) {
    _fb_properties.set_color_bits(1);
  }

  if (_fb_properties.get_depth_bits() >= 32) {
    _fb_properties.set_depth_bits(32);
  } else if (_fb_properties.get_depth_bits() > 16) {
    _fb_properties.set_depth_bits(24);
  } else if (_fb_properties.get_depth_bits() > 8) {
    _fb_properties.set_depth_bits(16);
  }

  // We're not going to get more than this, ever.
  if (_fb_properties.get_color_bits() > 96) {
    _fb_properties.set_color_bits(96);
  }
  if (_fb_properties.get_alpha_bits() > 32) {
    _fb_properties.set_alpha_bits(32);
  }

  if (!_gsg->get_supports_depth_stencil()) {
    // At least we know we won't be getting stencil bits.
    _fb_properties.set_stencil_bits(0);
  }
  _fb_properties.set_accum_bits(0);
  _fb_properties.set_multisamples(_host->get_fb_properties().get_multisamples());

  // Update aux settings to reflect the GL_MAX_DRAW_BUFFERS limit,
  // if we exceed it, that is.
  int availcolor = glgsg->_max_color_targets;

  if (totalcolor > availcolor) {
    int aux_rgba = _fb_properties.get_aux_rgba();
    int aux_hrgba = _fb_properties.get_aux_hrgba();
    int aux_float = _fb_properties.get_aux_float();

    if (_fb_properties.get_color_bits() > 0 && availcolor > 0) {
      --availcolor;
      if (_fb_properties.is_stereo()) {
        if (availcolor > 0) {
          --availcolor;
        } else {
          _fb_properties.set_stereo(0);
        }
      }
    }
    aux_rgba = min(aux_rgba, availcolor);
    availcolor -= aux_rgba;
    aux_hrgba = min(aux_hrgba, availcolor);
    availcolor -= aux_hrgba;
    aux_float = min(aux_float, availcolor);
    availcolor -= aux_float;

    _fb_properties.set_aux_rgba(aux_rgba);
    _fb_properties.set_aux_hrgba(aux_hrgba);
    _fb_properties.set_aux_float(aux_float);
  }

  _fb_properties.set_back_buffers(0);
  _fb_properties.set_indexed_color(0);
  _fb_properties.set_rgb_color(1);
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
  for (int i = 0; i < _fbo.size(); ++i) {
    glgsg->_glDeleteFramebuffers(1, &_fbo[i]);
  }
  _fbo.clear();

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
  input_graphics_output = DCAST(CLP(GraphicsBuffer), graphics_output);
  if (this != input_graphics_output && input_graphics_output) {

    state = true;
    this->unshare_depth_buffer();

    // Make sure that the buffers are both FBOs.
    if (!input_graphics_output->is_of_type(CLP(GraphicsBuffer)::get_class_type())) {
      GLCAT.error() << "share_depth_buffer: non-matching type\n";
      state = false;
    }

    // Check buffer size.
    if (this->get_x_size() != input_graphics_output->get_x_size()) {
      GLCAT.error() << "share_depth_buffer: non-matching width\n";
      state = false;
    }

    if (this->get_y_size() != input_graphics_output->get_y_size()) {
      GLCAT.error() << "share_depth_buffer: non-matching height\n";
      state = false;
    }

    // Check multisample compatibility.
    if (this->get_multisample_count() != input_graphics_output->get_multisample_count()) {
      GLCAT.error() << "share_depth_buffer: non-matching multisamples\n";
      state = false;
    }

    if (this->get_coverage_sample_count() != input_graphics_output->get_coverage_sample_count()) {
      GLCAT.error() << "share_depth_buffer: non-matching coverage samples\n";
      state = false;
    }

    if (state) {
      // let the input GraphicsOutput know that there is an object
      // sharing its depth buffer
      input_graphics_output->register_shared_depth_buffer(this);
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
    _shared_depth_buffer->unregister_shared_depth_buffer(this);
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

  nassertv(_fbo.size() > 0);

  glgsg->report_my_gl_errors();
  GLuint fbo = _fbo[0];
  if (_bound_tex_page != -1) {
    fbo = _fbo[_bound_tex_page];
  }
  glgsg->_glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, fbo);
  glgsg->_glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, _fbo_multisample);
  
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
  if (_fb_properties.is_stereo()) {
    glReadBuffer(next);
    glDrawBuffer(next);
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
  for (int i = 0; i < _fb_properties.get_aux_rgba(); ++i) {
    glReadBuffer(next);
    glDrawBuffer(next);
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
  for (int i = 0; i < _fb_properties.get_aux_hrgba(); ++i) {
    glReadBuffer(next);
    glDrawBuffer(next);
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
  for (int i = 0; i < _fb_properties.get_aux_float(); ++i) {
    glReadBuffer(next);
    glDrawBuffer(next);
    glgsg->_glBlitFramebuffer(0, 0, _rb_size_x, _rb_size_y, 0, 0, _rb_size_x, _rb_size_y,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
    next += 1;
  }
#endif
  report_my_gl_errors();

#ifndef OPENGLES
  if (_have_any_color) {
    glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  } else {
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
  }
#endif
  report_my_gl_errors();
}
