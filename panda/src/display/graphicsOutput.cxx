/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsOutput.cxx
 * @author drose
 * @date 2004-02-06
 */

#include "graphicsOutput.h"
#include "graphicsPipe.h"
#include "graphicsEngine.h"
#include "graphicsWindow.h"
#include "config_display.h"
#include "lightMutexHolder.h"
#include "renderBuffer.h"
#include "indirectLess.h"
#include "pStatTimer.h"
#include "configVariableBool.h"
#include "camera.h"
#include "displayRegion.h"
#include "lens.h"
#include "perspectiveLens.h"
#include "pointerTo.h"
#include "compassEffect.h"
#include "geom.h"
#include "geomNode.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "throw_event.h"
#include "config_gobj.h"

using std::string;

TypeHandle GraphicsOutput::_type_handle;

PStatCollector GraphicsOutput::_make_current_pcollector("Draw:Make current");
PStatCollector GraphicsOutput::_copy_texture_pcollector("Draw:Copy texture");
PStatCollector GraphicsOutput::_cull_pcollector("Cull");
PStatCollector GraphicsOutput::_draw_pcollector("Draw");

struct CubeFaceDef {
  CubeFaceDef(const char *name, const LPoint3 &look_at, const LVector3 &up) :
    _name(name), _look_at(look_at), _up(up) { }

  const char *_name;
  LPoint3 _look_at;
  LVector3 _up;
};

static CubeFaceDef cube_faces[6] = {
  CubeFaceDef("positive_x", LPoint3(1, 0, 0), LVector3(0, -1, 0)),
  CubeFaceDef("negative_x", LPoint3(-1, 0, 0), LVector3(0, -1, 0)),
  CubeFaceDef("positive_y", LPoint3(0, 1, 0), LVector3(0, 0, 1)),
  CubeFaceDef("negative_y", LPoint3(0, -1, 0), LVector3(0, 0, -1)),
  CubeFaceDef("positive_z", LPoint3(0, 0, 1), LVector3(0, -1, 0)),
  CubeFaceDef("negative_z", LPoint3(0, 0, -1), LVector3(0, -1, 0))
};

/**
 * Normally, the GraphicsOutput constructor is not called directly; these are
 * created instead via the GraphicsEngine::make_window() function.
 */
GraphicsOutput::
GraphicsOutput(GraphicsEngine *engine, GraphicsPipe *pipe,
               const string &name,
               const FrameBufferProperties &fb_prop,
               const WindowProperties &win_prop,
               int flags,
               GraphicsStateGuardian *gsg,
               GraphicsOutput *host,
               bool default_stereo_flags) :
  _lock("GraphicsOutput"),
  _cull_window_pcollector(_cull_pcollector, name),
  _draw_window_pcollector(_draw_pcollector, name),
  _clear_window_pcollector(_draw_window_pcollector, "Clear"),
  _size(0, 0)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _engine = engine;
  _pipe = pipe;
  _gsg = gsg;
  _host = host;
  _fb_properties = fb_prop;
  _name = name;
  _creation_flags = flags;
  _has_size = win_prop.has_size();
  _is_nonzero_size = false;
  if (_has_size) {
    _size = win_prop.get_size();
    _is_nonzero_size = (_size[0] > 0 && _size[1] > 0);
  }
  if (_creation_flags & GraphicsPipe::BF_size_track_host) {
    // If we're tracking the host size, we assume we'll be nonzero eventually.
    _is_nonzero_size = true;
  }

  _is_valid = false;
  _flip_ready = false;
  _target_tex_page = -1;
  _prev_page_dr = nullptr;
  _sort = 0;
  _child_sort = 0;
  _got_child_sort = false;
  _internal_sort_index = 0;
  _inverted = window_inverted;
  _swap_eyes = swap_eyes;
  _red_blue_stereo = false;
  _left_eye_color_mask = 0x0f;
  _right_eye_color_mask = 0x0f;
  _side_by_side_stereo = false;
  _sbs_left_dimensions.set(0.0f, 1.0f, 0.0f, 1.0f);
  _sbs_right_dimensions.set(0.0f, 1.0f, 0.0f, 1.0f);
  _delete_flag = false;

  if (_fb_properties.is_single_buffered()) {
    _draw_buffer_type = RenderBuffer::T_front;
  } else {
    _draw_buffer_type = RenderBuffer::T_back;
  }

  if (default_stereo_flags) {
    // Check the config variables to see if we should make this a "stereo"
    // buffer or window.
    _red_blue_stereo = red_blue_stereo && !fb_prop.is_stereo();
    if (_red_blue_stereo) {
      _left_eye_color_mask = parse_color_mask(red_blue_stereo_colors.get_word(0));
      _right_eye_color_mask = parse_color_mask(red_blue_stereo_colors.get_word(1));
    }
    _side_by_side_stereo = side_by_side_stereo && !fb_prop.is_stereo();
    if (_side_by_side_stereo) {
      _sbs_left_dimensions.set(sbs_left_dimensions[0], sbs_left_dimensions[1],
                               sbs_left_dimensions[2], sbs_left_dimensions[3]);
      _sbs_right_dimensions.set(sbs_right_dimensions[0], sbs_right_dimensions[1],
                                sbs_right_dimensions[2], sbs_right_dimensions[3]);
    }
  }

  // We start out with one DisplayRegion that covers the whole window, which
  // we may use internally for full-window operations like clear() and
  // get_screenshot().
  _overlay_display_region = make_mono_display_region(0.0f, 1.0f, 0.0f, 1.0f);
  _overlay_display_region->set_active(false);
  _overlay_display_region->set_scissor_enabled(false);

  // Make sure the "active" flag is set true for pipeline stage 0.
  {
    CDWriter cdata(_cycler, true);
    cdata->_active = true;
  }

  // By default, each new GraphicsOutput is set up to clear color and depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);
  set_clear_stencil_active(true);
  set_clear_color(background_color.get_value());
}

/**
 *
 */
GraphicsOutput::
~GraphicsOutput() {
  // The window should be closed by the time we destruct.
  nassertv(!is_valid());

  // We shouldn't have a GraphicsPipe pointer anymore.
  nassertv(_pipe == nullptr);

  // We don't have to destruct our child display regions explicitly, since
  // they are all reference-counted and will go away when their pointers do.
  // However, we do need to zero out their pointers to us.
  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    (*dri)->_window = nullptr;
  }

  _total_display_regions.clear();
  _overlay_display_region = nullptr;
}

/**
 * If the GraphicsOutput is currently rendering to a texture, then all
 * textures are dissociated from the GraphicsOuput.
 */
void GraphicsOutput::
clear_render_textures() {
  CDWriter cdata(_cycler, true);
  cdata->_textures.clear();
  ++(cdata->_textures_seq);
  throw_event("render-texture-targets-changed");
}

