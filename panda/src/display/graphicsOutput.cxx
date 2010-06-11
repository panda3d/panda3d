// Filename: graphicsOutput.cxx
// Created by:  drose (06Feb04)
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

TypeHandle GraphicsOutput::_type_handle;

PStatCollector GraphicsOutput::_make_current_pcollector("Draw:Make current");
PStatCollector GraphicsOutput::_copy_texture_pcollector("Draw:Copy texture");
PStatCollector GraphicsOutput::_cull_pcollector("Cull");
PStatCollector GraphicsOutput::_draw_pcollector("Draw");

struct CubeFaceDef {
  CubeFaceDef(const char *name, const LPoint3f &look_at, const LVector3f &up) :
    _name(name), _look_at(look_at), _up(up) { }

  const char *_name;
  LPoint3f _look_at;
  LVector3f _up;
};

static CubeFaceDef cube_faces[6] = {
  CubeFaceDef("positive_x", LPoint3f(1, 0, 0), LVector3f(0, -1, 0)),
  CubeFaceDef("negative_x", LPoint3f(-1, 0, 0), LVector3f(0, -1, 0)),
  CubeFaceDef("positive_y", LPoint3f(0, 1, 0), LVector3f(0, 0, 1)),
  CubeFaceDef("negative_y", LPoint3f(0, -1, 0), LVector3f(0, 0, -1)),
  CubeFaceDef("positive_z", LPoint3f(0, 0, 1), LVector3f(0, -1, 0)),
  CubeFaceDef("negative_z", LPoint3f(0, 0, -1), LVector3f(0, -1, 0))
};

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Constructor
//       Access: Protected
//  Description: Normally, the GraphicsOutput constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_window() function.
////////////////////////////////////////////////////////////////////
GraphicsOutput::
GraphicsOutput(GraphicsEngine *engine, GraphicsPipe *pipe,
               const string &name,
               const FrameBufferProperties &fb_prop,
               const WindowProperties &win_prop,
               int flags,
               GraphicsStateGuardian *gsg,
               GraphicsOutput *host) :
  _lock("GraphicsOutput"),
  _cull_window_pcollector(_cull_pcollector, name),
  _draw_window_pcollector(_draw_pcollector, name)
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
  _x_size = _y_size = 0;
  _has_size = win_prop.has_size();
  if (_has_size) {
    _x_size = win_prop.get_x_size();
    _y_size = win_prop.get_y_size();
  }
  _is_valid = false;
  _flip_ready = false;
  _cube_map_index = -1;
  _cube_map_dr = NULL;
  _sort = 0;
  _child_sort = 0;
  _got_child_sort = false;
  _internal_sort_index = 0;
  _active = true;
  _one_shot = false;
  _inverted = window_inverted;
  _red_blue_stereo = false;
  _left_eye_color_mask = 0x0f;
  _right_eye_color_mask = 0x0f;
  _delete_flag = false;
  _texture_card = 0;
  _trigger_copy = false;

  if (_fb_properties.is_single_buffered()) {
    _draw_buffer_type = RenderBuffer::T_front;
  } else {
    _draw_buffer_type = RenderBuffer::T_back;
  }

  // We start out with one DisplayRegion that covers the whole window,
  // which we may use internally for full-window operations like
  // clear() and get_screenshot().
  _default_display_region = make_mono_display_region(0.0f, 1.0f, 0.0f, 1.0f);
  _default_display_region->set_active(false);

  _display_regions_stale = false;

  // By default, each new GraphicsOutput is set up to clear color and
  // depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);
  set_clear_stencil_active(true);

  switch (background_color.get_num_words()) {
  case 1:
    set_clear_color(Colorf(background_color[0], background_color[0], background_color[0], 0.0f));
    break;

  case 2:
    set_clear_color(Colorf(background_color[0], background_color[0], background_color[0], background_color[1]));
    break;

  case 3:
    set_clear_color(Colorf(background_color[0], background_color[1], background_color[2], 0.0f));
    break;

  case 4:
    set_clear_color(Colorf(background_color[0], background_color[1], background_color[2], background_color[3]));
    break;

  default:
    display_cat.warning()
      << "Invalid background-color specification: "
      << background_color.get_string_value() << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Copy Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsOutput::
GraphicsOutput(const GraphicsOutput &) :
  _cull_window_pcollector(_cull_pcollector, "Invalid"),
  _draw_window_pcollector(_draw_pcollector, "Invalid")
{
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Copy Assignment Operator
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
operator = (const GraphicsOutput &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsOutput::
~GraphicsOutput() {
  // The window should be closed by the time we destruct.
  nassertv(!is_valid());

  // We shouldn't have a GraphicsPipe pointer anymore.
  nassertv(_pipe == (GraphicsPipe *)NULL);

  // We don't have to destruct our child display regions explicitly,
  // since they are all reference-counted and will go away when their
  // pointers do.  However, we do need to zero out their pointers to
  // us.
  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    (*dri)->_window = NULL;
  }

  _total_display_regions.clear();
  _active_display_regions.clear();
  _default_display_region = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear_render_textures
//       Access: Published
//  Description: If the GraphicsOutput is currently rendering to
//               a texture, then all textures are dissociated from
//               the GraphicsOuput.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
clear_render_textures() {
  LightMutexHolder holder(_lock);
  throw_event("render-texture-targets-changed");
  _textures.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::add_render_texture
//       Access: Published
//  Description: Creates a new Texture object, suitable for rendering
//               the contents of this buffer into, and appends it to
//               the list of render textures.
//
//               If tex is not NULL, it is the texture that will be
//               set up for rendering into; otherwise, a new Texture
//               object will be created, in which case you may call
//               get_texture() to retrieve the new texture pointer.
//
//               You can specify a bitplane to attach the texture to.
//               the legal choices are:
//
//               * RTP_depth
//               * RTP_depth_stencil
//               * RTP_color
//               * RTP_aux_rgba_0
//               * RTP_aux_rgba_1
//               * RTP_aux_rgba_2
//               * RTP_aux_rgba_3
//
//               If you do not specify a bitplane to attach the
//               texture to, this routine will use a default based
//               on the texture's format:
//
//               * F_depth_component attaches to RTP_depth
//               * F_depth_stencil attaches to RTP_depth_stencil
//               * all other formats attach to RTP_color.
//
//               The texture's format will be changed to match
//               the format of the bitplane to which it is attached.
//               For example, if you pass in an F_rgba texture and
//               order that it be attached to RTP_depth_stencil, it will turn
//               into an F_depth_stencil texture.
//
//               Also see make_texture_buffer(), which is a
//               higher-level interface for preparing
//               render-to-a-texture mode.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
add_render_texture(Texture *tex, RenderTextureMode mode,
                   RenderTexturePlane plane) {

  if (mode == RTM_none) {
    return;
  }
  LightMutexHolder holder(_lock);

  throw_event("render-texture-targets-changed");

  // Create texture if necessary.
  if (tex == (Texture *)NULL) {
    tex = new Texture(get_name());
    tex->set_wrap_u(Texture::WM_clamp);
    tex->set_wrap_v(Texture::WM_clamp);
  } else {
    tex->clear_ram_image();
  }

  // Set it to have no compression by default.  You can restore
  // compression later if you really, really want it; but this freaks
  // out some drivers, and presumably it's a mistake if you have
  // compression enabled for a rendered texture.
  tex->set_compression(Texture::CM_off);

  // Choose a default bitplane.
  if (plane == RTP_COUNT) {
    if (tex->get_format()==Texture::F_depth_stencil) {
      plane = RTP_depth_stencil;
    } else if (tex->get_format()==Texture::F_depth_component) {
      plane = RTP_depth;
    } else {
      plane = RTP_color;
    }
  }

  // Set the texture's format to match the bitplane.
  // (And validate the bitplane, while we're at it).

  if (plane == RTP_depth) {
    tex->set_format(Texture::F_depth_component);
    tex->set_match_framebuffer_format(true);
  } else if (plane == RTP_depth_stencil) {
    tex->set_format(Texture::F_depth_component);
    tex->set_match_framebuffer_format(true);
  } else if ((plane == RTP_color)||
             (plane == RTP_aux_rgba_0)||
             (plane == RTP_aux_rgba_1)||
             (plane == RTP_aux_rgba_2)||
             (plane == RTP_aux_rgba_3)) {
    tex->set_format(Texture::F_rgba);
    tex->set_match_framebuffer_format(true);
  } else if  ((plane == RTP_aux_hrgba_0)||
              (plane == RTP_aux_hrgba_1)||
              (plane == RTP_aux_hrgba_2)||
              (plane == RTP_aux_hrgba_3)) {
    tex->set_format(Texture::F_rgba16);
    tex->set_match_framebuffer_format(true);
  } else

  {
    display_cat.error() <<
      "add_render_texture: invalid bitplane specified.\n";
    return;
  }

  // Go ahead and tell the texture our anticipated size, even if it
  // might be inaccurate (particularly if this is a GraphicsWindow,
  // which has system-imposed restrictions on size).
  tex->set_size_padded(get_x_size(), get_y_size());

  if (mode == RTM_bind_or_copy) {
    // Binding is not supported or it is disabled, so just fall back
    // to copy instead.
    if (!support_render_texture) {
      mode = RTM_copy_texture;
    } else {
      nassertv(_gsg != NULL);
      if (!_gsg->get_supports_render_texture()) {
        mode = RTM_copy_texture;
      }
    }
  }

  if (mode == RTM_bind_or_copy) {
    // If we're still planning on binding, indicate it in texture
    // properly.
    tex->set_render_to_texture(true);
  }

  RenderTexture result;
  result._texture = tex;
  result._plane = plane;
  result._rtm_mode = mode;
  _textures.push_back(result);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::setup_render_texture
//       Access: Published
//  Description: This is a deprecated interface that made sense back
//               when GraphicsOutputs could only render into one
//               texture at a time.  From now on, use
//               clear_render_textures and add_render_texture
//               instead.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_active
//       Access: Published
//  Description: Sets the active flag associated with the
//               GraphicsOutput.  If the GraphicsOutput is marked
//               inactive, nothing is rendered.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_active(bool active) {
  _active = active;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_active
//       Access: Published, Virtual
//  Description: Returns true if the window is ready to be rendered
//               into, false otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
is_active() const {
  return _active && is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_inverted
//       Access: Published
//  Description: Changes the current setting of the inverted flag.
//               When this is true, the scene is rendered into the
//               window upside-down and backwards, that is, inverted
//               as if viewed through a mirror placed on the floor.
//
//               This is primarily intended to support DirectX (and a
//               few buggy OpenGL graphics drivers) that perform a
//               framebuffer-to-texture copy upside-down from the
//               usual OpenGL (and Panda) convention.  Panda will
//               automatically set this flag for offscreen buffers on
//               hardware that is known to do this, to compensate when
//               rendering offscreen into a texture.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_inverted(bool inverted) {
  if (_inverted != inverted) {
    _inverted = inverted;

    if (_y_size != 0) {
      // All of our DisplayRegions need to recompute their pixel
      // positions now.
      TotalDisplayRegions::iterator dri;
      for (dri = _total_display_regions.begin();
           dri != _total_display_regions.end();
           ++dri) {
        (*dri)->compute_pixels(_x_size, _y_size);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_sort
//       Access: Published, Virtual
//  Description: Adjusts the sorting order of this particular
//               GraphicsOutput, relative to other GraphicsOutputs.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_sort(int sort) {
  if (_sort != sort) {
    if (_gsg != (GraphicsStateGuardian *)NULL &&
        _gsg->get_engine() != (GraphicsEngine *)NULL) {
      _gsg->get_engine()->set_window_sort(this, sort);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the indicated
//               sub-rectangle within the window.  The range on all
//               parameters is 0..1.
//
//               If is_stereo() is true for this window, and
//               default-stereo-camera is configured true, this
//               actually makes a StereoDisplayRegion.  Call
//               make_mono_display_region() or
//               make_stereo_display_region() if you want to insist on
//               one or the other.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsOutput::
make_display_region(float l, float r, float b, float t) {
  if (is_stereo() && default_stereo_camera) {
    return make_stereo_display_region(l, r, b, t);
  } else {
    return make_mono_display_region(l, r, b, t);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_mono_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the indicated
//               sub-rectangle within the window.  The range on all
//               parameters is 0..1.
//
//               This always returns a mono DisplayRegion, even if
//               is_stereo() is true.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsOutput::
make_mono_display_region(float l, float r, float b, float t) {
  return add_display_region(new DisplayRegion(this, l, r, b, t));
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_stereo_display_region
//       Access: Published
//  Description: Creates a new DisplayRegion that covers the indicated
//               sub-rectangle within the window.  The range on all
//               parameters is 0..1.
//
//               This always returns a stereo DisplayRegion, even if
//               is_stereo() is false.
////////////////////////////////////////////////////////////////////
StereoDisplayRegion *GraphicsOutput::
make_stereo_display_region(float l, float r, float b, float t) {
  PT(DisplayRegion) left = new DisplayRegion(this, l, r, b, t);
  PT(DisplayRegion) right = new DisplayRegion(this, l, r, b, t);

  PT(StereoDisplayRegion) stereo = new StereoDisplayRegion(this, l, r, b, t,
                                                           left, right);
  add_display_region(stereo);
  add_display_region(left);
  add_display_region(right);

  return stereo;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::remove_display_region
//       Access: Published
//  Description: Removes the indicated DisplayRegion from the window,
//               and destructs it if there are no other references.
//
//               Returns true if the DisplayRegion is found and
//               removed, false if it was not a part of the window.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
remove_display_region(DisplayRegion *display_region) {
  LightMutexHolder holder(_lock);

  nassertr(display_region != _default_display_region, false);

  if (display_region->is_stereo()) {
    StereoDisplayRegion *sdr;
    DCAST_INTO_R(sdr, display_region, false);
    do_remove_display_region(sdr->get_left_eye());
    do_remove_display_region(sdr->get_right_eye());
  }

  return do_remove_display_region(display_region);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::remove_all_display_regions
//       Access: Published
//  Description: Removes all display regions from the window, except
//               the default one that is created with the window.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
remove_all_display_regions() {
  LightMutexHolder holder(_lock);

  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    DisplayRegion *display_region = (*dri);
    if (display_region != _default_display_region) {
      // Let's aggressively clean up the display region too.
      display_region->cleanup();
      display_region->_window = NULL;
    }
  }
  _total_display_regions.clear();
  _total_display_regions.push_back(_default_display_region);
  _display_regions_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_num_display_regions
//       Access: Published
//  Description: Returns the number of DisplayRegions that have
//               been created within the window, active or otherwise.
////////////////////////////////////////////////////////////////////
int GraphicsOutput::
get_num_display_regions() const {
  determine_display_regions();
  int result;
  {
    LightMutexHolder holder(_lock);
    result = _total_display_regions.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_display_region
//       Access: Published
//  Description: Returns the nth DisplayRegion of those that have been
//               created within the window.  This may return NULL if n
//               is out of bounds; particularly likely if the number
//               of display regions has changed since the last call to
//               get_num_display_regions().
////////////////////////////////////////////////////////////////////
PT(DisplayRegion) GraphicsOutput::
get_display_region(int n) const {
  determine_display_regions();
  PT(DisplayRegion) result;
  {
    LightMutexHolder holder(_lock);
    if (n >= 0 && n < (int)_total_display_regions.size()) {
      result = _total_display_regions[n];
    } else {
      result = NULL;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_num_active_display_regions
//       Access: Published
//  Description: Returns the number of active DisplayRegions that have
//               been created within the window.
////////////////////////////////////////////////////////////////////
int GraphicsOutput::
get_num_active_display_regions() const {
  determine_display_regions();
  int result;
  {
    LightMutexHolder holder(_lock);
    result = _active_display_regions.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_active_display_region
//       Access: Published
//  Description: Returns the nth active DisplayRegion of those that
//               have been created within the window.  This may return
//               NULL if n is out of bounds; particularly likely if
//               the number of display regions has changed since the
//               last call to get_num_active_display_regions().
////////////////////////////////////////////////////////////////////
PT(DisplayRegion) GraphicsOutput::
get_active_display_region(int n) const {
  determine_display_regions();
  PT(DisplayRegion) result;
  {
    LightMutexHolder holder(_lock);
    if (n >= 0 && n < (int)_active_display_regions.size()) {
      result = _active_display_regions[n];
    } else {
      result = NULL;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOuput::create_texture_card_vdata
//       Access: Private
//  Description: Generates a GeomVertexData for a texture card.
////////////////////////////////////////////////////////////////////
PT(GeomVertexData) GraphicsOutput::
create_texture_card_vdata(int x, int y) {
  float xhi = 1.0;
  float yhi = 1.0;

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

  vertex.add_data3f(Vertexf::rfu(-1.0f, 0.0f,  1.0f));
  vertex.add_data3f(Vertexf::rfu(-1.0f, 0.0f, -1.0f));
  vertex.add_data3f(Vertexf::rfu( 1.0f, 0.0f,  1.0f));
  vertex.add_data3f(Vertexf::rfu( 1.0f, 0.0f, -1.0f));

  texcoord.add_data2f( 0.0f,  yhi);
  texcoord.add_data2f( 0.0f, 0.0f);
  texcoord.add_data2f(  xhi,  yhi);
  texcoord.add_data2f(  xhi, 0.0f);

  normal.add_data3f(LVector3f::back());
  normal.add_data3f(LVector3f::back());
  normal.add_data3f(LVector3f::back());
  normal.add_data3f(LVector3f::back());

  return vdata;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_size_and_recalc
//       Access: Public
//  Description: Changes the x_size and y_size, then recalculates
//               structures that depend on size.  The recalculation
//               currently includes:
//               - compute_pixels on all the graphics regions.
//               - updating the texture card, if one is present.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_size_and_recalc(int x, int y) {
  _x_size = x;
  _y_size = y;
  _has_size = true;

  int fb_x_size = get_fb_x_size();
  int fb_y_size = get_fb_y_size();

  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    (*dri)->compute_pixels_all_stages(fb_x_size, fb_y_size);
  }

  if (_texture_card != 0) {
    _texture_card->set_vertex_data(create_texture_card_vdata(x, y));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_texture_card
//       Access: Published
//  Description: Returns a PandaNode containing a square polygon.
//               The dimensions are (-1,0,-1) to (1,0,1). The texture
//               coordinates are such that the texture of this
//               GraphicsOutput is aligned properly to the polygon.
//               The GraphicsOutput promises to surgically update
//               the Geom inside the PandaNode if necessary to maintain
//               this invariant.
//
//               Each invocation of this function returns a freshly-
//               allocated PandaNode.  You can therefore safely modify
//               the RenderAttribs of the PandaNode.  The
//               PandaNode is initially textured with the texture
//               of this GraphicOutput.
////////////////////////////////////////////////////////////////////
NodePath GraphicsOutput::
get_texture_card() {
  if (_texture_card == 0) {
    PT(GeomVertexData) vdata = create_texture_card_vdata(_x_size, _y_size);
    PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
    strip->set_shade_model(Geom::SM_uniform);
    strip->add_next_vertices(4);
    strip->close_primitive();
    _texture_card = new Geom(vdata);
    _texture_card->add_primitive(strip);
  }

  PT(GeomNode) gnode = new GeomNode("texture card");
  gnode->add_geom(_texture_card);
  NodePath path(gnode);

  // The texture card, by default, is textured with the first
  // render-to-texture output texture.  Depth and stencil
  // textures are ignored.  The user can freely alter the
  // card's texture attrib.
  for (int i=0; i<count_textures(); i++) {
    Texture *texture = get_texture(i);
    if ((texture->get_format() != Texture::F_depth_stencil)) {
      path.set_texture(texture, 0);
      break;
    }
  }

  return path;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_texture_buffer
//       Access: Published
//  Description: Creates and returns an offscreen buffer for rendering
//               into, the result of which will be a texture suitable
//               for applying to geometry within the scene rendered
//               into this window.
//
//               If tex is not NULL, it is the texture that will be
//               set up for rendering into; otherwise, a new Texture
//               object will be created.  In either case, the target
//               texture can be retrieved from the return value with
//               buffer->get_texture() (assuming the return value is
//               not NULL).
//
//               If to_ram is true, the buffer will be set up to
//               download its contents to the system RAM memory
//               associated with the Texture object, instead of
//               keeping it strictly within texture memory; this is
//               much slower, but it allows using the texture with any
//               GSG.
//
//               This will attempt to be smart about maximizing render
//               performance while minimizing framebuffer waste.  It
//               might return a GraphicsBuffer set to render directly
//               into a texture, if possible; or it might return a
//               ParasiteBuffer that renders into this window.  The
//               return value is NULL if the buffer could not be
//               created for some reason.
//
//               When you are done using the buffer, you should remove
//               it with a call to GraphicsEngine::remove_window() (or
//               set the one_shot flag so it removes itself after one
//               frame).
////////////////////////////////////////////////////////////////////
GraphicsOutput *GraphicsOutput::
make_texture_buffer(const string &name, int x_size, int y_size,
                    Texture *tex, bool to_ram, FrameBufferProperties *fbp) {

  FrameBufferProperties props;
  props.set_rgb_color(1);
  props.set_depth_bits(1);

  if (fbp == NULL) {
    fbp = &props;
  }

  int flags = GraphicsPipe::BF_refuse_window;
  if (textures_power_2 != ATS_none) {
    flags |= GraphicsPipe::BF_size_power_2;
  }
  if (tex != (Texture *)NULL &&
      tex->get_texture_type() == Texture::TT_cube_map) {
    flags |= GraphicsPipe::BF_size_square;
  }

  GraphicsOutput *buffer = get_gsg()->get_engine()->
    make_output(get_gsg()->get_pipe(),
                name, get_child_sort(),
                *fbp, WindowProperties::size(x_size, y_size),
                flags, get_gsg(), get_host());

  if (buffer != (GraphicsOutput *)NULL) {
    buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_bind_or_copy);
    return buffer;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_cube_map
//       Access: Published
//  Description: This is similar to make_texture_buffer() in that it
//               allocates a separate buffer suitable for rendering to
//               a texture that can be assigned to geometry in this
//               window, but in this case, the buffer is set up to
//               render the six faces of a cube map.
//
//               The buffer is automatically set up with six display
//               regions and six cameras, each of which are assigned
//               the indicated draw_mask and parented to the given
//               camera_rig node (which you should then put in your
//               scene to render the cube map from the appropriate
//               point of view).
//
//               You may take the texture associated with the buffer
//               and apply it to geometry, particularly with
//               TexGenAttrib::M_world_cube_map also in effect, to
//               apply a reflection of everything seen by the camera
//               rig.
////////////////////////////////////////////////////////////////////
GraphicsOutput *GraphicsOutput::
make_cube_map(const string &name, int size, NodePath &camera_rig,
              DrawMask camera_mask, bool to_ram, FrameBufferProperties *fbp) {
  if (!to_ram) {
    // Check the limits imposed by the GSG.  (However, if we're
    // rendering the texture to RAM only, these limits may be
    // irrelevant.)
    GraphicsStateGuardian *gsg = get_gsg();
    int max_dimension = gsg->get_max_cube_map_dimension();
    if (max_dimension == 0 || !gsg->get_supports_cube_map()) {
      // The GSG doesn't support cube mapping; too bad for you.
      display_cat.warning()
        << "Cannot make dynamic cube map; GSG does not support cube maps.\n";
      return NULL;
    }
    if (max_dimension > 0) {
      size = min(max_dimension, size);
    }
  }

  // Usually, we want the whole camera_rig to keep itself unrotated
  // with respect to the world coordinate space, so the user can apply
  // TexGenAttrib::M_world_cube_map to the objects on which the cube
  // map texture is applied.  If for some reason the user doesn't want
  // this behavior, he can take this effect off again.
  camera_rig.node()->set_effect(CompassEffect::make(NodePath()));

  PT(Texture) tex = new Texture(name);
  tex->setup_cube_map();
  tex->set_wrap_u(Texture::WM_clamp);
  tex->set_wrap_v(Texture::WM_clamp);
  GraphicsOutput *buffer;

  buffer = make_texture_buffer(name, size, size, tex, to_ram, fbp);

  // We don't need to clear the overall buffer; instead, we'll clear
  // each display region.
  buffer->set_clear_color_active(false);
  buffer->set_clear_depth_active(false);
  buffer->set_clear_stencil_active(false);

  PT(Lens) lens = new PerspectiveLens;
  lens->set_fov(90.0f);

  for (int i = 0; i < 6; i++) {
    PT(Camera) camera = new Camera(cube_faces[i]._name);
    camera->set_lens(lens);
    camera->set_camera_mask(camera_mask);
    NodePath camera_np = camera_rig.attach_new_node(camera);
    camera_np.look_at(cube_faces[i]._look_at, cube_faces[i]._up);

    DisplayRegion *dr;
    dr = buffer->make_display_region();

    dr->set_cube_map_index(i);
    dr->copy_clear_settings(*this);
    dr->set_camera(camera_np);
  }

  return buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_host
//       Access: Public, Virtual
//  Description: This is normally called only from within
//               make_texture_buffer().  When called on a
//               ParasiteBuffer, it returns the host of that buffer;
//               but when called on some other buffer, it returns the
//               buffer itself.
////////////////////////////////////////////////////////////////////
GraphicsOutput *GraphicsOutput::
get_host() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::request_open
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the window (or whatever) open itself or, in general,
//               make itself valid, at the next call to
//               process_events().
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
request_open() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::request_close
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to request that
//               the window (or whatever) close itself or, in general,
//               make itself invalid, at the next call to
//               process_events().  By that time we promise the gsg
//               pointer will be cleared.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
request_close() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::set_close_now
//       Access: Public, Virtual
//  Description: This is called by the GraphicsEngine to insist that
//               the output be closed immediately.  This is only
//               called from the window thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
set_close_now() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::reset_window
//       Access: Protected, Virtual
//  Description: Resets the window framebuffer from its derived
//               children. Does nothing here.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
reset_window(bool swapchain) {
  display_cat.info()
    << "Resetting " << get_type() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear_pipe
//       Access: Protected, Virtual
//  Description: Sets the window's _pipe pointer to NULL; this is
//               generally called only as a precursor to deleting the
//               window.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
clear_pipe() {
  _pipe = (GraphicsPipe *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
begin_frame(FrameMode mode, Thread *current_thread) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
end_frame(FrameMode mode, Thread *current_thread) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::pixel_factor_changed
//       Access: Published, Virtual
//  Description: Called internally when the pixel factor changes.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
pixel_factor_changed() {
  if (_has_size) {
    set_size_and_recalc(_x_size, _y_size);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::prepare_for_deletion
//       Access: Protected
//  Description: Set the delete flag, and do the usual cleanup
//               activities associated with that.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
prepare_for_deletion() {

  _active = false;
  _delete_flag = true;

  // We have to be sure to remove all of the display regions
  // immediately, so that circular reference counts can be cleared
  // up (each display region keeps a pointer to a CullResult,
  // which can hold all sorts of pointers).
  remove_all_display_regions();

  // If we were rendering directly to texture, we can't delete the
  // buffer until the texture is gone too.
  for (int i=0; i<count_textures(); i++) {
    if (get_rtm_mode(i) == RTM_bind_or_copy) {
      _hold_textures.push_back(get_texture(i));
    }
  }

  // We have to be sure to clear the _textures pointers, though, or
  // we'll end up holding a reference to the textures forever.
  clear_render_textures();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::clear
//       Access: Public
//  Description: Clears the entire framebuffer before rendering,
//               according to the settings of get_color_clear_active()
//               and get_depth_clear_active() (inherited from
//               DrawableRegion).
//
//               This function is called only within the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
clear(Thread *current_thread) {
  if (is_any_clear_active()) {
    if (display_cat.is_spam()) {
      display_cat.spam()
        << "clear(): " << get_type() << " "
        << get_name() << " " << (void *)this << "\n";
    }

    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    DisplayRegionPipelineReader dr_reader(_default_display_region, current_thread);
    _gsg->prepare_display_region(&dr_reader, Lens::SC_mono);
    _gsg->clear(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::copy_to_textures
//       Access: Protected
//  Description: For all textures marked RTM_copy_texture,
//               RTM_copy_ram, RTM_triggered_copy_texture, or
//               RTM_triggered_copy_ram, do the necessary copies.
//
//               Returns true if all copies are successful, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
copy_to_textures() {
  bool okflag = true;
  for (int i = 0; i < count_textures(); ++i) {
    RenderTextureMode rtm_mode = get_rtm_mode(i);
    if ((rtm_mode == RTM_none) || (rtm_mode == RTM_bind_or_copy)) {
      continue;
    }

    Texture *texture = get_texture(i);
    PStatTimer timer(_copy_texture_pcollector);
    nassertr(has_texture(), false);

    if ((rtm_mode == RTM_copy_texture)||
        (rtm_mode == RTM_copy_ram)||
        ((rtm_mode == RTM_triggered_copy_texture)&&(_trigger_copy))||
        ((rtm_mode == RTM_triggered_copy_ram)&&(_trigger_copy))) {
      if (display_cat.is_debug()) {
        display_cat.debug()
          << "Copying texture for " << get_name() << " at frame end.\n";
        display_cat.debug()
          << "cube_map_index = " << _cube_map_index << "\n";
      }
      int plane = get_texture_plane(i);
      RenderBuffer buffer(_gsg, DrawableRegion::get_renderbuffer_type(plane));
      if (plane == RTP_color) {
        buffer = _gsg->get_render_buffer(get_draw_buffer_type(),
                                         get_fb_properties());
      }

      bool copied = false;
      if (_cube_map_dr != (DisplayRegion *)NULL) {
        if ((rtm_mode == RTM_copy_ram)||(rtm_mode == RTM_triggered_copy_ram)) {
          copied =
            _gsg->framebuffer_copy_to_ram(texture, _cube_map_index,
                                          _cube_map_dr, buffer);
        } else {
          copied =
            _gsg->framebuffer_copy_to_texture(texture, _cube_map_index,
                                              _cube_map_dr, buffer);
        }
      } else {
        if ((rtm_mode == RTM_copy_ram)||(rtm_mode == RTM_triggered_copy_ram)) {
          copied =
            _gsg->framebuffer_copy_to_ram(texture, _cube_map_index,
                                          _default_display_region, buffer);
        } else {
          copied =
            _gsg->framebuffer_copy_to_texture(texture, _cube_map_index,
                                              _default_display_region, buffer);
        }
      }
      if (!copied) {
        okflag = false;
      }
    }
  }
  _trigger_copy = false;

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::change_scenes
//       Access: Public
//  Description: Called by the GraphicsEngine when the window is about
//               to change to another DisplayRegion.  This exists
//               mainly to provide a callback for switching the cube
//               map face, if we are rendering to the different faces
//               of a cube map.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
change_scenes(DisplayRegionPipelineReader *new_dr) {
  int new_cube_map_index = new_dr->get_cube_map_index();
  if (new_cube_map_index != -1 &&
      new_cube_map_index != _cube_map_index) {
    int old_cube_map_index = _cube_map_index;
    DisplayRegion *old_cube_map_dr = _cube_map_dr;
    _cube_map_index = new_cube_map_index;
    _cube_map_dr = new_dr->get_object();

    for (int i=0; i<count_textures(); i++) {
      Texture *texture = get_texture(i);
      RenderTextureMode rtm_mode = get_rtm_mode(i);
      if (rtm_mode != RTM_none) {
        if (rtm_mode == RTM_bind_or_copy) {
          // In render-to-texture mode, switch the rendering backend to
          // the new cube map face, so that the subsequent frame will be
          // rendered to the new face.

          select_cube_map(new_cube_map_index);

        } else if (old_cube_map_index != -1) {
          // In copy-to-texture mode, copy the just-rendered framebuffer
          // to the old cube map face.
          nassertv(old_cube_map_dr != (DisplayRegion *)NULL);
          if (display_cat.is_debug()) {
            display_cat.debug()
              << "Copying texture for " << get_name() << " at scene change.\n";
            display_cat.debug()
              << "cube_map_index = " << old_cube_map_index << "\n";
          }
          RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type(),
                                                        get_fb_properties());
          if (rtm_mode == RTM_copy_ram) {
            _gsg->framebuffer_copy_to_ram(texture, old_cube_map_index,
                                          old_cube_map_dr, buffer);
          } else {
            _gsg->framebuffer_copy_to_texture(texture, old_cube_map_index,
                                              old_cube_map_dr, buffer);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::select_cube_map
//       Access: Public, Virtual
//  Description: Called internally when the window is in
//               render-to-a-texture mode and we are in the process of
//               rendering the six faces of a cube map.  This should
//               do whatever needs to be done to switch the buffer to
//               the indicated face.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
select_cube_map(int) {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
begin_flip() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::ready_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip when it is command but not actually flip
//
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
ready_flip() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
end_flip() {
  _flip_ready = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing in the window thread is
//               appropriate for this output object each frame.
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
process_events() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::add_display_region
//       Access: Private
//  Description: Called by one of the make_display_region() methods to
//               add the new DisplayRegion to the list.
////////////////////////////////////////////////////////////////////
DisplayRegion *GraphicsOutput::
add_display_region(DisplayRegion *display_region) {
  LightMutexHolder holder(_lock);
  _total_display_regions.push_back(display_region);
  _display_regions_stale = true;

  return display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::do_remove_display_region
//       Access: Private
//  Description: Internal implementation of remove_display_region.
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
do_remove_display_region(DisplayRegion *display_region) {
  nassertr(display_region != _default_display_region, false);

  PT(DisplayRegion) drp = display_region;
  TotalDisplayRegions::iterator dri =
    find(_total_display_regions.begin(), _total_display_regions.end(), drp);
  if (dri != _total_display_regions.end()) {
    // Let's aggressively clean up the display region too.
    display_region->cleanup();
    display_region->_window = NULL;
    _total_display_regions.erase(dri);
    if (display_region->is_active()) {
      _display_regions_stale = true;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::do_determine_display_regions
//       Access: Private
//  Description: Re-sorts the list of active DisplayRegions within
//               the window.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
do_determine_display_regions() {
  LightMutexHolder holder(_lock);
  _display_regions_stale = false;

  _active_display_regions.clear();
  _active_display_regions.reserve(_total_display_regions.size());

  int index = 0;
  TotalDisplayRegions::const_iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    DisplayRegion *display_region = (*dri);
    if (display_region->is_active()) {
      _active_display_regions.push_back(display_region);
      display_region->set_active_index(index);
      ++index;
    } else {
      display_region->set_active_index(-1);
    }
  }

  stable_sort(_active_display_regions.begin(), _active_display_regions.end(), IndirectLess<DisplayRegion>());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::FrameMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, GraphicsOutput::FrameMode fm) {
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::share_depth_buffer
//       Access: Published, Virtual
//  Description: Will attempt to use the depth buffer of the input
//               graphics_output. The buffer sizes must be exactly
//               the same.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
share_depth_buffer(GraphicsOutput *graphics_output) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::unshare_depth_buffer
//       Access: Published, Virtual
//  Description: Discontinue sharing the depth buffer.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
unshare_depth_buffer() {

}
