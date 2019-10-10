/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGraphicsBuffer_src.cxx
 * @author jyelon
 * @date 2006-01-15
 */

#include "depthWriteAttrib.h"

using std::max;
using std::min;

TypeHandle CLP(GraphicsBuffer)::_type_handle;

/**
 *
 */
CLP(GraphicsBuffer)::
CLP(GraphicsBuffer)(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _bind_texture_pcollector(_draw_window_pcollector, "Bind textures"),
  _generate_mipmap_pcollector(_draw_window_pcollector, "Generate mipmaps"),
  _resolve_multisample_pcollector(_draw_window_pcollector, "Resolve multisamples"),
  _requested_multisamples(0),
  _requested_coverage_samples(0),
  _rb_context(nullptr)
{
  // A FBO doesn't have a back buffer.
  _draw_buffer_type       = RenderBuffer::T_front;
  _screenshot_buffer_type = RenderBuffer::T_front;

  // Initialize these.
  _fbo_multisample = 0;
  _initial_clear = true;
  _needs_rebuild = true;

  _rb_size_x = 0;
  _rb_size_y = 0;
  _rb_size_z = 0;
  for (int i = 0; i < RTP_COUNT; ++i) {
    _rb[i] = 0;
    _rbm[i] = 0;
  }
  _rb_data_size_bytes = 0;

  _shared_depth_buffer = 0;
  _bound_tex_page = -1;
}

/**
 *
 */
CLP(GraphicsBuffer)::
~CLP(GraphicsBuffer)() {
  // unshare shared depth buffer if any
  this->unshare_depth_buffer();

  // unshare all buffers that are sharing this object's depth buffer
  {
    CLP(GraphicsBuffer) *graphics_buffer;
    std::list <CLP(GraphicsBuffer) *>::iterator graphics_buffer_iterator;

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

#ifndef OPENGLES
/**
 * Clears the entire framebuffer before rendering, according to the settings
 * of get_color_clear_active() and get_depth_clear_active() (inherited from
 * DrawableRegion).
 *
 * This function is called only within the draw thread.
 */
void CLP(GraphicsBuffer)::
clear(Thread *current_thread) {
  if (!is_any_clear_active()) {
    return;
  }

  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  if (glgsg->_glClearBufferfv == nullptr) {
    // We can't efficiently clear the buffer.  Fall back to the inefficient
    // default implementation for now.
    GraphicsOutput::clear(current_thread);
    return;
  }

  if (display_cat.is_spam()) {
    display_cat.spam()
      << "clear(): " << get_type() << " "
      << get_name() << " " << (void *)this << "\n";
  }

  // Disable the scissor test, so we can clear the whole buffer.
  glDisable(GL_SCISSOR_TEST);
  glgsg->_scissor_enabled = false;
  glgsg->_scissor_array.clear();
  glgsg->_scissor_attrib_active = false;

  if (GLCAT.is_spam()) {
    GLCAT.spam()
      << "glDisable(GL_SCISSOR_TEST)\n";
  }

  // Set the buffers into which we'll be indexing with glClearBuffer.
  int draw_buffer_type = _draw_buffer_type & _fb_properties.get_buffer_mask();
  draw_buffer_type |= _fb_properties.get_aux_mask();
  glgsg->_color_write_mask = ColorWriteAttrib::C_all;
  glgsg->set_draw_buffer(draw_buffer_type);

  int index = 0;
  if (_fb_properties.get_color_bits() > 0) {
    if (_fb_properties.is_stereo()) {
      // Clear both left and right attachments.
      if (get_clear_active(RTP_color)) {
        LColorf v = LCAST(float, get_clear_value(RTP_color));
        glgsg->_glClearBufferfv(GL_COLOR, index, v.get_data());
        glgsg->_glClearBufferfv(GL_COLOR, index + 1, v.get_data());
      }
      index += 2;
    } else {
      if (get_clear_active(RTP_color)) {
        LColorf v = LCAST(float, get_clear_value(RTP_color));
        glgsg->_glClearBufferfv(GL_COLOR, index, v.get_data());
      }
      ++index;
    }
  }
  for (int i = 0; i < _fb_properties.get_aux_rgba(); ++i) {
    int layerid = RTP_aux_rgba_0 + i;
    if (get_clear_active(layerid)) {
      LColorf v = LCAST(float, get_clear_value(layerid));
      glgsg->_glClearBufferfv(GL_COLOR, index, v.get_data());
    }
    ++index;
  }
  for (int i = 0; i < _fb_properties.get_aux_hrgba(); ++i) {
    int layerid = RTP_aux_hrgba_0 + i;
    if (get_clear_active(layerid)) {
      LColorf v = LCAST(float, get_clear_value(layerid));
      glgsg->_glClearBufferfv(GL_COLOR, index, v.get_data());
    }
    ++index;
  }
  for (int i = 0; i < _fb_properties.get_aux_float(); ++i) {
    int layerid = RTP_aux_float_0 + i;
    if (get_clear_active(layerid)) {
      LColorf v = LCAST(float, get_clear_value(layerid));
      glgsg->_glClearBufferfv(GL_COLOR, index, v.get_data());
    }
    ++index;
  }

  if (get_clear_depth_active()) {
    glDepthMask(GL_TRUE);
    glgsg->_state_mask.clear_bit(DepthWriteAttrib::get_class_slot());

    if (get_clear_stencil_active()) {
      glStencilMask(~0);
      glgsg->_glClearBufferfi(GL_DEPTH_STENCIL, 0, get_clear_depth(), get_clear_stencil());
    } else {
      GLfloat depth = get_clear_depth();
      glgsg->_glClearBufferfv(GL_DEPTH, 0, &depth);
    }
  } else if (get_clear_stencil_active()) {
    GLint stencil = get_clear_stencil();
    glgsg->_glClearBufferiv(GL_STENCIL, 0, &stencil);
  }

  report_my_gl_errors();
}
#endif

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
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

  if (_host != nullptr) {
    if (!_host->begin_frame(FM_parasite, current_thread)) {
      if (GLCAT.is_debug()) {
        GLCAT.debug()
          << get_name() << "'s host is not ready\n";
      }
      return false;
    }
  } else {
    // We don't have a host window, which is possible for CocoaGraphicsBuffer.
    _gsg->set_current_properties(&get_fb_properties());
    if (!_gsg->begin_frame(current_thread)) {
      return false;
    }
  }

  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();
  glgsg->push_group_marker(std::string(CLASSPREFIX_QUOTED "GraphicsBuffer ") + get_name());

  // Figure out the desired size of the  buffer.
  if (mode == FM_render) {
    clear_cube_map_selection();

    {
      // If the set of render-to-textures has recently changed, we need to
      // rebuild bitplanes.
      CDReader cdata(_cycler);
      if (cdata->_textures_seq != _last_textures_seq) {
        _last_textures_seq = cdata->_textures_seq;
        _needs_rebuild = true;
      }
    }
    if (_creation_flags & GraphicsPipe::BF_size_track_host) {
      if (_host != nullptr && _host->get_size() != _size) {
        // We also need to rebuild if we need to change size.
        _needs_rebuild = true;
      }
    }

    rebuild_bitplanes();

    if (_needs_rebuild) {
      // If we still need rebuild, something went wrong with
      // rebuild_bitplanes().
      glgsg->pop_group_marker();
      return false;
    }

    // In case of multisample rendering, we don't need to issue the barrier
    // until we call glBlitFramebuffer.
#ifndef OPENGLES_1
    if (gl_enable_memory_barriers && _fbo_multisample == 0) {
      CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

      TextureContexts::iterator it;
      for (it = _texture_contexts.begin(); it != _texture_contexts.end(); ++it) {
        CLP(TextureContext) *gtc = *it;

        if (gtc != nullptr && gtc->needs_barrier(GL_FRAMEBUFFER_BARRIER_BIT)) {
          glgsg->issue_memory_barrier(GL_FRAMEBUFFER_BARRIER_BIT);
          // If we've done it for one, we've done it for all.
          break;
        }
      }
    }
#endif
  } else if (mode == FM_refresh) {
    // Just bind the FBO.
    rebuild_bitplanes();
  }

  // The host window may not have had sRGB enabled, so we need to do this.
#ifndef OPENGLES
  if (get_fb_properties().get_srgb_color()) {
    glEnable(GL_FRAMEBUFFER_SRGB);
  }
#endif

  _gsg->set_current_properties(&get_fb_properties());
  report_my_gl_errors();
  return true;
}

/**
 * Calls 'glCheckFramebufferStatus'.  On error, prints out an appropriate
 * error message and unbinds the fbo.  Returns true for OK or false for error.
 */
bool CLP(GraphicsBuffer)::
check_fbo() {
  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

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

/**
 * This function will be called within the draw thread to allocate/reallocate
 * the fbo and all the associated renderbuffers, just before rendering a
 * frame.
 */
void CLP(GraphicsBuffer)::
rebuild_bitplanes() {
  check_host_valid();
  if (_gsg == 0) {
    return;
  }

  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  if (!_needs_rebuild) {
    if (_fbo_multisample != 0) {
      glgsg->bind_fbo(_fbo_multisample);
    } else if (_fbo.size() > 0) {
      glgsg->bind_fbo(_fbo[0]);
    } else {
      glgsg->bind_fbo(0);
    }
    _rb_context->set_active(true);
    return;
  }

  PStatGPUTimer timer(glgsg, _bind_texture_pcollector);

  // Calculate bitplane size.  This can be larger than the buffer.
  if (_creation_flags & GraphicsPipe::BF_size_track_host) {
    if (_host != nullptr && _host->get_size() != _size) {
      set_size_and_recalc(_host->get_x_size(),
                          _host->get_y_size());
    }
  }

  int bitplane_x = get_x_size();
  int bitplane_y = get_y_size();
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
  _rb_data_size_bytes = 0;

  size_t num_fbos = 1;

  // These variables indicate what should be bound to each bitplane.
  Texture *attach[RTP_COUNT];
  memset(attach, 0, sizeof(Texture *) * RTP_COUNT);
  _texture_contexts.clear();

  // Sort the textures list into appropriate slots.
  {
    CDReader cdata(_cycler);

    // Determine whether this will be a layered or a regular FBO. If layered,
    // the number of _rb_size_z will be higher than 1.
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

      if (rtm_mode == RTM_bind_layered && glgsg->_supports_geometry_shaders) {
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

      // If I can't find an appropriate slot, or if there's already a texture
      // bound to this slot, then punt this texture.
      if (attach[plane]) {
        ((CData *)cdata.p())->_textures[i]._rtm_mode = RTM_copy_texture;
        continue;
      }

      if (tex->get_z_size() > 1) {
        num_fbos = max(num_fbos, (size_t)tex->get_z_size());
      }

      // Assign the texture to this slot.
      attach[plane] = tex;
    }
  }

  // Decide whether we should use a depth stencil or just a regular depth
  // attachment.  If nothing was attached to either RTP_depth_stencil or
  // RTP_depth, we use a depth-stencil renderbuffer.
  _use_depth_stencil = false;
  if (_gsg->get_supports_depth_stencil()) {
    if (attach[RTP_depth_stencil]) {
      // This is the obvious case, of course.
      _use_depth_stencil = true;

    } else if (attach[RTP_depth]) {
      // We won't use a depth stencil texture as the user explicitly bound
      // something to RTP_depth.
      _use_depth_stencil = false;

    } else if (_fb_properties.get_stencil_bits() > 0) {
      // The user requested stencil bits.  Let's take the hint.
      _use_depth_stencil = true;

    } else if (_fb_properties.get_depth_bits() > 24 ||
               _fb_properties.get_float_depth()) {
      // 32-bit float depth is supported in conjunction with depth stencil,
      // but it's a waste.  Let's not do it unless the user requested stencil.
      _use_depth_stencil = false;

    } else if (_fb_properties.get_depth_bits() > 0) {
      // Let's use a depth stencil buffer by default, if a depth buffer was
      // requested.
      _use_depth_stencil = true;
    }
  } else if (attach[RTP_depth_stencil] != nullptr && attach[RTP_depth] == nullptr) {
    // The depth stencil slot was assigned a texture, but we don't support it.
    // Downgrade to a regular depth texture.
    std::swap(attach[RTP_depth], attach[RTP_depth_stencil]);
  }

  // Knowing this, we can already be a tiny bit more accurate about the
  // framebuffer properties.
  if (_use_depth_stencil) {
    _fb_properties.set_stencil_bits(8);
  } else {
    _fb_properties.set_stencil_bits(0);
  }

  // Having both a depth texture and a depth_stencil texture is invalid:
  // depth_stencil implies depth, and we can't bind them both.  Detect that
  // case, normalize it, and complain.
  if (_use_depth_stencil && attach[RTP_depth] && attach[RTP_depth_stencil]) {
    attach[RTP_depth] = nullptr;
    GLCAT.warning() << "Attempt to bind both RTP_depth and RTP_depth_stencil bitplanes.\n";
  }

  // Now create the FBO's.
  _have_any_color = false;
  bool have_any_depth = false;

  if (num_fbos > _fbo.size()) {
    // Generate more FBO handles.
    size_t start = _fbo.size();
    GLuint zero = 0;
    _fbo.resize(num_fbos, zero);
    glgsg->_glGenFramebuffers(num_fbos - start, &_fbo[start]);
  }

  for (int layer = 0; layer < (int)num_fbos; ++layer) {
    // Bind the FBO
    if (_fbo[layer] == 0) {
      report_my_gl_errors();
      return;
    }
    glgsg->bind_fbo(_fbo[layer]);

    if (glgsg->_use_object_labels) {
      // Assign a label for OpenGL to use when displaying debug messages.
      if (num_fbos > 1) {
        std::ostringstream strm;
        strm << _name << '[' << layer << ']';
        std::string name = strm.str();
        glgsg->_glObjectLabel(GL_FRAMEBUFFER, _fbo[layer], name.size(), name.data());
      } else {
        glgsg->_glObjectLabel(GL_FRAMEBUFFER, _fbo[layer], _name.size(), _name.data());
      }
    }

    // For all slots, update the slot.
    if (_use_depth_stencil) {
      bind_slot(layer, rb_resize, attach, RTP_depth_stencil, GL_DEPTH_ATTACHMENT_EXT);
      have_any_depth = true;
    } else if (attach[RTP_depth] || _fb_properties.get_depth_bits() > 0) {
      bind_slot(layer, rb_resize, attach, RTP_depth, GL_DEPTH_ATTACHMENT_EXT);
      have_any_depth = true;
    }

    int next = GL_COLOR_ATTACHMENT0_EXT;
    if (attach[RTP_color] || _fb_properties.get_color_bits() > 0) {
      bind_slot(layer, rb_resize, attach, RTP_color, next++);

      if (_fb_properties.is_stereo()) {
        // The second tex view has already been initialized, so bind it
        // straight away.
        if (attach[RTP_color] != nullptr) {
          attach_tex(layer, 1, attach[RTP_color], next++);
        } else {
          // XXX hack: I needed a slot to use, and we don't currently use
          // RTP_stencil and it's treated as a color attachment below, so this
          // fits the bill.  Eventually, we might want to add RTP_color_left
          // and RTP_color_right.
          bind_slot(layer, rb_resize, attach, RTP_stencil, next++);
        }
      }
      _have_any_color = true;
    }

#ifndef OPENGLES_1
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

    if (_have_any_color || have_any_depth) {
      // Clear if the fbo was just created, regardless of the clear settings per
      // frame.
      if (_initial_clear) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      }
#ifndef OPENGLES_1
    } else if (glgsg->_supports_empty_framebuffer) {
      // Set the "default" width and height, which is required to have an FBO
      // without any attachments.
      glgsg->_glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, _rb_size_x);
      glgsg->_glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, _rb_size_y);
#endif
    } else {
      // If all else fails, just bind a "dummy" attachment.
      bind_slot(layer, rb_resize, attach, RTP_color, next++);
    }
  }

#ifndef OPENGLES
  // Setup any required multisample buffers.  Does not work for layered
  // buffers.
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
        // TODO: figure out how multisample is supposed to work with stereo
        // buffers.
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

    // Mark the GSG as supporting multisampling, so that it will respect an
    // AntialiasAttrib with mode M_multisample.
    glgsg->_supports_multisample = true;
  } else {
    glDisable(GL_MULTISAMPLE);
  }