/**
 * Creates a new Texture object, suitable for rendering the contents of this
 * buffer into, and appends it to the list of render textures.
 *
 * If tex is not NULL, it is the texture that will be set up for rendering
 * into; otherwise, a new Texture object will be created, in which case you
 * may call get_texture() to retrieve the new texture pointer.
 *
 * You can specify a bitplane to attach the texture to.  the legal choices
 * are:
 *
 * - RTP_depth
 * - RTP_depth_stencil
 * - RTP_color
 * - RTP_aux_rgba_0
 * - RTP_aux_rgba_1
 * - RTP_aux_rgba_2
 * - RTP_aux_rgba_3
 *
 * If you do not specify a bitplane to attach the texture to, this routine
 * will use a default based on the texture's format:
 *
 * - F_depth_component attaches to RTP_depth
 * - F_depth_stencil attaches to RTP_depth_stencil
 * - all other formats attach to RTP_color.
 *
 * The texture's format will be changed to match the format of the bitplane to
 * which it is attached.  For example, if you pass in an F_rgba texture and
 * order that it be attached to RTP_depth_stencil, it will turn into an
 * F_depth_stencil texture.
 *
 * Also see make_texture_buffer(), which is a higher-level interface for
 * preparing render-to-a-texture mode.
 */
void GraphicsOutput::
add_render_texture(Texture *tex, RenderTextureMode mode,
                   RenderTexturePlane plane) {

  if (mode == RTM_none) {
    return;
  }
  LightMutexHolder holder(_lock);

  // Create texture if necessary.
  if (tex == nullptr) {
    tex = new Texture(get_name());
    tex->set_wrap_u(SamplerState::WM_clamp);
    tex->set_wrap_v(SamplerState::WM_clamp);
  } else {
    tex->clear_ram_image();
  }

  // Set it to have no compression by default.  You can restore compression
  // later if you really, really want it; but this freaks out some drivers,
  // and presumably it's a mistake if you have compression enabled for a
  // rendered texture.
  tex->set_compression(Texture::CM_off);

  // Choose a default bitplane.
  if (plane == RTP_COUNT) {
    switch (tex->get_format()) {
    case Texture::F_depth_stencil:
      plane = RTP_depth_stencil;
      break;

    case Texture::F_depth_component:
    case Texture::F_depth_component16:
    case Texture::F_depth_component24:
    case Texture::F_depth_component32:
      plane = RTP_depth;
      break;

    default:
      plane = RTP_color;
      break;
    }
  }

  // Set the texture's format to match the bitplane.  (And validate the
  // bitplane, while we're at it).

  if (plane == RTP_depth) {
    _fb_properties.setup_depth_texture(tex);
    tex->set_match_framebuffer_format(true);

  } else if (plane == RTP_depth_stencil) {
    tex->set_format(Texture::F_depth_stencil);
    if (_fb_properties.get_float_depth()) {
      tex->set_component_type(Texture::T_float);
    } else {
      tex->set_component_type(Texture::T_unsigned_int_24_8);
    }
    tex->set_match_framebuffer_format(true);

  } else if (plane == RTP_color ||
             plane == RTP_aux_rgba_0 ||
             plane == RTP_aux_rgba_1 ||
             plane == RTP_aux_rgba_2 ||
             plane == RTP_aux_rgba_3) {
    _fb_properties.setup_color_texture(tex);
    tex->set_match_framebuffer_format(true);

  } else if (plane == RTP_aux_hrgba_0 ||
             plane == RTP_aux_hrgba_1 ||
             plane == RTP_aux_hrgba_2 ||
             plane == RTP_aux_hrgba_3) {
    tex->set_format(Texture::F_rgba16);
    tex->set_match_framebuffer_format(true);

  } else if (plane == RTP_aux_float_0 ||
             plane == RTP_aux_float_1 ||
             plane == RTP_aux_float_2 ||
             plane == RTP_aux_float_3) {
    tex->set_format(Texture::F_rgba32);
    tex->set_component_type(Texture::T_float);
    tex->set_match_framebuffer_format(true);

  } else {
    display_cat.error() <<
      "add_render_texture: invalid bitplane specified.\n";
    return;
  }

  // Go ahead and tell the texture our anticipated size, even if it might be
  // inaccurate (particularly if this is a GraphicsWindow, which has system-
  // imposed restrictions on size).
  tex->set_size_padded(get_x_size(), get_y_size(), tex->get_z_size());

  if (_fb_properties.is_stereo() && plane == RTP_color) {
    if (tex->get_num_views() < 2) {
      tex->set_num_views(2);
    }
  }

  if (!support_render_texture || !get_supports_render_texture()) {
    // Binding is not supported or it is disabled, so just fall back to copy
    // instead.
    if (mode == RTM_bind_or_copy) {
      mode = RTM_copy_texture;
    } else if (mode == RTM_bind_layered) {
      // We can't fallback to copy, because that doesn't work for layered
      // textures.  The best thing we can do is raise an error message.
      display_cat.error() <<
        "add_render_texture: RTM_bind_layered was requested but "
        "render-to-texture is not supported or has been disabled!\n";
    }
  }

  if (mode == RTM_bind_layered && _gsg != nullptr && !_gsg->get_supports_geometry_shaders()) {
    // Layered FBOs require a geometry shader to write to any but the first
    // layer.
    display_cat.warning() <<
      "add_render_texture: RTM_bind_layered was requested but "
      "geometry shaders are not supported!\n";
  }

  if (mode == RTM_bind_or_copy || mode == RTM_bind_layered) {
    // If we're still planning on binding, indicate it in texture properly.
    tex->set_render_to_texture(true);
  }

  CDWriter cdata(_cycler, true);
  RenderTexture result;
  result._texture = tex;
  result._plane = plane;
  result._rtm_mode = mode;
  cdata->_textures.push_back(result);
  ++(cdata->_textures_seq);

  throw_event("render-texture-targets-changed");
}

/**
 * This is a deprecated interface that made sense back when GraphicsOutputs
 * could only render into one texture at a time.  From now on, use
 * clear_render_textures and add_render_texture instead.
 *
 * @deprecated Use add_render_texture() instead.
 */
void GraphicsOutput::
setup_render_texture(Texture *tex, bool allow_bind, bool to_ram) {
  display_cat.warning() <<
    "Using deprecated setup_render_texture interface.\n";
  clear_render_textures();
  if (to_ram) {
    add_render_texture(tex, RTM_copy_ram);
  } else if (allow_bind) {
    add_render_texture(tex, RTM_bind_or_copy);
  } else {
    add_render_texture(tex, RTM_copy_texture);
  }
}

/**
 * Sets the active flag associated with the GraphicsOutput.  If the
 * GraphicsOutput is marked inactive, nothing is rendered.
 */
