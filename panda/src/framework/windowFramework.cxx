// Filename: windowFramework.cxx
// Created by:  drose (02Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "windowFramework.h"
#include "pandaFramework.h"
#include "mouseAndKeyboard.h"
#include "buttonThrower.h"
#include "transform2sg.h"
#include "dSearchPath.h"
#include "filename.h"
#include "loader.h"
#include "keyboardButton.h"
#include "geomTri.h"
#include "texturePool.h"
#include "textureAttrib.h"
#include "perspectiveLens.h"
#include "auto_bind.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "lightAttrib.h"
#include "boundingSphere.h"
#include "deg_2_rad.h"
#include "config_framework.h"

// This number is chosen arbitrarily to override any settings in model
// files.
static const int override_priority = 100;

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
WindowFramework::
WindowFramework(PandaFramework *panda_framework) :
  _panda_framework(panda_framework)
{
  _alight = (AmbientLight *)NULL;
  _dlight = (DirectionalLight *)NULL;
  _got_keyboard = false;
  _got_trackball = false;
  _got_lights = false;
  _wireframe_enabled = false;
  _texture_enabled = true;
  _two_sided_enabled = false;
  _lighting_enabled = false;
  _background_type = BT_default;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::Destructor
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
WindowFramework::
~WindowFramework() {
  close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::open_window
//       Access: Protected
//  Description: Opens the actual window.  This is normally called
//               only from PandaFramework::open_window().
////////////////////////////////////////////////////////////////////
GraphicsWindow *WindowFramework::
open_window(const WindowProperties &props, GraphicsEngine *engine,
            GraphicsPipe *pipe) {
  nassertr(_window == (GraphicsWindow *)NULL, _window);

  _window = engine->make_window(pipe);
  if (_window != (GraphicsWindow *)NULL) {
    _window->request_properties(props);
    set_background_type(_background_type);
  }

  // Set up a 3-d camera for the window by default.
  make_camera();
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::close_window
//       Access: Protected
//  Description: Closes the window.  This is normally called
//               from PandaFramework::close_window().
////////////////////////////////////////////////////////////////////
void WindowFramework::
close_window() {
  _window.clear();
  _camera_group.remove_node();
  _render.remove_node();
  _render_2d.remove_node();
  _mouse.remove_node();

  _alight = (AmbientLight *)NULL;
  _dlight = (DirectionalLight *)NULL;
  _got_keyboard = false;
  _got_trackball = false;
  _got_lights = false;

  _wireframe_enabled = false;
  _texture_enabled = true;
  _two_sided_enabled = false;
  _lighting_enabled = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_camera_group
//       Access: Public
//  Description: Returns the node above the collection of 3-d cameras
//               in the scene graph.  This node may be moved around to
//               represent the viewpoint.
////////////////////////////////////////////////////////////////////
const NodePath &WindowFramework::
get_camera_group() {
  if (_camera_group.is_empty()) {
    _camera_group = get_render().attach_new_node("camera_group");
  }
  return _camera_group;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_render
//       Access: Public
//  Description: Returns the root of the 3-d scene graph.
////////////////////////////////////////////////////////////////////
const NodePath &WindowFramework::
get_render() {
  if (_render.is_empty()) {
    _render = NodePath("render");

    // This is maybe here temporarily, and maybe not.
    _render.set_two_sided(0);
  }
  return _render;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_render_2d
//       Access: Public
//  Description: Returns the root of the 2-d scene graph.
////////////////////////////////////////////////////////////////////
const NodePath &WindowFramework::
get_render_2d() {
  if (_render_2d.is_empty()) {
    _render_2d = NodePath("render_2d");
  }
  return _render_2d;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::get_mouse
//       Access: Public
//  Description: Returns the node in the data graph corresponding to
//               the mouse associated with this window.
////////////////////////////////////////////////////////////////////
const NodePath &WindowFramework::
get_mouse() {
  if (_mouse.is_empty()) {
    nassertr(_window->get_num_input_devices() > 0, _mouse);
    NodePath data_root = _panda_framework->get_data_root();
    MouseAndKeyboard *mouse_node = 
      new MouseAndKeyboard(_window, 0, "mouse");
    _mouse = data_root.attach_new_node(mouse_node);
  }
  return _mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::enable_keyboard
//       Access: Public
//  Description: Creates a ButtonThrower to listen to button presses
//               and throw them as events.
////////////////////////////////////////////////////////////////////
void WindowFramework::
enable_keyboard() {
  if (_got_keyboard) {
    return;
  }

  if (_window->get_num_input_devices() > 0) {
    NodePath mouse = get_mouse();

    PT(ButtonThrower) bt = new ButtonThrower("kb-events");
    ModifierButtons mods;
    mods.add_button(KeyboardButton::shift());
    mods.add_button(KeyboardButton::control());
    mods.add_button(KeyboardButton::alt());
    bt->set_modifier_buttons(mods);
    mouse.attach_new_node(bt);
  }

  _got_keyboard = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::setup_trackball
//       Access: Public
//  Description: Sets up the mouse to trackball around the camera.
////////////////////////////////////////////////////////////////////
void WindowFramework::
setup_trackball() {
  if (_got_trackball) {
    return;
  }

  if (_window->get_num_input_devices() > 0) {
    NodePath mouse = get_mouse();
    NodePath camera = get_camera_group();

    _trackball = new Trackball("trackball");
    _trackball->set_pos(LVector3f::forward() * 50.0);
    mouse.attach_new_node(_trackball);
    
    PT(Transform2SG) tball2cam = new Transform2SG("tball2cam");
    tball2cam->set_node(camera.node());
    _trackball->add_child(tball2cam);
  }

  _got_trackball = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::center_trackball
//       Access: Public
//  Description: Centers the trackball on the indicated object, and
//               scales the trackball motion suitably.
////////////////////////////////////////////////////////////////////
void WindowFramework::
center_trackball(const NodePath &object) {
  if (_trackball == (Trackball *)NULL) {
    return;
  }

  PT(BoundingVolume) volume = object.get_bounds();
  // We expect at least a geometric bounding volume around the world.
  nassertv(volume != (BoundingVolume *)NULL);
  nassertv(volume->is_of_type(GeometricBoundingVolume::get_class_type()));
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, volume);
  
  // Determine the bounding sphere around the world.  The topmost
  // BoundingVolume might itself be a sphere (it's likely), but since
  // it might not, we'll take no chances and make our own sphere.
  PT(BoundingSphere) sphere = new BoundingSphere;
  if (!sphere->extend_by(gbv)) {
    framework_cat.warning()
      << "Cannot determine bounding volume of " << object << "\n";
    return;
  }

  if (sphere->is_infinite()) {
    framework_cat.warning()
      << "Infinite bounding volume for " << object << "\n";
    return;
  }

  if (sphere->is_empty()) {
    framework_cat.warning()
      << "Empty bounding volume for " << object << "\n";
    return;
  }

  LPoint3f center = sphere->get_center();
  float radius = sphere->get_radius();

  float distance = 50.0f;

  // Choose a suitable distance to view the whole volume in our frame.
  // This is based on the camera lens in use.  Determine the lens
  // based on the first camera; this will be the default camera.
  Lens *lens = (Lens *)NULL;
  if (!_cameras.empty()) {
    Cameras::const_iterator ci;
    for (ci = _cameras.begin();
         ci != _cameras.end() && lens == (Lens *)NULL;
         ++ci) {
      lens = (*ci)->get_lens();
    }
  }

  if (lens != (Lens *)NULL) {
    LVecBase2f fov = lens->get_fov();
    distance = radius / ctan(deg_2_rad(min(fov[0], fov[1]) / 2.0f));

    // Ensure the far plane is far enough back to see the entire object.
    float ideal_far_plane = distance + radius;
    lens->set_far(max(lens->get_default_far(), ideal_far_plane)); 

    // And that the near plane is far enough forward.
    float ideal_near_plane = distance - radius;
    lens->set_near(min(lens->get_default_near(), ideal_near_plane)); 
  }

  _trackball->set_origin(center);
  _trackball->set_pos(LVector3f::forward() * distance);

  // Also set the movement scale on the trackball to be consistent
  // with the size of the model and the lens field-of-view.
  _trackball->set_forward_scale(distance * 0.006);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::load_models
//       Access: Public
//  Description: Loads up all the model files listed in the indicated
//               argument list.  If first_arg is supplied, it is the
//               first argument in the list to consider.
//
//               Returns true if all models loaded successfully, or
//               false if at least one of them had an error.
////////////////////////////////////////////////////////////////////
bool WindowFramework::
load_models(const NodePath &parent, int argc, char *argv[], int first_arg) {
  pvector<Filename> files;

  for (int i = first_arg; i < argc && argv[i] != (char *)NULL; i++) {
    files.push_back(Filename::from_os_specific(argv[i]));
  }

  return load_models(parent, files);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::load_models
//       Access: Public
//  Description: Loads up all the model files listed in the indicated
//               argument list.
//
//               Returns true if all models loaded successfully, or
//               false if at least one of them had an error.
////////////////////////////////////////////////////////////////////
bool WindowFramework::
load_models(const NodePath &parent, const pvector<Filename> &files) {
  bool all_ok = true;

  NodePath render = get_render();

  pvector<Filename>::const_iterator fi;
  for (fi = files.begin(); fi != files.end(); ++fi) {
    const Filename &filename = (*fi);
    NodePath model = load_model(parent, filename);
    if (model.is_empty()) {
      all_ok = false;
    }
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::load_model
//       Access: Public
//  Description: Loads up the indicated model and returns the new
//               NodePath, or the empty NodePath if the model could
//               not be loaded.
////////////////////////////////////////////////////////////////////
NodePath WindowFramework::
load_model(const NodePath &parent, Filename filename) {
  nout << "Loading " << filename << "\n";

  // If the filename already exists where it is, or if it is fully
  // qualified, don't search along the model path for it.
  bool search = !(filename.is_fully_qualified() || filename.exists());
  
  Loader loader;
  PT(PandaNode) node = loader.load_sync(filename, search);
  if (node == (PandaNode *)NULL) {
    nout << "Unable to load " << filename << "\n";
    return NodePath::not_found();
  }    

  return parent.attach_new_node(node);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::load_default_model
//       Access: Public
//  Description: Loads our favorite blue triangle.  This is intended
//               to provide some default geometry to have *something*
//               to look at for testing, when no other models are
//               provided.
////////////////////////////////////////////////////////////////////
NodePath WindowFramework::
load_default_model(const NodePath &parent) {
  PTA_Vertexf coords;
  PTA_TexCoordf uvs;
  PTA_Normalf norms;
  PTA_Colorf colors;
  PTA_ushort cindex;
  
  coords.push_back(Vertexf::rfu(0.0, 0.0, 0.0));
  coords.push_back(Vertexf::rfu(1.0, 0.0, 0.0));
  coords.push_back(Vertexf::rfu(0.0, 0.0, 1.0));
  uvs.push_back(TexCoordf(0.0, 0.0));
  uvs.push_back(TexCoordf(1.0, 0.0));
  uvs.push_back(TexCoordf(0.0, 1.0));
  norms.push_back(Normalf::back());
  colors.push_back(Colorf(0.5, 0.5, 1.0, 1.0));
  cindex.push_back(0);
  cindex.push_back(0);
  cindex.push_back(0);
  
  PT(GeomTri) geom = new GeomTri;
  geom->set_num_prims(1);
  geom->set_coords(coords);
  geom->set_texcoords(uvs, G_PER_VERTEX);
  geom->set_normals(norms, G_PER_PRIM);
  geom->set_colors(colors, G_PER_VERTEX, cindex);

  CPT(RenderState) state = RenderState::make_empty();
  Texture *tex = TexturePool::load_texture("rock-floor.rgb");
  if (tex != (Texture *)NULL) {
    tex->set_minfilter(Texture::FT_linear);
    tex->set_magfilter(Texture::FT_linear);
    state = state->add_attrib(TextureAttrib::make(tex));
  }
  
  GeomNode *geomnode = new GeomNode("tri");
  geomnode->add_geom(geom, state);

  return parent.attach_new_node(geomnode);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::loop_animations
//       Access: Public
//  Description: Looks for characters and their matching animation
//               files in the scene graph; binds and loops any
//               matching animations found.
////////////////////////////////////////////////////////////////////
void WindowFramework::
loop_animations() {
  // If we happened to load up both a character file and its matching
  // animation file, attempt to bind them together now and start the
  // animations looping.
  auto_bind(get_render().node(), _anim_controls, ~0);
  _anim_controls.loop_all(true);
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::set_wireframe
//       Access: Public
//  Description: Forces wireframe state (true) or restores default
//               rendering (false).
////////////////////////////////////////////////////////////////////
void WindowFramework::
set_wireframe(bool enable) {
  if (enable == _wireframe_enabled) {
    return;
  }

  NodePath render = get_render();

  if (enable) {
    render.set_render_mode_wireframe(override_priority);
    render.set_two_sided(true, override_priority);
  } else {
    render.clear_render_mode();
    if (!_two_sided_enabled) {
      render.clear_two_sided();
    }
  }

  _wireframe_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::set_texture
//       Access: Public
//  Description: Forces textures off (false) or restores default
//               rendering (true).
////////////////////////////////////////////////////////////////////
void WindowFramework::
set_texture(bool enable) {
  if (enable == _texture_enabled) {
    return;
  }

  NodePath render = get_render();

  if (!enable) {
    render.set_texture_off(override_priority);
  } else {
    render.clear_texture();
  }

  _texture_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::set_two_sided
//       Access: Public
//  Description: Forces two-sided rendering (true) or restores default
//               rendering (false).
////////////////////////////////////////////////////////////////////
void WindowFramework::
set_two_sided(bool enable) {
  if (enable == _two_sided_enabled) {
    return;
  }

  NodePath render = get_render();

  if (enable) {
    render.set_two_sided(true, override_priority);
  } else {
    if (!_wireframe_enabled) {
      render.clear_two_sided();
    }
  }

  _two_sided_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::set_lighting
//       Access: Public
//  Description: Turns lighting on (true) or off (false).
////////////////////////////////////////////////////////////////////
void WindowFramework::
set_lighting(bool enable) {
  if (enable == _lighting_enabled) {
    return;
  }

  NodePath render = get_render();

  if (enable) {
    if (!_got_lights) {
      setup_lights();
    }
    render.node()->set_attrib(LightAttrib::make(LightAttrib::O_add, 
                                                _alight, _dlight));
  } else {
    render.node()->clear_attrib(LightAttrib::get_class_type());
  }

  _lighting_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::set_background_type
//       Access: Public
//  Description: Sets the background of the window to one of the
//               pre-canned background types (or to BT_other, which
//               indicates the user intends to set up his own special
//               background mode).
////////////////////////////////////////////////////////////////////
void WindowFramework::
set_background_type(WindowFramework::BackgroundType type) {
  _background_type = type;

  if (_window != (GraphicsWindow *)NULL) {
    switch (_background_type) {
    case BT_other:
      break;
      
    case BT_default:
      _window->set_clear_color_active(true);
      _window->set_clear_depth_active(true);
      _window->set_clear_color(Colorf(win_background_r, win_background_g, 
                                      win_background_b, 1.0f));
      _window->set_clear_depth(1.0f);
      break;
      
    case BT_gray:
      _window->set_clear_color_active(true);
      _window->set_clear_depth_active(true);
      _window->set_clear_color(Colorf(0.3f, 0.3f, 0.3f, 1.0f));
      _window->set_clear_depth(1.0f);
      break;
      
    case BT_black:
      _window->set_clear_color_active(true);
      _window->set_clear_depth_active(true);
      _window->set_clear_color(Colorf(0.0f, 0.0f, 0.0f, 1.0f));
      _window->set_clear_depth(1.0f);
      break;

    case BT_none:
      _window->set_clear_color_active(false);
      _window->set_clear_depth_active(false);
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::make_camera
//       Access: Protected
//  Description: Makes a new 3-d camera for the window.
////////////////////////////////////////////////////////////////////
PT(Camera) WindowFramework::
make_camera() {
  // Get the first channel on the window.  This will be the only
  // channel on non-SGI hardware.
  PT(GraphicsChannel) channel = _window->get_channel(0);

  // Make a layer on the channel to hold our display region.
  PT(GraphicsLayer) layer = channel->make_layer();

  // And create a display region that covers the entire window.
  PT(DisplayRegion) dr = layer->make_display_region();

  // Finally, we need a camera to associate with the display region.
  PT(Camera) camera = new Camera("camera");
  NodePath camera_np = get_camera_group().attach_new_node(camera);
  _cameras.push_back(camera);

  PT(Lens) lens = new PerspectiveLens;
  WindowProperties properties = _window->get_properties();
  if (!properties.has_size()) {
    properties = _window->get_requested_properties();
  }
  if (properties.has_size()) {
    lens->set_film_size(properties.get_x_size(), properties.get_y_size());
  }
  camera->set_lens(lens);
  camera->set_scene(get_render());
  dr->set_camera(camera_np);

  return camera;
}

////////////////////////////////////////////////////////////////////
//     Function: WindowFramework::setup_lights
//       Access: Protected
//  Description: Makes light nodes and attaches them to the camera for
//               viewing the scene.
////////////////////////////////////////////////////////////////////
void WindowFramework::
setup_lights() {
  if (_got_lights) {
    return;
  }

  NodePath camera_group = get_camera_group();
  NodePath light_group = camera_group.attach_new_node("lights");

  _alight = new AmbientLight("ambient");
  _alight->set_color(Colorf(0.2f, 0.2f, 0.2f, 1.0f));
  _dlight = new DirectionalLight("directional");
  light_group.attach_new_node(_alight);
  light_group.attach_new_node(_dlight);
  
  _got_lights = true;
}