#endif  // OPENGLES

  if (!_have_any_color) {
    _fb_properties.set_rgba_bits(0, 0, 0, 0);
  }

  _rb_context->set_active(true);
  _rb_context->update_data_size_bytes(_rb_data_size_bytes);

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

/**
 * Attaches either a texture or a renderbuffer to the specified bitplane.
 */
void CLP(GraphicsBuffer)::
bind_slot(int layer, bool rb_resize, Texture **attach, RenderTexturePlane slot, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  Texture *tex = attach[slot];

  if (tex && layer >= tex->get_z_size()) {
    // If the requested layer index exceeds the number of layers in the
    // texture, we will not bind this layer.
    tex = nullptr;
  }

  if (!tex && _rb_size_z > 1) {
    // Since there is no such thing as a layered renderbuffer (to my
    // knowledge), we have to create a dummy texture to render to if no
    // texture was provided.
    tex = new Texture();

    if (_rb_size_z > 1) {
      // Apparently, it doesn't make a difference whether we use
      // setup_cube_map or setup_2d_texture_array here, since it's the same
      // internal storage.
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
    tex->set_pad_size(_rb_size_x - get_x_size(), _rb_size_y - get_y_size());

    // Adjust the texture format based on the requested framebuffer settings.
    switch (slot) {
    case RTP_depth:
      _fb_properties.setup_depth_texture(tex);
      break;
    case RTP_depth_stencil:
      tex->set_format(Texture::F_depth_stencil);

      if (_fb_properties.get_float_depth()) {
        tex->set_component_type(Texture::T_float);
      } else {
        tex->set_component_type(Texture::T_unsigned_int_24_8);
      }
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
      _fb_properties.setup_color_texture(tex);
    }

    GLenum target = glgsg->get_texture_target(tex->get_texture_type());
    if (target == GL_TEXTURE_CUBE_MAP) {
      target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer;
    }

    if (attachpoint == GL_DEPTH_ATTACHMENT_EXT) {
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Binding texture " << *tex << " to depth attachment.\n";
      }

      attach_tex(layer, 0, tex, GL_DEPTH_ATTACHMENT_EXT);

#ifndef OPENGLES
      GLint depth_size = 0;
      glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH_SIZE, &depth_size);
      _fb_properties.set_depth_bits(depth_size);
#endif

      if (slot == RTP_depth_stencil) {
        if (GLCAT.is_debug()) {
          GLCAT.debug() << "Binding texture " << *tex << " to stencil attachment.\n";
        }

        attach_tex(layer, 0, tex, GL_STENCIL_ATTACHMENT_EXT);

#ifndef OPENGLES
        GLint stencil_size = 0;
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_STENCIL_SIZE, &stencil_size);
        _fb_properties.set_stencil_bits(stencil_size);
#endif
      }

    } else {
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Binding texture " << *tex << " to color attachment.\n";
      }

      attach_tex(layer, 0, tex, attachpoint);