void GraphicsOutput::
set_active(bool active) {
  CDLockedReader cdata(_cycler);
  if (cdata->_active != active) {
    CDWriter cdataw(((GraphicsOutput *)this)->_cycler, cdata, true);
    cdataw->_active = active;
  }
}

/**
 * Returns true if the window is ready to be rendered into, false otherwise.
 */
bool GraphicsOutput::
is_active() const {
  if (!is_valid()) {
    return false;
  }

  CDLockedReader cdata(_cycler);
  if (!cdata->_active) {
    return false;
  }

  if (cdata->_one_shot_frame != -1) {
    // If one_shot is in effect, then we are active only for the one indicated
    // frame.
    if (cdata->_one_shot_frame != ClockObject::get_global_clock()->get_frame_count()) {
      return false;
    } else {
      return true;
    }
  }

  // If the window has a clear value set, it is active.
  if (is_any_clear_active()) {
    return true;
  }

  // If we triggered a copy operation, it is also active.
  if (_trigger_copy) {
    return true;
  }

  // The window is active if at least one display region is active.
  if (cdata->_active_display_regions_stale) {
    CDWriter cdataw(((GraphicsOutput *)this)->_cycler, cdata, false);
    ((GraphicsOutput *)this)->do_determine_display_regions(cdataw);
    return !cdataw->_active_display_regions.empty();
  } else {
    return !cdata->_active_display_regions.empty();
  }
}

/**
 * Changes the current setting of the one-shot flag.  When this is true, the
 * GraphicsOutput will render the current frame and then automatically set
 * itself inactive.  This is particularly useful for buffers that are created
 * for the purposes of render-to-texture, for static textures that don't need
 * to be continually re-rendered once they have been rendered the first time.
 *
 * Setting the buffer inactive is not the same thing as destroying it.  You
 * are still responsible for passing this buffer to
 * GraphicsEngine::remove_window() when you no longer need the texture, in
 * order to clean up fully.  (However, you should not call remove_window() on
 * this buffer while the texture is still needed, because depending on the
 * render-to-texture mechanism in use, this may invalidate the texture
 * contents.)
 */
void GraphicsOutput::
set_one_shot(bool one_shot) {
  CDWriter cdata(_cycler, true);
  if (one_shot) {
    cdata->_one_shot_frame = ClockObject::get_global_clock()->get_frame_count();
  } else {
    cdata->_one_shot_frame = -1;
  }
}

/**
 * Returns the current setting of the one-shot flag.  When this is true, the
 * GraphicsOutput will automatically set itself inactive after the next frame.
 */
bool GraphicsOutput::
get_one_shot() const {
  CDReader cdata(_cycler);
  return (cdata->_one_shot_frame == ClockObject::get_global_clock()->get_frame_count());
}

/**
 * Changes the current setting of the inverted flag.  When this is true, the
 * scene is rendered into the window upside-down and backwards, that is,
 * inverted as if viewed through a mirror placed on the floor.
 *
 * This is primarily intended to support DirectX (and a few buggy OpenGL
 * graphics drivers) that perform a framebuffer-to-texture copy upside-down
 * from the usual OpenGL (and Panda) convention.  Panda will automatically set
 * this flag for offscreen buffers on hardware that is known to do this, to
 * compensate when rendering offscreen into a texture.
 */
void GraphicsOutput::
set_inverted(bool inverted) {
  if (_inverted != inverted) {
    _inverted = inverted;

    if (get_y_size() != 0) {
      // All of our DisplayRegions need to recompute their pixel positions
      // now.
      TotalDisplayRegions::iterator dri;
      for (dri = _total_display_regions.begin();
           dri != _total_display_regions.end();
           ++dri) {
        (*dri)->compute_pixels(get_x_size(), get_y_size());
      }
    }
  }
}

/**
 * Enables side-by-side stereo mode on this particular window.  When side-by-
 * side stereo mode is in effect, DisplayRegions that have the "left" channel
 * set will render on the part of the window specified by sbs_left_dimensions
 * (typically the left half: (0, 0.5, 0, 1)), while DisplayRegions that have
 * the "right" channel set will render on the part of the window specified by
 * sbs_right_dimensions (typically the right half: (0.5, 1, 0, 1)).
 *
 * This is commonly used in a dual-monitor mode, where a window is opened that
 * spans two monitors, and each monitor represents a different eye.
 */
void GraphicsOutput::
set_side_by_side_stereo(bool side_by_side_stereo) {
  LVecBase4 left, right;
  left.set(sbs_left_dimensions[0], sbs_left_dimensions[1],
           sbs_left_dimensions[2], sbs_left_dimensions[3]);
  right.set(sbs_right_dimensions[0], sbs_right_dimensions[1],
            sbs_right_dimensions[2], sbs_right_dimensions[3]);
  set_side_by_side_stereo(side_by_side_stereo, left, right);
}

/**
 * Enables side-by-side stereo mode on this particular window.  When side-by-
 * side stereo mode is in effect, DisplayRegions that have the "left" channel
 * set will render on the part of the window specified by sbs_left_dimensions
 * (typically the left half: (0, 0.5, 0, 1)), while DisplayRegions that have
 * the "right" channel set will render on the part of the window specified by
 * sbs_right_dimensions (typically the right half: (0.5, 1, 0, 1)).
 *
 * This is commonly used in a dual-monitor mode, where a window is opened that
 * spans two monitors, and each monitor represents a different eye.
 */
void GraphicsOutput::
set_side_by_side_stereo(bool side_by_side_stereo,
                        const LVecBase4 &sbs_left_dimensions,
                        const LVecBase4 &sbs_right_dimensions) {
  _side_by_side_stereo = side_by_side_stereo;
  if (_side_by_side_stereo) {
    _sbs_left_dimensions = sbs_left_dimensions;
    _sbs_right_dimensions = sbs_right_dimensions;
  } else {
    _sbs_left_dimensions.set(0.0f, 1.0f, 0.0f, 1.0f);
    _sbs_right_dimensions.set(0.0f, 1.0f, 0.0f, 1.0f);
  }
}

/**
 * Returns the current setting of the delete flag.  When this is true, the
 * GraphicsOutput will automatically be removed before the beginning of the
 * next frame by the GraphicsEngine.
 */
bool GraphicsOutput::
get_delete_flag() const {
  // We only delete the window or buffer automatically when it is no longer
  // associated with a texture.
  for (int i = 0; i < (int)_hold_textures.size(); i++) {
    if (_hold_textures[i].is_valid_pointer()) {
      return false;
    }
  }

  return _delete_flag;
}

/**
 * Adjusts the sorting order of this particular GraphicsOutput, relative to
 * other GraphicsOutputs.
 */
void GraphicsOutput::
set_sort(int sort) {
  if (_sort != sort) {
    if (_gsg != nullptr &&
        _gsg->get_engine() != nullptr) {
      _gsg->get_engine()->set_window_sort(this, sort);
    }
  }
}

