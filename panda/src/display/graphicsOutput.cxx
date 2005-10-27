// Filename: graphicsOutput.cxx
// Created by:  drose (06Feb04)
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
////////////////////////////////////////////////////////////////////

#include "graphicsOutput.h"
#include "graphicsPipe.h"
#include "graphicsEngine.h"
#include "graphicsWindow.h"
#include "config_display.h"
#include "mutexHolder.h"
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

TypeHandle GraphicsOutput::_type_handle;

PStatCollector GraphicsOutput::_make_current_pcollector("Draw:Make current");
PStatCollector GraphicsOutput::_copy_texture_pcollector("Draw:Copy texture");

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
GraphicsOutput(GraphicsPipe *pipe, GraphicsStateGuardian *gsg, 
               const string &name) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
  _pipe = pipe;
  _gsg = gsg;
  _name = name;
  _x_size = 0;
  _y_size = 0;
  _has_size = false;
  _is_valid = false;
  _flip_ready = false;
  _needs_context = true;
  _cube_map_index = -1;
  _cube_map_dr = NULL;
  _sort = 0;
  _internal_sort_index = 0;
  _active = true;
  _one_shot = false;
  _inverted = window_inverted;
  _delete_flag = false;
  _texture_card = 0;
  _trigger_copy = false;

  int mode = gsg->get_properties().get_frame_buffer_mode();
  if ((mode & FrameBufferProperties::FM_buffer) == FrameBufferProperties::FM_single_buffer) {
    // Single buffered; we must draw into the front buffer.
    _draw_buffer_type = RenderBuffer::T_front;
  }

  // We start out with one DisplayRegion that covers the whole window,
  // which we may use internally for full-window operations like
  // clear() and get_screenshot().
  _default_display_region = make_display_region(0.0f, 1.0f, 0.0f, 1.0f);
  _default_display_region->set_active(false);

  _display_regions_stale = false;

  // By default, each new GraphicsOutput is set up to clear color and
  // depth.
  set_clear_color_active(true);
  set_clear_depth_active(true);

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
GraphicsOutput(const GraphicsOutput &) {
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
  MutexHolder holder(_lock);
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
//               object will be created (in which case you may call
//               get_texture() to retrieve the new texture pointer
//               later).
//
//               Also see make_texture_buffer(), which is a
//               higher-level interface for preparing
//               render-to-a-texture mode.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
add_render_texture(Texture *tex, RenderTextureMode mode) {
  if (mode == RTM_none) {
    return;
  }
  MutexHolder holder(_lock);

  if (tex == (Texture *)NULL) {
    tex = new Texture(get_name());
    tex->set_wrap_u(Texture::WM_clamp);
    tex->set_wrap_v(Texture::WM_clamp);
  } else {
    tex->clear_ram_image();
  }
  tex->set_match_framebuffer_format(true);
  
  // Go ahead and tell the texture our anticipated size, even if it
  // might be inaccurate (particularly if this is a GraphicsWindow,
  // which has system-imposed restrictions on size).
  tex->set_x_size(get_x_size());
  tex->set_y_size(get_y_size());
  
  RenderTexture result;
  result._texture = tex;
  result._rtm_mode = mode;
  _textures.push_back(result);
  
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  set_inverted(_gsg->get_copy_texture_inverted());

  // Sanity check that we don't have two textures of the same type.
  int count_stencil_textures = 0;
  int count_depth_textures = 0;
  int count_color_textures = 0;
  for (int i=0; i<count_textures(); i++) {
    Texture::Format fmt = get_texture(i)->get_format();
    if (fmt == Texture::F_depth_component) {
      count_depth_textures += 1;
    } else if (fmt == Texture::F_stencil_index) {
      count_stencil_textures += 1;
    } else {
      count_color_textures += 1;
    }
  }
  if ((count_color_textures > 1)||
      (count_depth_textures > 1)||
      (count_stencil_textures > 1)) {
    display_cat.error() <<
      "Currently, each GraphicsOutput can only render to one color texture, "
      "one depth texture, and one stencil texture at a time.  RTM aborted.\n";
    clear_render_textures();
  }
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
//       Access: Published
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
  MutexHolder holder(_lock);

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
//     Function: GraphicsOutput::remove_all_display_regions
//       Access: Published
//  Description: Removes all display regions from the window, except
//               the default one that is created with the window.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
remove_all_display_regions() {
  MutexHolder holder(_lock);

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
    MutexHolder holder(_lock);
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
    MutexHolder holder(_lock);
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
    MutexHolder holder(_lock);
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
    MutexHolder holder(_lock);
    if (n >= 0 && n < (int)_active_display_regions.size()) {
      result = _active_display_regions[n];
    } else {
      result = NULL;
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: create_texture_card_vdata
//       Access: Static
//  Description: Generates a GeomVertexData for a texture card.
////////////////////////////////////////////////////////////////////
static PT(GeomVertexData)
create_texture_card_vdata(int x, int y)
{
  int xru = Texture::up_to_power_2(x);
  int yru = Texture::up_to_power_2(y);
  
  float xhi = (x * 1.0f) / xru;
  float yhi = (y * 1.0f) / yru;
  
  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3n3cpt2();
  
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
  
  TotalDisplayRegions::iterator dri;
  for (dri = _total_display_regions.begin(); 
       dri != _total_display_regions.end(); 
       ++dri) {
    (*dri)->compute_pixels(x,y);
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
    if ((texture->get_format() != Texture::F_depth_component) &&
        (texture->get_format() != Texture::F_stencil_index)) {
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
                    Texture *tex, bool to_ram) {
  GraphicsStateGuardian *gsg = get_gsg();
  GraphicsEngine *engine = gsg->get_engine();
  GraphicsOutput *host = get_host();

  // The new buffer should be drawn before this buffer is drawn.  If
  // the user requires more control than this, he can set the sort
  // value himself.
  int sort = get_sort() - 1;

  GraphicsOutput *buffer = NULL;

  if ((x_size == 0) && (y_size == 0)) {
    // Currently, only parasite buffers support the tracking of the
    // host window size.  If the user requests this, we have to use a
    // parasite buffer.
    buffer = engine->make_parasite(host, name, sort, x_size, y_size);
    buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_copy_texture);
    return buffer;
  }

  if (show_buffers) {
    // If show_buffers is true, just go ahead and call make_buffer(),
    // since it all amounts to the same thing anyway--this will
    // actually create a new GraphicsWindow.
    buffer = engine->make_buffer(gsg, name, sort, x_size, y_size);
    buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_copy_texture);
    return buffer;
  }
  
  bool allow_bind = 
    (prefer_texture_buffer && support_render_texture && 
     gsg->get_supports_render_texture() && !to_ram);

  // If the user so indicated in the Config.prc file, try to create a
  // parasite buffer first.  We can only do this if the requested size
  // fits within the available framebuffer size.  Also, don't do this
  // if we want to try using render-to-a-texture mode, since using a
  // ParasiteButter will preclude that.
  if (prefer_parasite_buffer && !allow_bind &&
      (x_size <= host->get_x_size() && y_size <= host->get_y_size())) {
    buffer = engine->make_parasite(host, name, sort, x_size, y_size);
    if (buffer != (GraphicsOutput *)NULL) {
      buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_copy_texture);
      return buffer;
    }
  }

  // Attempt to create a single-buffered offscreen buffer.
  if (prefer_single_buffer) {
    FrameBufferProperties sb_props = gsg->get_properties();
    int orig_mode = sb_props.get_frame_buffer_mode();
    int sb_mode = (orig_mode & ~FrameBufferProperties::FM_buffer) | FrameBufferProperties::FM_single_buffer;
    sb_props.set_frame_buffer_mode(sb_mode);
    
    if (sb_mode != orig_mode) {
      PT(GraphicsStateGuardian) sb_gsg = 
        engine->make_gsg(gsg->get_pipe(), sb_props, gsg);
      if (sb_gsg != (GraphicsStateGuardian *)NULL) {
        buffer = engine->make_buffer(sb_gsg, name, sort, x_size, y_size);
        if (buffer != (GraphicsOutput *)NULL) {
          // Check the buffer for goodness.
          if (allow_bind) {
            buffer->add_render_texture(tex, RTM_bind_or_copy);
          } else {
            buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_copy_texture);
          }
          engine->open_windows();
          if (buffer->is_valid()) {
            return buffer;
          }

          // No good; delete the buffer and keep trying.
          bool removed = engine->remove_window(buffer);
          nassertr(removed, NULL);
          buffer = (GraphicsOutput *)NULL;
        }
      }
    }
  }

  // All right, attempt to create an offscreen buffer, using the same
  // GSG.  This will be a double-buffered offscreen buffer, if the
  // source window is double-buffered.
  buffer = engine->make_buffer(gsg, name, sort, x_size, y_size);
  if (buffer != (GraphicsOutput *)NULL) {
    if (allow_bind) {
      buffer->add_render_texture(tex, RTM_bind_or_copy);
    } else {
      buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_copy_texture);
    }
    engine->open_windows();
    if (buffer->is_valid()) {
      return buffer;
    }
    
    bool removed = engine->remove_window(buffer);
    nassertr(removed, NULL);
    buffer = (GraphicsOutput *)NULL;
  }

  // Looks like we have to settle for a parasite buffer.
  if (x_size <= host->get_x_size() && y_size <= host->get_y_size()) {
    buffer = engine->make_parasite(host, name, sort, x_size, y_size);
    buffer->add_render_texture(tex, to_ram ? RTM_copy_ram : RTM_copy_texture);
    return buffer;
  }

  return NULL;
}

struct ShowBuffersCubeMapRegions {
  float l, r, b, t;
};
static ShowBuffersCubeMapRegions cube_map_regions[6] = {
  { 0.0, 0.3333, 0.5, 1.0 },
  { 0.0, 0.3333, 0.0, 0.5 },
  { 0.3333, 0.6667, 0.5, 1.0 },
  { 0.3333, 0.6667, 0.0, 0.5 },
  { 0.6667, 1.0, 0.5, 1.0 },
  { 0.6667, 1.0, 0.0, 0.5 },
};

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
	      DrawMask camera_mask, bool to_ram) {
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
  if (show_buffers) {
    // If show_buffers is true, we'd like to create a window with the
    // six buffers spread out and all visible at once, for the user's
    // convenience.
    buffer = make_texture_buffer(name, size * 3, size * 2, tex, to_ram);
    tex->set_x_size(size);
    tex->set_y_size(size);

  } else {
    // In the normal case, the six buffers are stacked on top of each
    // other like pancakes.
    buffer = make_texture_buffer(name, size, size, tex, to_ram);
  }

  // We don't need to clear the overall buffer; instead, we'll clear
  // each display region.
  buffer->set_clear_color_active(false);
  buffer->set_clear_depth_active(false);

  PT(Lens) lens = new PerspectiveLens;
  lens->set_fov(90.0f);

  for (int i = 0; i < 6; i++) {
    PT(Camera) camera = new Camera(cube_faces[i]._name);
    camera->set_lens(lens);
    camera->set_camera_mask(camera_mask);
    NodePath camera_np = camera_rig.attach_new_node(camera);
    camera_np.look_at(cube_faces[i]._look_at, cube_faces[i]._up);
    
    DisplayRegion *dr;
    if (show_buffers) {
      const ShowBuffersCubeMapRegions &r = cube_map_regions[i];
      dr = buffer->make_display_region(r.l, r.r, r.b, r.t);
    } else {
      dr = buffer->make_display_region();
    }
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
//  Description: resets the window framebuffer from its derived
//               children. Does nothing here.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
reset_window(bool swapchain) {
  display_cat.info()
    << "Resetting " << get_type() << "\n";
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
begin_frame() {
  if (display_cat.is_spam()) {
    display_cat.spam()
      << "begin_frame(): " << get_type() << " " 
      << get_name() << " " << (void *)this << "\n";
  }

  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  // Track the size of some other graphics output, if desired.
  auto_resize();
  
  if (needs_context()) {
    if (!make_context()) {
      return false;
    }
  }

  // Okay, we already have a GSG, so activate it.
  make_current();
  
  begin_render_texture();
  
  _cube_map_index = -1;
  _cube_map_dr = NULL;

  return _gsg->begin_frame();
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
clear() {
  if (is_any_clear_active()) {
    if (display_cat.is_spam()) {
      display_cat.spam()
        << "clear(): " << get_type() << " " 
        << get_name() << " " << (void *)this << "\n";
    }

    nassertv(_gsg != (GraphicsStateGuardian *)NULL);

    DisplayRegionStack old_dr = _gsg->push_display_region(_default_display_region);
    _gsg->clear(this);
    _gsg->pop_display_region(old_dr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
end_frame() {
  if (display_cat.is_spam()) {
    display_cat.spam()
      << "end_frame(): " << get_type() << " " 
      << get_name() << " " << (void *)this << "\n";
  }

  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  _gsg->end_frame();

  // Handle all render-to-texture operations that use bind-to-texture
  end_render_texture();
  
  // Handle all render-to-texture operations that use copy-to-texture
  for (int i=0; i<count_textures(); i++) {
    RenderTextureMode rtm_mode = get_rtm_mode(i);
    if ((rtm_mode == RTM_none)||(rtm_mode == RTM_bind_or_copy)) {
      continue;
    }

    Texture *texture = get_texture(i);
    PStatTimer timer(_copy_texture_pcollector);
    nassertv(has_texture());
    
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
      RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
      if (_cube_map_dr != (DisplayRegion *)NULL) {
        if ((rtm_mode == RTM_copy_ram)||(rtm_mode == RTM_triggered_copy_ram)) {
          _gsg->framebuffer_copy_to_ram(texture, _cube_map_index,
                                        _cube_map_dr, buffer);
        } else {
          _gsg->framebuffer_copy_to_texture(texture, _cube_map_index,
                                            _cube_map_dr, buffer);
        }
      } else {
        if ((rtm_mode == RTM_copy_ram)||(rtm_mode == RTM_triggered_copy_ram)) {
          _gsg->framebuffer_copy_to_ram(texture, _cube_map_index,
                                        _default_display_region, buffer);
        } else {
          _gsg->framebuffer_copy_to_texture(texture, _cube_map_index,
                                            _default_display_region, buffer);
        }
      }
    }
  }
  _trigger_copy = false;
  
  // If we're not single-buffered, we're now ready to flip.
  if (!_gsg->get_properties().is_single_buffered()) {
    _flip_ready = true;
  }
  
  // In one-shot mode, we request the GraphicsEngine to delete the
  // window after we have rendered a frame.
  if (_one_shot) {
    // But when show-buffers mode is enabled, we want to keep the
    // window around until the user has a chance to see the texture.
    // So we don't do most of the following in show-buffers mode.
    if (!show_buffers) {
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
    }

    // We have to be sure to clear the _textures pointers, though, or
    // we'll end up holding a reference to the textures forever.
    clear_render_textures();
  }
  
  _cube_map_index = -1;
  _cube_map_dr = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::begin_render_texture
//       Access: Public, Virtual
//  Description: If the GraphicsOutput supports direct render-to-texture,
//               and if any setup needs to be done during begin_frame,
//               then the setup code should go here.  Any textures that
//               can not be rendered to directly should be reflagged
//               as RTM_copy_texture.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
begin_render_texture() {
  for (int i=0; i<count_textures(); i++) {
    if (get_rtm_mode(i) == RTM_bind_or_copy) {
      _textures[i]._rtm_mode = RTM_copy_texture;
    }
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
void GraphicsOutput::
end_render_texture() {
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
change_scenes(DisplayRegion *new_dr) {
  int new_cube_map_index = new_dr->get_cube_map_index();
  if (new_cube_map_index != -1 &&
      new_cube_map_index != _cube_map_index) {
    int old_cube_map_index = _cube_map_index;
    DisplayRegion *old_cube_map_dr = _cube_map_dr;
    _cube_map_index = new_cube_map_index;
    _cube_map_dr = new_dr;

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
          RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
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
//     Function: GraphicsOutput::make_context
//       Access: Public, Virtual
//  Description: If _needs_context is true, this will be called
//               in the draw thread prior to rendering into the
//               window.  It should attempt to create a graphics
//               context, and return true if successful, false
//               otherwise.  If it returns false the window will be
//               considered failed.
////////////////////////////////////////////////////////////////////
bool GraphicsOutput::
make_context() {
  _needs_context = false;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
make_current() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::release_gsg
//       Access: Public
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
release_gsg() {
  _gsg.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::auto_resize
//       Access: Public, Virtual
//  Description: Certain graphics outputs can automatically resize
//               themselves to automatically stay the same size as
//               some other graphics output.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
auto_resize() {
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
  MutexHolder holder(_lock);
  _total_display_regions.push_back(display_region);
  _display_regions_stale = true;

  return display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::do_determine_display_regions
//       Access: Private
//  Description: Re-sorts the list of active DisplayRegions within
//               the window.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
do_determine_display_regions() {
  MutexHolder holder(_lock);
  _display_regions_stale = false;

  _active_display_regions.clear();
  _active_display_regions.reserve(_total_display_regions.size());

  TotalDisplayRegions::const_iterator dri;
  for (dri = _total_display_regions.begin();
       dri != _total_display_regions.end();
       ++dri) {
    DisplayRegion *display_region = (*dri);
    if (display_region->is_active()) {
      _active_display_regions.push_back(display_region);
    }
  }

  stable_sort(_active_display_regions.begin(), _active_display_regions.end(), IndirectLess<DisplayRegion>());
}