#ifndef OPENGLES
      if (attachpoint == GL_COLOR_ATTACHMENT0_EXT) {
        GLint red_size = 0, green_size = 0, blue_size = 0, alpha_size = 0;
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_RED_SIZE, &red_size);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_GREEN_SIZE, &green_size);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_BLUE_SIZE, &blue_size);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_ALPHA_SIZE, &alpha_size);

        _fb_properties.set_rgba_bits(red_size, green_size, blue_size, alpha_size);
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
    // No texture to bind.  Instead, create a renderbuffer.  Choose a suitable
    // renderbuffer format based on the requirements.
#ifdef OPENGLES
    // OpenGL ES case.
    GLuint gl_format = GL_RGBA4;
    switch (slot) {
    case RTP_depth_stencil:
      gl_format = GL_DEPTH24_STENCIL8_OES;
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
    // NB: we currently use RTP_stencil to store the right eye for stereo.
    // case RTP_stencil: gl_format = GL_STENCIL_INDEX8; break
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
        if (_fb_properties.get_depth_bits() > 24 ||
            _fb_properties.get_float_depth()) {
          if (!glgsg->_use_remapped_depth_range) {
            gl_format = GL_DEPTH32F_STENCIL8;
          } else {
            gl_format = GL_DEPTH32F_STENCIL8_NV;
          }
        } else {
          gl_format = GL_DEPTH24_STENCIL8;
        }
        break;
      case RTP_depth:
        if (_fb_properties.get_float_depth()) {
          if (!glgsg->_use_remapped_depth_range) {
            gl_format = GL_DEPTH_COMPONENT32F;
          } else {
            gl_format = GL_DEPTH_COMPONENT32F_NV;
          }
        } else if (_fb_properties.get_depth_bits() > 24) {
          gl_format = GL_DEPTH_COMPONENT32;
        } else if (_fb_properties.get_depth_bits() > 16) {
          gl_format = GL_DEPTH_COMPONENT24;
        } else if (_fb_properties.get_depth_bits() > 1) {
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
          if (_fb_properties.get_srgb_color()) {
            gl_format = GL_SRGB8;
          } else if (_fb_properties.get_color_bits() > 16 * 3 ||
                     _fb_properties.get_red_bits() > 16 ||
                     _fb_properties.get_green_bits() > 16 ||
                     _fb_properties.get_blue_bits() > 16) {
            // 32-bit, which is always floating-point.
            if (_fb_properties.get_blue_bits() > 0 ||
                _fb_properties.get_color_bits() == 1 ||
                _fb_properties.get_color_bits() > 32 * 2) {
              gl_format = GL_RGB32F;
            } else if (_fb_properties.get_green_bits() > 0 ||
                       _fb_properties.get_color_bits() > 32) {
              gl_format = GL_RG32F;
            } else {
              gl_format = GL_R32F;
            }
          } else if (_fb_properties.get_float_color()) {
            // 16-bit floating-point.
            if (_fb_properties.get_blue_bits() > 0 ||
                _fb_properties.get_color_bits() == 1 ||
                _fb_properties.get_color_bits() > 16 * 2) {
              gl_format = GL_RGB16F;
            } else if (_fb_properties.get_green_bits() > 0 ||
                       _fb_properties.get_color_bits() > 16) {
              gl_format = GL_RG16F;
            } else {
              gl_format = GL_R16F;
            }
          } else if (_fb_properties.get_color_bits() > 8 * 3) {
            gl_format = GL_RGB16_EXT;
          } else {
            gl_format = GL_RGB;
          }
        } else {
          if (_fb_properties.get_srgb_color()) {
            gl_format = GL_SRGB8_ALPHA8;
          } else if (_fb_properties.get_float_color()) {
            if (_fb_properties.get_color_bits() > 16 * 3) {
              gl_format = GL_RGBA32F_ARB;
            } else {
              gl_format = GL_RGBA16F_ARB;
            }
          } else {
            if (_fb_properties.get_color_bits() > 16 * 3) {
              gl_format = GL_RGBA32F_ARB;
            } else if (_fb_properties.get_color_bits() > 8 * 3) {
              gl_format = GL_RGBA16_EXT;
            } else {
              gl_format = GL_RGBA;
            }
          }
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
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Creating depth stencil renderbuffer.\n";
      }
      // Allocate renderbuffer storage for depth stencil.
      GLint depth_size = 0, stencil_size = 0;
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &depth_size);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_STENCIL_SIZE_EXT, &stencil_size);
      _fb_properties.set_depth_bits(depth_size);
      _fb_properties.set_stencil_bits(stencil_size);
      _rb_data_size_bytes += _rb_size_x * _rb_size_y * ((depth_size + stencil_size) / 8);

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
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Creating depth renderbuffer.\n";
      }
      // Allocate renderbuffer storage for regular depth.
      GLint depth_size = 0;
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &depth_size);