/**
 * Creates a new DisplayRegion that covers the indicated sub-rectangle within
 * the window.  The range on all parameters is 0..1.
 *
 * If is_stereo() is true for this window, and default-stereo-camera is
 * configured true, this actually makes a StereoDisplayRegion.  Call
 * make_mono_display_region() or make_stereo_display_region() if you want to
 * insist on one or the other.
 */
DisplayRegion *GraphicsOutput::
make_display_region(const LVecBase4 &dimensions) {
  if (is_stereo() && default_stereo_camera) {
    return make_stereo_display_region(dimensions);
  } else {
    return make_mono_display_region(dimensions);
  }
}

/**
 * Creates a new DisplayRegion that covers the indicated sub-rectangle within
 * the window.  The range on all parameters is 0..1.
 *
 * This generally returns a mono DisplayRegion, even if is_stereo() is true.
 * However, if side-by-side stereo is enabled, this will return a
 * StereoDisplayRegion whose two eyes are both set to SC_mono.  (This is
 * necessary because in side-by-side stereo mode, it is necessary to draw even
 * mono DisplayRegions twice).
 */
DisplayRegion *GraphicsOutput::
make_mono_display_region(const LVecBase4 &dimensions) {
  if (_side_by_side_stereo) {
    StereoDisplayRegion *dr = make_stereo_display_region(dimensions);
    dr->get_left_eye()->set_stereo_channel(Lens::SC_mono);
    dr->get_right_eye()->set_stereo_channel(Lens::SC_mono);
    return dr;
  }

  return new DisplayRegion(this, dimensions);
}

/**
 * Creates a new DisplayRegion that covers the indicated sub-rectangle within
 * the window.  The range on all parameters is 0..1.
 *
 * This always returns a stereo DisplayRegion, even if is_stereo() is false.
 */
StereoDisplayRegion *GraphicsOutput::
make_stereo_display_region(const LVecBase4 &dimensions) {
  PT(DisplayRegion) left, right;

  if (_side_by_side_stereo) {
    // On a side-by-side stereo window, each eye gets the corresponding
    // dimensions of its own sub-region.
    PN_stdfloat left_l = _sbs_left_dimensions[0];
    PN_stdfloat left_b = _sbs_left_dimensions[2];
    PN_stdfloat left_w = _sbs_left_dimensions[1] - _sbs_left_dimensions[0];
    PN_stdfloat left_h = _sbs_left_dimensions[3] - _sbs_left_dimensions[2];
    LVecBase4 left_dimensions(dimensions[0] * left_w + left_l,
                              dimensions[1] * left_w + left_l,
                              dimensions[2] * left_h + left_b,
                              dimensions[3] * left_h + left_b);
    left = new DisplayRegion(this, left_dimensions);

    PN_stdfloat right_l = _sbs_right_dimensions[0];
    PN_stdfloat right_b = _sbs_right_dimensions[2];
    PN_stdfloat right_w = _sbs_right_dimensions[1] - _sbs_right_dimensions[0];
    PN_stdfloat right_h = _sbs_right_dimensions[3] - _sbs_right_dimensions[2];
    LVecBase4 right_dimensions(dimensions[0] * right_w + right_l,
                               dimensions[1] * right_w + right_l,
                               dimensions[2] * right_h + right_b,
                               dimensions[3] * right_h + right_b);
    right = new DisplayRegion(this, right_dimensions);

    if (_swap_eyes) {
      DisplayRegion *t = left;
      left = right;
      right = t;
    }

  } else {
    // Not a side-by-side stereo window; thus, both the left and right eyes
    // are the same region: the region specified.
    left = new DisplayRegion(this, dimensions);
    right = new DisplayRegion(this, dimensions);

    // In this case, we assume that the two eyes will share the same depth
    // buffer, which means the right eye should clear the depth buffer by
    // default.
    if (get_clear_depth_active()) {
      right->set_clear_depth_active(true);
    }
    if (get_clear_stencil_active()) {
      right->set_clear_stencil_active(true);
    }
  }

  PT(StereoDisplayRegion) stereo = new StereoDisplayRegion(this, dimensions,
                                                           left, right);

  return stereo;
}

/**
 * Removes the indicated DisplayRegion from the window, and destructs it if
 * there are no other references.
 *
 * Returns true if the DisplayRegion is found and removed, false if it was not
 * a part of the window.
 */
bool GraphicsOutput::
remove_display_region(DisplayRegion *display_region) {
  LightMutexHolder holder(_lock);

  nassertr(display_region != _overlay_display_region, false);

  if (display_region->is_stereo()) {
    StereoDisplayRegion *sdr;
    DCAST_INTO_R(sdr, display_region, false);
    do_remove_display_region(sdr->get_left_eye());
    do_remove_display_region(sdr->get_right_eye());
  }

  return do_remove_display_region(display_region);
}

/**
 * Removes all display regions from the window, except the default one that is
 * created with the window.
 */
void GraphicsOutput::
remove_all_display_regions() {
  LightMutexHolder holder(_lock);

  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    DisplayRegion *display_region = (*dri);
    if (display_region != _overlay_display_region) {
      // Let's aggressively clean up the display region too.
      display_region->cleanup();
      display_region->_window = nullptr;
    }
  }
  _total_display_regions.clear();
  _total_display_regions.push_back(_overlay_display_region);

  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    cdata->_active_display_regions_stale = true;
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

/**
 * Replaces the special "overlay" DisplayRegion that is created for each
 * window or buffer.  See get_overlay_display_region().  This must be a new
 * DisplayRegion that has already been created for this window, for instance
 * via a call to make_mono_display_region().  You are responsible for ensuring
 * that the new DisplayRegion covers the entire window.  The previous overlay
 * display region is not automatically removed; you must explicitly call
 * remove_display_region() on it after replacing it with this method, if you
 * wish it to be removed.
 *
 * Normally, there is no reason to change the overlay DisplayRegion, so this
 * method should be used only in very unusual circumstances.
 */
void GraphicsOutput::
set_overlay_display_region(DisplayRegion *display_region) {
  nassertv(display_region->get_window() == this);
  _overlay_display_region = display_region;
}

/**
 * Returns the number of DisplayRegions that have been created within the
 * window, active or otherwise.
 */
int GraphicsOutput::
get_num_display_regions() const {
  LightMutexHolder holder(_lock);
  return _total_display_regions.size();
}

/**
 * Returns the nth DisplayRegion of those that have been created within the
 * window.  This may return NULL if n is out of bounds; particularly likely if
 * the number of display regions has changed since the last call to
 * get_num_display_regions().
 */
PT(DisplayRegion) GraphicsOutput::
get_display_region(int n) const {
  determine_display_regions();
  PT(DisplayRegion) result;
  {
    LightMutexHolder holder(_lock);
    if (n >= 0 && n < (int)_total_display_regions.size()) {
      result = _total_display_regions[n];
    } else {
      result = nullptr;
    }
  }
  return result;
}

