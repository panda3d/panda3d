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
  _rtm_mode = RTM_none;
  _flip_ready = false;
  _needs_context = true;
  _cube_map_index = -1;
  _sort = 0;
  _internal_sort_index = 0;
  _active = true;
  _one_shot = false;
  _inverted = window_inverted;
  _delete_flag = false;

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
//     Function: GraphicsOutput::detach_texture
//       Access: Published
//  Description: Disassociates the texture from the GraphicsOutput.
//               The texture will no longer be filled as the frame
//               renders, and it may be used as an ordinary texture in
//               its own right.  However, its contents may be
//               undefined.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
detach_texture() {
  MutexHolder holder(_lock);

  if (_rtm_mode == RTM_bind_texture && _gsg != (GraphicsStateGuardian *)NULL) {
    _gsg->framebuffer_release_texture(this, get_texture());
  }

  _texture = NULL;
  _rtm_mode = RTM_none;

  set_inverted(window_inverted);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::setup_render_texture
//       Access: Published
//  Description: Creates a new Texture object, suitable for rendering
//               the contents of this buffer into, and stores it in
//               _texture.  This also disassociates the previous
//               texture (if any).
//
//               If tex is not NULL, it is the texture that will be
//               set up for rendering into; otherwise, a new Texture
//               object will be created (in which case you may call
//               get_texture() to retrieve the new texture pointer
//               later).
//
//               If allow_bind is true, and this GraphicsOutput is an
//               offscreen graphics buffer that has not yet been
//               rendered into, it will attempt to set up the buffer
//               for rendering directly into the texture, avoiding the
//               cost of the copy-to-texture-memory each frame.  This
//               is not supported by all graphics hardware, but if it
//               is not supported, this option is quietly ignored.
//
//               If to_ram is true, the texture image will be
//               downloaded from the framebuffer memory into system
//               RAM each frame, which is more expensive but allows
//               the texture to subsequently be applied to any GSG.
//               Otherwise, the texture image remains in texture
//               memory only.
//
//               Also see make_texture_buffer(), which is a
//               higher-level interface for preparing
//               render-to-a-texture mode.
////////////////////////////////////////////////////////////////////
void GraphicsOutput::
setup_render_texture(Texture *tex, bool allow_bind, bool to_ram) {
  MutexHolder holder(_lock);

  if (_rtm_mode == RTM_bind_texture && _gsg != (GraphicsStateGuardian *)NULL) {
    _gsg->framebuffer_release_texture(this, get_texture());
  }

  if (tex == (Texture *)NULL) {
    _texture = new Texture(get_name());
    _texture->set_wrap_u(Texture::WM_clamp);
    _texture->set_wrap_v(Texture::WM_clamp);

  } else {
    _texture = tex;
    _texture->clear_ram_image();
  }
  _texture->set_match_framebuffer_format(true);

  // Go ahead and tell the texture our anticipated size, even if it
  // might be inaccurate (particularly if this is a GraphicsWindow,
  // which has system-imposed restrictions on size).
  _texture->set_x_size(get_x_size());
  _texture->set_y_size(get_y_size());

  if (to_ram) {
    _rtm_mode = RTM_copy_ram;
  } else if (allow_bind) {
    _rtm_mode = RTM_bind_if_possible;
  } else {
    _rtm_mode = RTM_copy_texture;
  }

  nassertv(_gsg != (GraphicsStateGuardian *)NULL);
  set_inverted(_gsg->get_copy_texture_inverted());
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

  if (show_buffers) {
    // If show_buffers is true, just go ahead and call make_buffer(),
    // since it all amounts to the same thing anyway--this will
    // actually create a new GraphicsWindow.
    buffer = engine->make_buffer(gsg, name, sort, x_size, y_size);
    buffer->setup_render_texture(tex, false, to_ram);
    return buffer;
  }
  
  bool allow_bind = 
    (prefer_texture_buffer && gsg->get_supports_render_texture() && !to_ram);

  // If the user so indicated in the Config.prc file, try to create a
  // parasite buffer first.  We can only do this if the requested size
  // fits within the available framebuffer size.  Also, don't do this
  // if we want to try using render-to-a-texture mode, since using a
  // ParasiteButter will preclude that.
  if (prefer_parasite_buffer && !allow_bind &&
      (x_size <= host->get_x_size() && y_size <= host->get_y_size())) {
    buffer = engine->make_parasite(host, name, sort, x_size, y_size);
    if (buffer != (GraphicsOutput *)NULL) {
      buffer->setup_render_texture(tex, false, to_ram);
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
          buffer->setup_render_texture(tex, allow_bind, to_ram);
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
    buffer->setup_render_texture(tex, allow_bind, to_ram);
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
    buffer->setup_render_texture(tex, false, to_ram);
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
make_cube_map(const string &name, int size, bool to_ram,
              NodePath &camera_rig, DrawMask camera_mask) {
  if (!to_ram) {
    // Check the limits imposed by the GSG.  (However, if we're
    // rendering the texture to RAM only, these limits may be
    // irrelevant.)
    GraphicsStateGuardian *gsg = get_gsg();
    int max_dimension = gsg->get_max_cube_map_dimension();
    if (max_dimension == 0) {
      // The GSG doesn't support cube mapping; too bad for you.
      return NULL;
    }
    if (max_dimension > 0) {
      size = min(max_dimension, size);
    }
  }

  PT(Texture) tex = new Texture(name);
  tex->setup_cube_map();
  tex->set_wrap_u(Texture::WM_clamp);
  tex->set_wrap_v(Texture::WM_clamp);
  GraphicsOutput *buffer = make_texture_buffer(name, size, size, tex, to_ram);

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
    
    DisplayRegion *dr = buffer->make_display_region();
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

  if (needs_context()) {
    if (!make_context()) {
      return false;
    }
  }

  // Okay, we already have a GSG, so activate it.
  make_current();

  if (_rtm_mode == RTM_bind_texture) {
    // Release the texture so we can render into the frame buffer.
    _gsg->framebuffer_release_texture(this, get_texture());
  }

  _cube_map_index = -1;

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

  // If _rtm_mode isn't RTM_none, it means we should copy or lock the
  // framebuffer to the GraphicsOutput's associated texture after the
  // frame has rendered.
  if (_rtm_mode != RTM_none) {
    PStatTimer timer(_copy_texture_pcollector);
    nassertv(has_texture());

    // If _rtm_mode is RTM_bind_texture, it means we should attempt to
    // lock the framebuffer directly to the texture memory, avoiding
    // the copy.
    if (_rtm_mode == RTM_bind_texture) {
      if (display_cat.is_debug()) {
        display_cat.debug()
          << "Locking texture for " << get_name() << " at frame end.\n";
      }
      if (!_gsg->framebuffer_bind_to_texture(this, get_texture())) {
        display_cat.warning()
          << "Lock-to-texture failed, resorting to copy.\n";
        _rtm_mode = RTM_copy_texture;
      }
    }

    if (_rtm_mode != RTM_bind_texture) {
      if (display_cat.is_debug()) {
        display_cat.debug()
          << "Copying texture for " << get_name() << " at frame end.\n";
        display_cat.debug()
          << "cube_map_index = " << _cube_map_index << "\n";
      }
      RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
      if (_rtm_mode == RTM_copy_ram) {
        _gsg->framebuffer_copy_to_ram(get_texture(), _cube_map_index,
                                      _default_display_region, buffer);
      } else {
        _gsg->framebuffer_copy_to_texture(get_texture(), _cube_map_index,
                                          _default_display_region, buffer);
      }
    }
  }

  // If we're not single-buffered, we're now ready to flip.
  if (!_gsg->get_properties().is_single_buffered()) {
    _flip_ready = true;
  }

  if (_one_shot && !show_buffers) {
    // In one-shot mode, we request the GraphicsEngine to delete the
    // window after we have rendered a frame.  But when show-buffers
    // mode is enabled, we don't do this, to give the user a chance to
    // see the output.
    _active = false;
    _delete_flag = true;
  }

  _cube_map_index = -1;
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
    _cube_map_index = new_cube_map_index;

    if (_rtm_mode != RTM_none) {
      if (_rtm_mode == RTM_bind_texture) {
        // In render-to-texture mode, switch the rendering backend to
        // the new cube map face, so that the subsequent frame will be
        // rendered to the new face.

        select_cube_map(new_cube_map_index);
        
      } else if (old_cube_map_index != -1) {
        // In copy-to-texture mode, copy the just-rendered framebuffer
        // to the old cube map face.
        if (display_cat.is_debug()) {
          display_cat.debug()
            << "Copying texture for " << get_name() << " at scene change.\n";
          display_cat.debug()
            << "cube_map_index = " << old_cube_map_index << "\n";
        }
        RenderBuffer buffer = _gsg->get_render_buffer(get_draw_buffer_type());
        if (_rtm_mode == RTM_copy_ram) {
          _gsg->framebuffer_copy_to_ram(get_texture(), old_cube_map_index, 
                                        _default_display_region, buffer);
        } else {
          _gsg->framebuffer_copy_to_texture(get_texture(), old_cube_map_index, 
                                            _default_display_region, buffer);
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