#ifndef OPENGLES
      // Are we getting only 24 bits of depth when we requested 32?  It may be
      // because GL_DEPTH_COMPONENT32 is not a required format, while 32F is.
      if (gl_format == GL_DEPTH_COMPONENT32 && depth_size < 32) {
        if (!glgsg->_use_remapped_depth_range) {
          gl_format = GL_DEPTH_COMPONENT32F;
        } else {
          gl_format = GL_DEPTH_COMPONENT32F_NV;
        }
        glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);
        glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &depth_size);

        _fb_properties.set_float_depth(true);
      }
#endif

      _fb_properties.set_depth_bits(depth_size);
      _rb_data_size_bytes += _rb_size_x * _rb_size_y * (depth_size / 8);

      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);

      GLuint rb = _rb[slot];
      if (_shared_depth_buffer) {
        rb = _shared_depth_buffer->_rb[slot];
      }

      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                        GL_RENDERBUFFER_EXT, rb);

      report_my_gl_errors();

    } else {
      if (GLCAT.is_debug()) {
        GLCAT.debug() << "Creating color renderbuffer.\n";
      }
      glgsg->_glRenderbufferStorage(GL_RENDERBUFFER_EXT, gl_format, _rb_size_x, _rb_size_y);

      GLint red_size = 0, green_size = 0, blue_size = 0, alpha_size = 0;
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_RED_SIZE_EXT, &red_size);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_GREEN_SIZE_EXT, &green_size);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_BLUE_SIZE_EXT, &blue_size);
      glgsg->_glGetRenderbufferParameteriv(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_ALPHA_SIZE_EXT, &alpha_size);

      if (attachpoint == GL_COLOR_ATTACHMENT0_EXT) {
        _fb_properties.set_rgba_bits(red_size, green_size, blue_size, alpha_size);
      }
      _rb_data_size_bytes += _rb_size_x * _rb_size_y * ((red_size + green_size + blue_size + alpha_size) / 8);

      glgsg->_glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
      glgsg->_glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, attachpoint,
                                        GL_RENDERBUFFER_EXT, _rb[slot]);

      report_my_gl_errors();
    }
  }
}