/**
 * Returns the number of active DisplayRegions that have been created within
 * the window.
 */
int GraphicsOutput::
get_num_active_display_regions() const {
  determine_display_regions();
  CDReader cdata(_cycler);
  return cdata->_active_display_regions.size();
}

/**
 * Returns the nth active DisplayRegion of those that have been created within
 * the window.  This may return NULL if n is out of bounds; particularly
 * likely if the number of display regions has changed since the last call to
 * get_num_active_display_regions().
 */
PT(DisplayRegion) GraphicsOutput::
get_active_display_region(int n) const {
  determine_display_regions();

  CDReader cdata(_cycler);
  if (n >= 0 && n < (int)cdata->_active_display_regions.size()) {
    return cdata->_active_display_regions[n];
  }
  return nullptr;
}

/**
 * Creates and returns an offscreen buffer for rendering into, the result of
 * which will be a texture suitable for applying to geometry within the scene
 * rendered into this window.
 *
 * If tex is not NULL, it is the texture that will be set up for rendering
 * into; otherwise, a new Texture object will be created.  In either case, the
 * target texture can be retrieved from the return value with
 * buffer->get_texture() (assuming the return value is not NULL).
 *
 * If to_ram is true, the buffer will be set up to download its contents to
 * the system RAM memory associated with the Texture object, instead of
 * keeping it strictly within texture memory; this is much slower, but it
 * allows using the texture with any GSG.
 *
 * This will attempt to be smart about maximizing render performance while
 * minimizing framebuffer waste.  It might return a GraphicsBuffer set to
 * render directly into a texture, if possible; or it might return a
 * ParasiteBuffer that renders into this window.  The return value is NULL if
 * the buffer could not be created for some reason.
 *
 * When you are done using the buffer, you should remove it with a call to
 * GraphicsEngine::remove_window().
 */
GraphicsOutput *GraphicsOutput::
make_texture_buffer(const string &name, int x_size, int y_size,
                    Texture *tex, bool to_ram, FrameBufferProperties *fbp) {

  FrameBufferProperties props;
  props.set_rgb_color(1);
  props.set_color_bits(1);
  props.set_alpha_bits(1);
  props.set_depth_bits(1);

  if (fbp == nullptr) {
    fbp = &props;
  }

  int flags = GraphicsPipe::BF_refuse_window;
  if (textures_power_2 != ATS_none) {
    flags |= GraphicsPipe::BF_size_power_2;
  }
  if (tex != nullptr &&
      tex->get_texture_type() == Texture::TT_cube_map) {
    flags |= GraphicsPipe::BF_size_square;
  }

  GraphicsOutput *buffer = get_gsg()->get_engine()->
    make_output(get_gsg()->get_pipe(),
                name, get_child_sort(),
                *fbp, WindowProperties::size(x_size, y_size),
                flags, get_gsg(), get_host());

  if (buffer != nullptr) {
    if (buffer->get_gsg() == nullptr ||
        buffer->get_gsg()->get_prepared_objects() != get_gsg()->get_prepared_objects()) {
      // If the newly-created buffer doesn't share texture objects with the
      // current GSG, then we will have to force the texture copy to go
      // through RAM.
      to_ram = true;
    }

    buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_bind_or_copy);
    return buffer;
  }

  return nullptr;
}

/**
 * This is similar to make_texture_buffer() in that it allocates a separate
 * buffer suitable for rendering to a texture that can be assigned to geometry
 * in this window, but in this case, the buffer is set up to render the six
 * faces of a cube map.
 *
 * The buffer is automatically set up with six display regions and six
 * cameras, each of which are assigned the indicated draw_mask and parented to
 * the given camera_rig node (which you should then put in your scene to
 * render the cube map from the appropriate point of view).
 *
 * You may take the texture associated with the buffer and apply it to
 * geometry, particularly with TexGenAttrib::M_world_cube_map also in effect,
 * to apply a reflection of everything seen by the camera rig.
 */
GraphicsOutput *GraphicsOutput::
make_cube_map(const string &name, int size, NodePath &camera_rig,
              DrawMask camera_mask, bool to_ram, FrameBufferProperties *fbp) {
  if (!to_ram) {
    // Check the limits imposed by the GSG.  (However, if we're rendering the
    // texture to RAM only, these limits may be irrelevant.)
    GraphicsStateGuardian *gsg = get_gsg();
    int max_dimension = gsg->get_max_cube_map_dimension();
    if (max_dimension == 0 || !gsg->get_supports_cube_map()) {
      // The GSG doesn't support cube mapping; too bad for you.
      display_cat.warning()
        << "Cannot make dynamic cube map; GSG does not support cube maps.\n";
      return nullptr;
    }
    if (max_dimension > 0) {
      size = std::min(max_dimension, size);
    }
  }

  // Usually, we want the whole camera_rig to keep itself unrotated with
  // respect to the world coordinate space, so the user can apply
  // TexGenAttrib::M_world_cube_map to the objects on which the cube map
  // texture is applied.  If for some reason the user doesn't want this
  // behavior, he can take this effect off again.
  camera_rig.node()->set_effect(CompassEffect::make(NodePath()));

  PT(Texture) tex = new Texture(name);
  tex->setup_cube_map();
  tex->set_wrap_u(SamplerState::WM_clamp);
  tex->set_wrap_v(SamplerState::WM_clamp);
  GraphicsOutput *buffer;

  buffer = make_texture_buffer(name, size, size, tex, to_ram, fbp);

  // We don't need to clear the overall buffer; instead, we'll clear each
  // display region.
  buffer->set_clear_color_active(false);
  buffer->set_clear_depth_active(false);
  buffer->set_clear_stencil_active(false);

  PT(Lens) lens = new PerspectiveLens(90, 90);

  for (int i = 0; i < 6; i++) {
    PT(Camera) camera = new Camera(cube_faces[i]._name);
    camera->set_lens(lens);
    camera->set_camera_mask(camera_mask);
    NodePath camera_np = camera_rig.attach_new_node(camera);
    camera_np.look_at(cube_faces[i]._look_at, cube_faces[i]._up);

    DisplayRegion *dr;
    dr = buffer->make_display_region();

    dr->set_target_tex_page(i);
    dr->copy_clear_settings(*this);
    dr->set_camera(camera_np);
  }

  return buffer;
}

/**
 * Returns a PandaNode containing a square polygon.  The dimensions are
 * (-1,0,-1) to (1,0,1). The texture coordinates are such that the texture of
 * this GraphicsOutput is aligned properly to the polygon.  The GraphicsOutput
 * promises to surgically update the Geom inside the PandaNode if necessary to
 * maintain this invariant.
 *
 * Each invocation of this function returns a freshly- allocated PandaNode.
 * You can therefore safely modify the RenderAttribs of the PandaNode.  The
 * PandaNode is initially textured with the texture of this GraphicOutput.
 */
NodePath GraphicsOutput::
get_texture_card() {
  if (_texture_card == nullptr) {
    PT(GeomVertexData) vdata = create_texture_card_vdata(get_x_size(), get_y_size());
    PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
    strip->set_shade_model(Geom::SM_uniform);
    strip->add_next_vertices(4);
    strip->close_primitive();
    PT(Geom) geom = new Geom(vdata);
    geom->add_primitive(strip);
    _texture_card = new GeomNode("texture card");
    _texture_card->add_geom(geom);
  }

  NodePath path("texture card");
  path.node()->add_child(_texture_card);

  // The texture card, by default, is textured with the first render-to-
  // texture output texture.  Depth and stencil textures are ignored.  The
  // user can freely alter the card's texture attrib.
  CDReader cdata(_cycler);
  RenderTextures::const_iterator ri;
  for (ri = cdata->_textures.begin(); ri != cdata->_textures.end(); ++ri) {
    Texture *texture = (*ri)._texture;
    if ((texture->get_format() != Texture::F_depth_stencil)) {
      path.set_texture(texture, 0);
      break;
    }
  }

  return path;
}

/**
 * Will attempt to use the depth buffer of the input graphics_output.  The
 * buffer sizes must be exactly the same.
 */
bool GraphicsOutput::
share_depth_buffer(GraphicsOutput *graphics_output) {
  return false;
}

/**
 * Discontinue sharing the depth buffer.
 */
void GraphicsOutput::
unshare_depth_buffer() {
}

/**
 * Returns true if this particular GraphicsOutput can render directly into a
 * texture, or false if it must always copy-to-texture at the end of each
 * frame to achieve this effect.
 */
bool GraphicsOutput::
get_supports_render_texture() const {
  return false;
}

/**
 * Returns true if a frame has been rendered and needs to be flipped, false
 * otherwise.
 */
bool GraphicsOutput::
flip_ready() const {
  return _flip_ready;
}

/**
 * This is normally called only from within make_texture_buffer().  When
 * called on a ParasiteBuffer, it returns the host of that buffer; but when
 * called on some other buffer, it returns the buffer itself.
 */
GraphicsOutput *GraphicsOutput::
get_host() {
  return this;
}

/**
 * This is called by the GraphicsEngine to request that the window (or
 * whatever) open itself or, in general, make itself valid, at the next call
 * to process_events().
 */
void GraphicsOutput::
request_open() {
}

/**
 * This is called by the GraphicsEngine to request that the window (or
 * whatever) close itself or, in general, make itself invalid, at the next
 * call to process_events().  By that time we promise the gsg pointer will be
 * cleared.
 */
void GraphicsOutput::
request_close() {
}

/**
 * This is called by the GraphicsEngine to insist that the output be closed
 * immediately.  This is only called from the window thread.
 */
void GraphicsOutput::
set_close_now() {
}

/**
 * Resets the window framebuffer from its derived children.  Does nothing
 * here.
 */
void GraphicsOutput::
reset_window(bool swapchain) {
  display_cat.info()
    << "Resetting " << get_type() << "\n";
}

/**
 * Sets the window's _pipe pointer to NULL; this is generally called only as a
 * precursor to deleting the window.
 */
void GraphicsOutput::
clear_pipe() {
  _pipe = nullptr;
}

/**
 * Changes the x_size and y_size, then recalculates structures that depend on
 * size.  The recalculation currently includes: - compute_pixels on all the
 * graphics regions.  - updating the texture card, if one is present.
 */
void GraphicsOutput::
set_size_and_recalc(int x, int y) {
  _size.set(x, y);
  _has_size = true;

  _is_nonzero_size = (x > 0 && y > 0);

  int fb_x_size = get_fb_x_size();
  int fb_y_size = get_fb_y_size();

  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    (*dri)->compute_pixels_all_stages(fb_x_size, fb_y_size);
  }

  if (_texture_card != nullptr && _texture_card->get_num_geoms() > 0) {
    _texture_card->modify_geom(0)->set_vertex_data(create_texture_card_vdata(x, y));
  }
}

/**
 * Clears the entire framebuffer before rendering, according to the settings
 * of get_color_clear_active() and get_depth_clear_active() (inherited from
 * DrawableRegion).
 *
 * This function is called only within the draw thread.
 */