/**
 * Attaches incoming Texture or renderbuffer to the required bitplanes for the
 * 2 FBOs comprising a multisample graphics buffer.
 */
void CLP(GraphicsBuffer)::
bind_slot_multisample(bool rb_resize, Texture **attach, RenderTexturePlane slot, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

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
#ifndef OPENGLES
      if (_fb_properties.get_float_depth()) {
        if (!glgsg->_use_remapped_depth_range) {
          format = GL_DEPTH_COMPONENT32F;
        } else {
          format = GL_DEPTH_COMPONENT32F_NV;
        }
      } else
#endif
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
          default:
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
    GLuint gl_format = GL_RGBA;
#ifndef OPENGLES
    switch (slot) {
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
      case RTP_aux_rgba_0:
      case RTP_aux_rgba_1:
      case RTP_aux_rgba_2:
      case RTP_aux_rgba_3:
      default:
        if (_fb_properties.get_srgb_color()) {
          gl_format = GL_SRGB8_ALPHA8;
        } else if (_fb_properties.get_float_color()) {
          if (_fb_properties.get_color_bits() > 16 * 3) {
            gl_format = GL_RGBA32F_ARB;
          } else {
            gl_format = GL_RGBA16F_ARB;
          }
        } else {
          gl_format = GL_RGBA;
        }
        break;
    }
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

/**
 * This function attaches the given texture to the given attachment point.
 */
void CLP(GraphicsBuffer)::
attach_tex(int layer, int view, Texture *attach, GLenum attachpoint) {
  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  if (view >= attach->get_num_views()) {
    attach->set_num_views(view + 1);
  }

  // Create the OpenGL texture object.
  TextureContext *tc = attach->prepare_now(view, glgsg->get_prepared_objects(), glgsg);
  nassertv(tc != nullptr);
  CLP(TextureContext) *gtc = DCAST(CLP(TextureContext), tc);

  glgsg->update_texture(gtc, true);
  gtc->set_active(true);
  _texture_contexts.push_back(gtc);

  // It seems that binding the texture is necessary before binding to a
  // framebuffer attachment.
  glgsg->apply_texture(gtc);

#if !defined(OPENGLES) && defined(SUPPORT_FIXED_FUNCTION)
  if (glgsg->has_fixed_function_pipeline()) {
    GLclampf priority = 1.0f;
    glPrioritizeTextures(1, &gtc->_index, &priority);
  }
#endif

#ifndef OPENGLES
  if (_rb_size_z != 1) {
    // Bind all of the layers of the texture.
    nassertv(glgsg->_glFramebufferTexture != nullptr);
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
  case GL_TEXTURE_2D_ARRAY:
    glgsg->_glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT, attachpoint,
                                      gtc->_index, 0, layer);
    break;
#endif
  default:
    glgsg->_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachpoint,
                                   target, gtc->_index, 0);
  }
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  If we've just rendered into level zero of a
 * mipmapped texture, then all subsequent mipmap levels will now be
 * calculated.
 */