void GraphicsOutput::
clear(Thread *current_thread) {
  if (is_any_clear_active()) {
    if (display_cat.is_spam()) {
      display_cat.spam()
        << "clear(): " << get_type() << " "
        << get_name() << " " << (void *)this << "\n";
    }

    nassertv(_gsg != nullptr);

    DisplayRegionPipelineReader dr_reader(_overlay_display_region, current_thread);
    _gsg->prepare_display_region(&dr_reader);
    _gsg->clear(this);
  }
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool GraphicsOutput::
begin_frame(FrameMode mode, Thread *current_thread) {
  return false;
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void GraphicsOutput::
end_frame(FrameMode mode, Thread *current_thread) {
}

/**
 * Called by the GraphicsEngine when the window is about to change to another
 * DisplayRegion.  This exists mainly to provide a callback for switching the
 * cube map face, if we are rendering to the different faces of a cube map.
 */
void GraphicsOutput::
change_scenes(DisplayRegionPipelineReader *new_dr) {
  int new_target_tex_page = new_dr->get_target_tex_page();

  if (new_target_tex_page != -1 && new_target_tex_page != _target_tex_page) {

    if (new_target_tex_page == -1) {
      new_target_tex_page = 0;
    }
    int old_target_tex_page = _target_tex_page;
    DisplayRegion *old_page_dr = _prev_page_dr;
    _target_tex_page = new_target_tex_page;
    _prev_page_dr = new_dr->get_object();

    CDReader cdata(_cycler);
    RenderTextures::const_iterator ri;
    for (ri = cdata->_textures.begin(); ri != cdata->_textures.end(); ++ri) {
      RenderTextureMode rtm_mode = (*ri)._rtm_mode;
      RenderTexturePlane plane = (*ri)._plane;
      Texture *texture = (*ri)._texture;
      if (rtm_mode != RTM_none) {
        if (rtm_mode == RTM_bind_or_copy || rtm_mode == RTM_bind_layered) {
          // In render-to-texture mode, switch the rendering backend to the
          // new page, so that the subsequent frame will be rendered to the
          // correct page.
          select_target_tex_page(_target_tex_page);

        } else if (old_target_tex_page != -1) {
          // In copy-to-texture mode, copy the just-rendered framebuffer to
          // the old texture page.

          nassertv(old_page_dr != nullptr);
          if (display_cat.is_debug()) {
            display_cat.debug()
              << "Copying texture for " << get_name() << " at scene change.\n";
            display_cat.debug()
              << "target_tex_page = " << old_target_tex_page << "\n";
          }
          RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type(),
                                                        get_fb_properties());

          if (plane == RTP_color && _fb_properties.is_stereo()) {
            // We've got two texture views to copy.
            RenderBuffer left(_gsg, buffer._buffer_type & ~RenderBuffer::T_right);
            RenderBuffer right(_gsg, buffer._buffer_type & ~RenderBuffer::T_left);

            if (rtm_mode == RTM_copy_ram) {
              _gsg->framebuffer_copy_to_ram(texture, 0, old_target_tex_page,
                                            old_page_dr, left);
              _gsg->framebuffer_copy_to_ram(texture, 1, old_target_tex_page,
                                            old_page_dr, right);
            } else {
              _gsg->framebuffer_copy_to_texture(texture, 0, old_target_tex_page,
                                                old_page_dr, left);
              _gsg->framebuffer_copy_to_texture(texture, 1, old_target_tex_page,
                                                old_page_dr, right);
            }
          } else {
            if (rtm_mode == RTM_copy_ram) {
              _gsg->framebuffer_copy_to_ram(texture, 0, old_target_tex_page,
                                            old_page_dr, buffer);
            } else {
              _gsg->framebuffer_copy_to_texture(texture, 0, old_target_tex_page,
                                                old_page_dr, buffer);
            }
          }
        }
      }
    }
  }
}

/**
 * Called internally when the window is in render-to-a-texture mode and we are
 * in the process of rendering the six faces of a cube map, or any other
 * multi-page texture.  This should do whatever needs to be done to switch the
 * buffer to the indicated page.
 */
void GraphicsOutput::
select_target_tex_page(int) {
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip at the next video
 * sync, but it should not wait.
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void GraphicsOutput::
begin_flip() {
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip when it is command
 * but not actually flip
 *
 */
void GraphicsOutput::
ready_flip() {
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void GraphicsOutput::
end_flip() {
  _flip_ready = false;
}

/**
 * Do whatever processing in the window thread is appropriate for this output
 * object each frame.
 *
 * This function is called only within the window thread.
 */
void GraphicsOutput::
process_events() {
}

/**
 * Called internally when the pixel factor changes.
 */
void GraphicsOutput::
pixel_factor_changed() {
  if (_has_size) {
    set_size_and_recalc(get_x_size(), get_y_size());
  }
}

/**
 * Set the delete flag, and do the usual cleanup activities associated with
 * that.
 */
void GraphicsOutput::
prepare_for_deletion() {
  CDWriter cdata(_cycler, true);
  cdata->_active = false;

  // If we were rendering directly to texture, we can't delete the buffer
  // until all the textures are gone too.
  RenderTextures::iterator ri;
  for (ri = cdata->_textures.begin(); ri != cdata->_textures.end(); ++ri) {
    if ((*ri)._rtm_mode == RTM_bind_or_copy || (*ri)._rtm_mode == RTM_bind_layered) {
      _hold_textures.push_back((*ri)._texture);
    }
  }
  cdata->_textures.clear();

  _delete_flag = true;

  // We have to be sure to remove all of the display regions immediately, so
  // that circular reference counts can be cleared up (each display region
  // keeps a pointer to a CullResult, which can hold all sorts of pointers).
  remove_all_display_regions();
}

/**
 * If any textures are marked RTM_bind_or_copy, change them to
 * RTM_copy_texture.  This does not change textures that are set to
 * RTM_bind_layered, as layered framebuffers aren't supported with
 * RTM_copy_texture.
 */
void GraphicsOutput::
promote_to_copy_texture() {
  CDLockedReader cdata(_cycler);
  RenderTextures::const_iterator ri;

  bool any_bind = false;
  for (ri = cdata->_textures.begin(); ri != cdata->_textures.end(); ++ri) {
    if ((*ri)._rtm_mode == RTM_bind_or_copy) {
      any_bind = true;
      break;
    }
  }
  if (any_bind) {
    CDWriter cdataw(((GraphicsOutput *)this)->_cycler, cdata, true);
    RenderTextures::iterator ri;
    for (ri = cdataw->_textures.begin(); ri != cdataw->_textures.end(); ++ri) {
      if ((*ri)._rtm_mode == RTM_bind_or_copy) {
        (*ri)._rtm_mode = RTM_copy_texture;
      }
    }
  }
}

/**
 * For all textures marked RTM_copy_texture, RTM_copy_ram,
 * RTM_triggered_copy_texture, or RTM_triggered_copy_ram, do the necessary
 * copies.
 *
 * Returns true if all copies are successful, false otherwise.
 */
bool GraphicsOutput::
copy_to_textures() {
  bool okflag = true;

  CDReader cdata(_cycler);
  RenderTextures::const_iterator ri;
  for (ri = cdata->_textures.begin(); ri != cdata->_textures.end(); ++ri) {
    RenderTextureMode rtm_mode = (*ri)._rtm_mode;
    if ((rtm_mode == RTM_none) || (rtm_mode == RTM_bind_or_copy)) {
      continue;
    }

    Texture *texture = (*ri)._texture;
    PStatTimer timer(_copy_texture_pcollector);

    if ((rtm_mode == RTM_copy_texture)||
        (rtm_mode == RTM_copy_ram)||
        ((rtm_mode == RTM_triggered_copy_texture)&&(_trigger_copy))||
        ((rtm_mode == RTM_triggered_copy_ram)&&(_trigger_copy))) {
      if (display_cat.is_debug()) {
        display_cat.debug()
          << "Copying texture for " << get_name() << " at frame end.\n";
        display_cat.debug()
          << "target_tex_page = " << _target_tex_page << "\n";
      }
      RenderTexturePlane plane = (*ri)._plane;
      RenderBuffer buffer(_gsg, DrawableRegion::get_renderbuffer_type(plane));
      if (plane == RTP_color) {
        buffer = _gsg->get_render_buffer(get_draw_buffer_type(),
                                         get_fb_properties());
      }

      bool copied = false;
      DisplayRegion *dr = _overlay_display_region;
      if (_prev_page_dr != nullptr) {
        dr = _prev_page_dr;
      }

      if (plane == RTP_color && _fb_properties.is_stereo()) {
        // We've got two texture views to copy.
        RenderBuffer left(_gsg, buffer._buffer_type & ~RenderBuffer::T_right);
        RenderBuffer right(_gsg, buffer._buffer_type & ~RenderBuffer::T_left);

        if ((rtm_mode == RTM_copy_ram)||(rtm_mode == RTM_triggered_copy_ram)) {
          copied = _gsg->framebuffer_copy_to_ram(texture, 0, _target_tex_page,
                                                 dr, left);
          copied = _gsg->framebuffer_copy_to_ram(texture, 1, _target_tex_page,
                                                 dr, right) && copied;
        } else {
          copied = _gsg->framebuffer_copy_to_texture(texture, 0, _target_tex_page,
                                                     dr, left);
          copied = _gsg->framebuffer_copy_to_texture(texture, 1, _target_tex_page,
                                                     dr, right) && copied;
        }
      } else {
        if ((rtm_mode == RTM_copy_ram)||(rtm_mode == RTM_triggered_copy_ram)) {
          copied = _gsg->framebuffer_copy_to_ram(texture, 0, _target_tex_page,
                                                 dr, buffer);
        } else {
          copied = _gsg->framebuffer_copy_to_texture(texture, 0, _target_tex_page,
                                                     dr, buffer);
        }
      }
      if (!copied) {
        okflag = false;
      }
    }
  }
  if (_trigger_copy != nullptr) {
    _trigger_copy->set_result(nullptr);
    _trigger_copy = nullptr;
  }

  return okflag;
}

/**
 * Generates a GeomVertexData for a texture card.
 */
PT(GeomVertexData) GraphicsOutput::
create_texture_card_vdata(int x, int y) {
  PN_stdfloat xhi = 1.0;
  PN_stdfloat yhi = 1.0;

  if (Texture::get_textures_power_2() != ATS_none) {
    int xru = Texture::up_to_power_2(x);
    int yru = Texture::up_to_power_2(y);
    xhi = (x * 1.0f) / xru;
    yhi = (y * 1.0f) / yru;
  }

  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3n3t2();

  PT(GeomVertexData) vdata = new GeomVertexData
    ("card", format, Geom::UH_static);

  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
  GeomVertexWriter normal(vdata, InternalName::get_normal());

  vertex.add_data3(LVertex::rfu(-1.0f, 0.0f,  1.0f));
  vertex.add_data3(LVertex::rfu(-1.0f, 0.0f, -1.0f));
  vertex.add_data3(LVertex::rfu( 1.0f, 0.0f,  1.0f));
  vertex.add_data3(LVertex::rfu( 1.0f, 0.0f, -1.0f));

  texcoord.add_data2( 0.0f,  yhi);
  texcoord.add_data2( 0.0f, 0.0f);
  texcoord.add_data2(  xhi,  yhi);
  texcoord.add_data2(  xhi, 0.0f);

  normal.add_data3(LVector3::back());
  normal.add_data3(LVector3::back());
  normal.add_data3(LVector3::back());
  normal.add_data3(LVector3::back());

  return vdata;
}

/**
 * Called by the DisplayRegion constructor to add the new DisplayRegion to the
 * list.
 */
DisplayRegion *GraphicsOutput::
add_display_region(DisplayRegion *display_region) {
  LightMutexHolder holder(_lock);
  CDWriter cdata(_cycler, true);
  cdata->_active_display_regions_stale = true;

  _total_display_regions.push_back(display_region);

  return display_region;
}

/**
 * Internal implementation of remove_display_region.  Assumes the lock is
 * already held.
 */
bool GraphicsOutput::
do_remove_display_region(DisplayRegion *display_region) {
  nassertr(display_region != _overlay_display_region, false);

  PT(DisplayRegion) drp = display_region;
  TotalDisplayRegions::iterator dri =
    find(_total_display_regions.begin(), _total_display_regions.end(), drp);
  if (dri != _total_display_regions.end()) {
    // Let's aggressively clean up the display region too.
    display_region->cleanup();
    display_region->_window = nullptr;
    _total_display_regions.erase(dri);

    OPEN_ITERATE_ALL_STAGES(_cycler) {
      CDStageWriter cdata(_cycler, pipeline_stage);
      cdata->_active_display_regions_stale = true;
    }
    CLOSE_ITERATE_ALL_STAGES(_cycler);
    return true;
  }

  return false;
}

/**
 * Re-sorts the list of active DisplayRegions within the window.
 */
void GraphicsOutput::
do_determine_display_regions(GraphicsOutput::CData *cdata) {
  cdata->_active_display_regions_stale = false;

  cdata->_active_display_regions.clear();
  cdata->_active_display_regions.reserve(_total_display_regions.size());

  int index = 0;
  TotalDisplayRegions::const_iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    DisplayRegion *display_region = (*dri);
    if (display_region->is_active()) {
      cdata->_active_display_regions.push_back(display_region);
      display_region->set_active_index(index);
      ++index;
    } else {
      display_region->set_active_index(-1);
    }
  }

  std::stable_sort(cdata->_active_display_regions.begin(),
                   cdata->_active_display_regions.end(),
                   IndirectLess<DisplayRegion>());
}

/**
 * Parses one of the keywords in the red-blue-stereo-colors Config.prc
 * variable, and returns the corresponding bitmask.
 *
 * These bitmask values are taken from ColorWriteAttrib.
 */
unsigned int GraphicsOutput::
parse_color_mask(const string &word) {
  unsigned int result = 0;
  vector_string components;
  tokenize(word, components, "|");

  vector_string::const_iterator ci;
  for (ci = components.begin(); ci != components.end(); ++ci) {
    string w = downcase(*ci);
    if (w == "red" || w == "r") {
      result |= 0x001;

    } else if (w == "green" || w == "g") {
      result |= 0x002;

    } else if (w == "blue" || w == "b") {
      result |= 0x004;

    } else if (w == "yellow" || w == "y") {
      result |= 0x003;

    } else if (w == "magenta" || w == "m") {
      result |= 0x005;

    } else if (w == "cyan" || w == "c") {
      result |= 0x006;

    } else if (w == "alpha" || w == "a") {
      result |= 0x008;

    } else if (w == "off") {

    } else {
      display_cat.warning()
        << "Invalid color in red-blue-stereo-colors: " << (*ci) << "\n";
    }
  }

  return result;
}

/**
 *
 */
GraphicsOutput::CData::
CData() {
  // The default is *not* active, so the entire pipeline stage is initially
  // populated with inactive outputs.  Pipeline stage 0 is set to active in
  // the constructor.
  _active = false;
  _one_shot_frame = -1;
  _active_display_regions_stale = false;
}

/**
 *
 */
GraphicsOutput::CData::
CData(const GraphicsOutput::CData &copy) :
  _textures(copy._textures),
  _active(copy._active),
  _one_shot_frame(copy._one_shot_frame),
  _active_display_regions(copy._active_display_regions),
  _active_display_regions_stale(copy._active_display_regions_stale)
{
}

/**
 *
 */
CycleData *GraphicsOutput::CData::
make_copy() const {
  return new CData(*this);
}

/**
 *
 */
std::ostream &
operator << (std::ostream &out, GraphicsOutput::FrameMode fm) {
  switch (fm) {
  case GraphicsOutput::FM_render:
    return out << "render";
  case GraphicsOutput::FM_parasite:
    return out << "parasite";
  case GraphicsOutput::FM_refresh:
    return out << "refresh";
  }

  return out << "(**invalid GraphicsOutput::FrameMode(" << (int)fm << ")**)";
}