void CLP(GraphicsBuffer)::
generate_mipmaps() {
  if (gl_ignore_mipmaps && !gl_force_mipmaps) {
    return;
  }

  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  // PStatGPUTimer timer(glgsg, _generate_mipmap_pcollector);

  pvector<CLP(TextureContext)*>::iterator it;
  for (it = _texture_contexts.begin(); it != _texture_contexts.end(); ++it) {
    CLP(TextureContext) *gtc = *it;

    if (gtc->_generate_mipmaps) {
      glgsg->generate_mipmaps(gtc);
    }
  }

  report_my_gl_errors();
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void CLP(GraphicsBuffer)::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != nullptr);

  // Resolve Multisample rendering if using it.
  if (_requested_multisamples && _fbo_multisample) {
    resolve_multisamples();
  }

  if (mode == FM_render) {
    copy_to_textures();
  }

  // Unbind the FBO.  TODO: calling bind_fbo is slow, so we should probably
  // move this to begin_frame to prevent unnecessary calls.
  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();
  glgsg->bind_fbo(0);
  _bound_tex_page = -1;

  if (mode == FM_render) {
    generate_mipmaps();
  }

  if (_host != nullptr) {
    _host->end_frame(FM_parasite, current_thread);
  } else {
    glgsg->end_frame(current_thread);
  }

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
  report_my_gl_errors();

  glgsg->pop_group_marker();
}

/**
 *
 */
void CLP(GraphicsBuffer)::
set_size(int x, int y) {
  if (_size.get_x() != x || _size.get_y() != y) {
    _needs_rebuild = true;
  }

  set_size_and_recalc(x, y);
}

/**
 * Called internally when the window is in render-to-a-texture mode and we are
 * in the process of rendering the six faces of a cube map, or any other
 * multi-page texture.  This should do whatever needs to be done to switch the
 * buffer to the indicated page.
 */
void CLP(GraphicsBuffer)::
select_target_tex_page(int page) {
  nassertv(page >= 0 && (size_t)page < _fbo.size());

  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  bool switched_page = (_bound_tex_page != page);

  if (switched_page) {
    if (_bound_tex_page != -1) {
      // Resolve the multisample rendering for the previous face.
      if (_requested_multisamples && _fbo_multisample) {
        resolve_multisamples();
      }
    }

    if (_fbo_multisample != 0) {
      // TODO: re-issue clears?
    } else {
      glgsg->bind_fbo(_fbo[page]);
    }
    _bound_tex_page = page;
  }

  report_my_gl_errors();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool CLP(GraphicsBuffer)::
open_buffer() {
  report_my_gl_errors();

  // Double check that we have a valid gsg
  nassertr(_gsg != nullptr, false);
  if (!_gsg->is_valid()) {
    return false;
  }

  // Count total color buffers.
  int totalcolor =
   (_fb_properties.get_stereo() ? 2 : 1) +
    _fb_properties.get_aux_rgba() +
    _fb_properties.get_aux_hrgba() +
    _fb_properties.get_aux_float();

  // Check for support of relevant extensions.
  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, _gsg, false);
  if (!glgsg->_supports_framebuffer_object) {
    return false;
  }

  if (_rb_context == nullptr) {
    _rb_context = new BufferContext(&(glgsg->_renderbuffer_residency), nullptr);
  }

/*
 * Describe the framebuffer properties of the FBO. Unfortunately, we can't
 * currently predict which formats the implementation will allow us to use at
 * this point, so we'll just have to make some guesses and parrot the rest of
 * the properties back to the user.  When we actually attach the textures,
 * we'll update the properties more appropriately.
 */

  // Rounding the depth bits is not spectacular, but at least we're telling
  // the user *something* about what we're going to get.

  // A lot of code seems to depend on being able to get a color buffer by just
  // setting the rgb_color bit.
  if (_fb_properties.get_color_bits() == 0 &&
      _fb_properties.get_rgb_color()) {
    _fb_properties.set_color_bits(1);
    _fb_properties.set_red_bits(1);
    _fb_properties.set_green_bits(1);
    _fb_properties.set_blue_bits(1);
  }

  // Actually, let's always get a colour buffer for now until we figure out
  // why Intel HD Graphics cards complain otherwise.
  if (gl_force_fbo_color && _fb_properties.get_color_bits() == 0) {
    _fb_properties.set_color_bits(1);
  }

  if (_fb_properties.get_depth_bits() > 24) {
    _fb_properties.set_depth_bits(32);
  } else if (_fb_properties.get_depth_bits() > 16) {
    _fb_properties.set_depth_bits(24);
  } else if (_fb_properties.get_depth_bits() > 0) {
    _fb_properties.set_depth_bits(16);
  } else {
    _fb_properties.set_depth_bits(0);
  }

  // We're not going to get more than this, ever.  At least not until OpenGL
  // introduces 64-bit texture formats.
  if (_fb_properties.get_color_bits() > 96) {
    _fb_properties.set_color_bits(96);
  }
  if (_fb_properties.get_red_bits() > 32) {
    _fb_properties.set_red_bits(32);
  }
  if (_fb_properties.get_green_bits() > 32) {
    _fb_properties.set_green_bits(32);
  }
  if (_fb_properties.get_blue_bits() > 32) {
    _fb_properties.set_blue_bits(32);
  }
  if (_fb_properties.get_alpha_bits() > 32) {
    _fb_properties.set_alpha_bits(32);
  }

  if (_fb_properties.get_float_depth()) {
    // GL_DEPTH_COMPONENT32F seems the only depth float format.
    _fb_properties.set_depth_bits(32);
  }

  // We currently only support color formats this big as float.
  if (_fb_properties.get_color_bits() > 16 * 3) {
    _fb_properties.set_float_color(true);
  }

  if (_fb_properties.get_srgb_color()) {
    // This is the only sRGB color format OpenGL supports.
    _fb_properties.set_rgba_bits(8, 8, 8,
      (_fb_properties.get_alpha_bits() > 0) ? 8 : 0);
    _fb_properties.set_float_color(false);
  }

  if (!_gsg->get_supports_depth_stencil()) {
    // At least we know we won't be getting stencil bits.
    _fb_properties.set_stencil_bits(0);

  } else if (_fb_properties.get_stencil_bits() > 0) {
    // We don't currently support stencil-only targets.
    _fb_properties.set_stencil_bits(8);
    if (_fb_properties.get_depth_bits() < 24) {
      _fb_properties.set_depth_bits(24);
    }
  }

  // Accumulation buffers aren't supported for FBOs.
  _fb_properties.set_accum_bits(0);

  if (glgsg->get_supports_framebuffer_multisample() && glgsg->get_supports_framebuffer_blit()) {
    _requested_multisamples = _fb_properties.get_multisamples();
  } else {
    _requested_multisamples = 0;
  }

#ifndef OPENGLES
  if (glgsg->get_supports_framebuffer_multisample_coverage_nv() && glgsg->get_supports_framebuffer_blit()) {
    _requested_coverage_samples = _fb_properties.get_coverage_samples();
    // Note:  Only 4 and 8 actual samples are supported by the extension, with
    // 8 or 16 coverage samples.
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
#endif

  if (_requested_multisamples > glgsg->_max_fb_samples) {
    _requested_multisamples = glgsg->_max_fb_samples;
  }
  _fb_properties.set_multisamples(_requested_multisamples);
  _fb_properties.set_coverage_samples(_requested_coverage_samples);

  // Update aux settings to reflect the GL_MAX_DRAW_BUFFERS limit, if we
  // exceed it, that is.
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
  if (_host != nullptr) {
    _fb_properties.set_force_hardware(_host->get_fb_properties().get_force_hardware());
    _fb_properties.set_force_software(_host->get_fb_properties().get_force_software());
  }

  _is_valid = true;
  _needs_rebuild = true;
  report_my_gl_errors();

  return true;
}

/**
 * This is normally called only from within make_texture_buffer().  When
 * called on a ParasiteBuffer, it returns the host of that buffer; but when
 * called on some other buffer, it returns the buffer itself.
 */
GraphicsOutput *CLP(GraphicsBuffer)::
get_host() {
  return (_host != nullptr) ? _host : this;
}

/**
 * Closes the buffer right now.  Called from the window thread.
 */
void CLP(GraphicsBuffer)::
close_buffer() {
  _rb_data_size_bytes = 0;
  if (_rb_context != nullptr) {
    _rb_context->update_data_size_bytes(0);
    delete _rb_context;
    _rb_context = nullptr;
  }

  check_host_valid();

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
  }
  // Delete the multisample renderbuffers.
  for (int i=0; i<RTP_COUNT; i++) {
    if (_rbm[i] != 0) {
      glgsg->_glDeleteRenderbuffers(1, &(_rbm[i]));
      _rb[i] = 0;
    }
  }

  _rb_size_x = 0;
  _rb_size_y = 0;
  report_my_gl_errors();

  // Delete the FBO itself.
  if (!_fbo.empty()) {
    glgsg->_glDeleteFramebuffers(_fbo.size(), _fbo.data());
    _fbo.clear();
  }

  report_my_gl_errors();

  // Release the Gsg
  _gsg.clear();

  _is_valid = false;
}

/**
 * Will attempt to use the depth buffer of the input graphics_output.  The
 * buffer sizes must be exactly the same.
 */
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
      // let the input GraphicsOutput know that there is an object sharing its
      // depth buffer
      input_graphics_output->register_shared_depth_buffer(this);
      _shared_depth_buffer = input_graphics_output;
      state = true;
    }
    _needs_rebuild = true;
  }
  report_my_gl_errors();
  return state;
}

/**
 * Discontinue sharing the depth buffer.
 */
void CLP(GraphicsBuffer)::
unshare_depth_buffer() {
  if (_shared_depth_buffer) {
    // let the GraphicsOutput know that this object is no longer sharing its
    // depth buffer
    _shared_depth_buffer->unregister_shared_depth_buffer(this);
    _shared_depth_buffer = 0;
    _needs_rebuild = true;
  }
}

/**
 * Returns true if this particular GraphicsOutput can render directly into a
 * texture, or false if it must always copy-to-texture at the end of each
 * frame to achieve this effect.
 */
bool CLP(GraphicsBuffer)::
get_supports_render_texture() const {
  // FBO-based buffers, by their nature, can always bind-to-texture.
  return true;
}

/**
 * Register/save who is sharing the depth buffer.
 */
void CLP(GraphicsBuffer)::
register_shared_depth_buffer(GraphicsOutput *graphics_output) {
  CLP(GraphicsBuffer) *input_graphics_output;

  input_graphics_output = DCAST (CLP(GraphicsBuffer), graphics_output);
  if (input_graphics_output) {
    // add to list
    _shared_depth_buffer_list.push_back(input_graphics_output);
  }
}

/**
 * Unregister who is sharing the depth buffer.
 */
void CLP(GraphicsBuffer)::
unregister_shared_depth_buffer(GraphicsOutput *graphics_output) {
  CLP(GraphicsBuffer) *input_graphics_output;

  input_graphics_output = DCAST (CLP(GraphicsBuffer), graphics_output);
  if (input_graphics_output) {
    // remove from list
    _shared_depth_buffer_list.remove(input_graphics_output);
  }
}

/**
 * Unregister who is sharing the depth buffer.
 */
void CLP(GraphicsBuffer)::
report_my_errors(int line, const char *file) {
  if (_gsg == 0) {
    GLenum error_code = glGetError();
    if (error_code != GL_NO_ERROR) {
      GLCAT.error() << file << ", line " << line << ": GL error " << (int)error_code << "\n";
    }
  } else {
    CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();
    glgsg->report_my_errors(line, file);
  }
}

/**
 * If the host window has been closed, then this buffer is dead too.
 */
void CLP(GraphicsBuffer)::
check_host_valid() {
  if (_host != nullptr && !_host->is_valid()) {
    _rb_data_size_bytes = 0;
    if (_rb_context != nullptr) {
      // We must delete this object first, because when the GSG destructs, so
      // will the tracker that this context is attached to.
      _rb_context->update_data_size_bytes(0);
      delete _rb_context;
      _rb_context = nullptr;
    }
    _is_valid = false;
    _gsg.clear();
    _host.clear();
  }
}

/**
 * After the frame has been rendered into the multisample buffer, filters it
 * down into the final render buffer.
 */
void CLP(GraphicsBuffer)::
resolve_multisamples() {
  nassertv(_fbo.size() > 0);

  CLP(GraphicsStateGuardian) *glgsg = (CLP(GraphicsStateGuardian) *)_gsg.p();

  PStatGPUTimer timer(glgsg, _resolve_multisample_pcollector);

#ifndef OPENGLES_1
  if (gl_enable_memory_barriers) {
    // Issue memory barriers as necessary to make sure that the texture memory
    // is synchronized before we blit to it.
    pvector<CLP(TextureContext)*>::iterator it;
    for (it = _texture_contexts.begin(); it != _texture_contexts.end(); ++it) {
      CLP(TextureContext) *gtc = *it;

      if (gtc != nullptr && gtc->needs_barrier(GL_FRAMEBUFFER_BARRIER_BIT)) {
        glgsg->issue_memory_barrier(GL_FRAMEBUFFER_BARRIER_BIT);
        // If we've done it for one, we've done it for all.
        break;
      }
    }
  }
#endif

  glgsg->report_my_gl_errors();
  GLuint fbo = _fbo[0];
  if (_bound_tex_page != -1) {
    fbo = _fbo[_bound_tex_page];
  }
  glgsg->_glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, fbo);
  glgsg->_glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, _fbo_multisample);
  glgsg->_current_fbo = fbo;

  // If the depth buffer is shared, resolve it only on the last to render FBO.
  bool do_depth_blit = false;
  if (_rbm[RTP_depth_stencil] != 0 || _rbm[RTP_depth] != 0) {
    if (_shared_depth_buffer) {
      CLP(GraphicsBuffer) *graphics_buffer = nullptr;
      //CLP(GraphicsBuffer) *highest_sort_graphics_buffer = NULL;
      std::list <CLP(GraphicsBuffer) *>::iterator graphics_buffer_iterator;

      int max_sort_order = 0;
      for (graphics_buffer_iterator = _shared_depth_buffer_list.begin();
           graphics_buffer_iterator != _shared_depth_buffer_list.end();
           graphics_buffer_iterator++) {
        graphics_buffer = (*graphics_buffer_iterator);
        if (graphics_buffer) {
          // this call removes the entry from the list
          if (graphics_buffer->get_sort() >= max_sort_order) {
            max_sort_order = graphics_buffer->get_sort();
            //highest_sort_graphics_buffer = graphics_buffer;
          }
        }
      }
      if (max_sort_order == this->get_sort()) {
        do_depth_blit = true;
      }
    } else {
      do_depth_blit = true;
    }
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
  // Now handle the other color buffers.
#ifndef OPENGLES
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
