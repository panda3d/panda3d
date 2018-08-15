/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowFramework.cxx
 * @author drose
 * @date 2002-04-02
 */

#include "windowFramework.h"
#include "pandaFramework.h"
#include "displayRegion.h"
#include "buttonThrower.h"
#include "transform2sg.h"
#include "dSearchPath.h"
#include "filename.h"
#include "loader.h"
#include "keyboardButton.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomTristrips.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "texturePool.h"
#include "textureAttrib.h"
#include "colorAttrib.h"
#include "perspectiveLens.h"
#include "orthographicLens.h"
#include "auto_bind.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "lightAttrib.h"
#include "boundingSphere.h"
#include "deg_2_rad.h"
#include "config_framework.h"
#include "cullFaceAttrib.h"
#include "rescaleNormalAttrib.h"
#include "shadeModelAttrib.h"
#include "pgTop.h"
#include "geomNode.h"
#include "texture.h"
#include "texturePool.h"
#include "loaderFileTypeRegistry.h"
#include "pnmImage.h"
#include "virtualFileSystem.h"
#include "string_utils.h"
#include "bamFile.h"
#include "staticTextFont.h"
#include "mouseButton.h"

// This is generated data for the standard texture we apply to the blue
// triangle.
#include "rock_floor.rgb_src.c"

// This is generated data for shuttle_controls.bam, a bamified version of
// shuttle_controls.egg (found in the models tree).  It's compiled in
// shuttle_controls.bam_src.c.
#include "shuttle_controls.bam_src.c"

using std::istringstream;
using std::ostringstream;
using std::string;

// This number is chosen arbitrarily to override any settings in model files.
static const int override_priority = 100;

PT(TextFont) WindowFramework::_shuttle_controls_font = nullptr;
TypeHandle WindowFramework::_type_handle;

/**
 *
 */
WindowFramework::
WindowFramework(PandaFramework *panda_framework) :
  _panda_framework(panda_framework)
{
  _got_keyboard = false;
  _got_trackball = false;
  _got_lights = false;
  _anim_controls_enabled = false;
  _anim_index = 0;
  _wireframe_enabled = false;
  _wireframe_filled = false;
  _texture_enabled = true;
  _two_sided_enabled = false;
  _one_sided_reverse_enabled = false;
  _lighting_enabled = false;
  _perpixel_enabled = false;
  _background_type = BT_default;
}

/**
 *
 */
WindowFramework::
WindowFramework(const WindowFramework &copy, DisplayRegion *display_region) :
  _panda_framework(copy._panda_framework),
  _window(copy._window),
  _display_region_3d(display_region)
{
  _got_keyboard = false;
  _got_trackball = false;
  _got_lights = false;
  _anim_controls_enabled = false;
  _anim_index = 0;
  _wireframe_enabled = false;
  _texture_enabled = true;
  _two_sided_enabled = false;
  _one_sided_reverse_enabled = false;
  _lighting_enabled = false;
  _perpixel_enabled = false;
  _background_type = BT_default;

  set_background_type(copy._background_type);
  // Set up a 3-d camera for the window by default.
  NodePath camera_np = make_camera();
  _display_region_3d->set_camera(camera_np);
}

/**
 *
 */
WindowFramework::
~WindowFramework() {
  close_window();
}

/**
 * Opens the actual window or buffer.  This is normally called only from
 * PandaFramework::open_window().
 */
GraphicsOutput *WindowFramework::
open_window(const WindowProperties &props, int flags, GraphicsEngine *engine,
            GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
            const FrameBufferProperties &fbprops) {
  nassertr(_window == nullptr, _window);

  static int next_window_index = 1;
  ostringstream stream;
  stream << "window" << next_window_index;
  next_window_index++;
  string name = stream.str();

  _window = nullptr;
  GraphicsOutput *winout =
    engine->make_output(pipe, name, 0, fbprops,
                        props, flags, gsg, nullptr);
  if (winout != nullptr) {
    _window = winout;
    // _window->request_properties(props);

    // Create a display region that covers the entire window.
    _display_region_3d = _window->make_display_region();

    // Make sure the DisplayRegion does the clearing, not the window, so we
    // can have multiple DisplayRegions of different colors.
    _window->set_clear_color_active(false);
    _window->set_clear_depth_active(false);
    _window->set_clear_stencil_active(false);

    // Set up a 3-d camera for the window by default.
    NodePath camera_np = make_camera();
    _display_region_3d->set_camera(camera_np);

    set_background_type(_background_type);

    if (show_frame_rate_meter) {
      _frame_rate_meter = new FrameRateMeter("frame_rate_meter");
      _frame_rate_meter->setup_window(_window);
    }
    if (show_scene_graph_analyzer_meter) {
      _scene_graph_analyzer_meter = new SceneGraphAnalyzerMeter("scene_graph_analyzer_meter", get_render().node());
      _scene_graph_analyzer_meter->setup_window(_window);
    }
  }

  return _window;
}

/**
 * Closes the window or buffer.  This is normally called from
 * PandaFramework::close_window().
 */
void WindowFramework::
close_window() {
  _window.clear();
  _camera_group.remove_node();
  _render.remove_node();
  _render_2d.remove_node();
  _mouse.remove_node();

  _alight.clear();
  _dlight.clear();
  _got_keyboard = false;
  _got_trackball = false;
  _got_lights = false;

  _wireframe_enabled = false;
  _texture_enabled = true;
  _two_sided_enabled = false;
  _one_sided_reverse_enabled = false;
  _lighting_enabled = false;
  _perpixel_enabled = false;

  if (_frame_rate_meter != nullptr) {
    _frame_rate_meter->clear_window();
    _frame_rate_meter = nullptr;
  }
  if (_scene_graph_analyzer_meter != nullptr) {
    _scene_graph_analyzer_meter->clear_window();
    _scene_graph_analyzer_meter = nullptr;
  }
}

/**
 * Returns the node above the collection of 3-d cameras in the scene graph.
 * This node may be moved around to represent the viewpoint.
 */
NodePath WindowFramework::
get_camera_group() {
  if (_camera_group.is_empty()) {
    _camera_group = get_render().attach_new_node("camera_group");
  }
  return _camera_group;
}

/**
 * Returns the root of the 3-d scene graph.
 */
NodePath WindowFramework::
get_render() {
  if (_render.is_empty()) {
    _render = NodePath("render");

    _render.node()->set_attrib(RescaleNormalAttrib::make_default());
    _render.node()->set_attrib(ShadeModelAttrib::make(ShadeModelAttrib::M_smooth));

    // This is maybe here temporarily, and maybe not.
    _render.set_two_sided(0);
  }
  return _render;
}

/**
 * Returns the root of the 2-d scene graph.
 */
NodePath WindowFramework::
get_render_2d() {
  if (_render_2d.is_empty()) {
    _render_2d = NodePath("render_2d");

    // Some standard properties for the 2-d display.

    _render_2d.set_depth_write(0);
    _render_2d.set_depth_test(0);
    _render_2d.set_material_off(1);
    _render_2d.set_two_sided(1);

    // Now set up a 2-d camera to view render_2d.

    // Create a display region that matches the size of the 3-d display
    // region.
    PN_stdfloat l, r, b, t;
    _display_region_3d->get_dimensions(l, r, b, t);
    _display_region_2d = _window->make_mono_display_region(l, r, b, t);
    _display_region_2d->set_sort(10);

    // Finally, we need a camera to associate with the display region.
    PT(Camera) camera = new Camera("camera2d");
    NodePath camera_np = _render_2d.attach_new_node(camera);

    PT(Lens) lens = new OrthographicLens;

    static const PN_stdfloat left = -1.0f;
    static const PN_stdfloat right = 1.0f;
    static const PN_stdfloat bottom = -1.0f;
    static const PN_stdfloat top = 1.0f;
    lens->set_film_size(right - left, top - bottom);
    lens->set_film_offset((right + left) * 0.5, (top + bottom) * 0.5);
    lens->set_near_far(-1000, 1000);

    camera->set_lens(lens);
    _display_region_2d->set_camera(camera_np);
  }

  return _render_2d;
}

/**
 * Returns the node under the 2-d scene graph that is scaled to suit the
 * window's aspect ratio.
 */
NodePath WindowFramework::
get_aspect_2d() {
  if (_aspect_2d.is_empty()) {
    PGTop *top = new PGTop("aspect_2d");
    _aspect_2d = get_render_2d().attach_new_node(top);

    // Tell the PGTop about our MouseWatcher object, so the PGui system can
    // operate.
    PandaNode *mouse_node = get_mouse().node();
    if (mouse_node->is_of_type(MouseWatcher::get_class_type())) {
      top->set_mouse_watcher(DCAST(MouseWatcher, mouse_node));
    }

    PN_stdfloat this_aspect_ratio = aspect_ratio;
    if (this_aspect_ratio == 0.0f) {
      // An aspect ratio of 0.0 means to try to infer it.
      this_aspect_ratio = 1.0f;

      if (_window->has_size()) {
        int x_size = _window->get_sbs_left_x_size();
        int y_size = _window->get_sbs_left_y_size();
        if (y_size != 0) {
          this_aspect_ratio = (PN_stdfloat)x_size / (PN_stdfloat)y_size;
        }
      }
    }

    _aspect_2d.set_scale(1.0f / this_aspect_ratio, 1.0f, 1.0f);
  }

  return _aspect_2d;
}

/**
 * Returns a special root that uses units in pixels that are relative to the
 * window.  The upperleft corner of the window is (0, 0), the lowerleft corner
 * is (xsize, -ysize), in this coordinate system.
 */
NodePath WindowFramework::
get_pixel_2d() {
  if (_pixel_2d.is_empty()) {
    PGTop *top = new PGTop("pixel_2d");
    _pixel_2d = get_render_2d().attach_new_node(top);
    _pixel_2d.set_pos(-1, 0, 1);

    if (_window->has_size()) {
      int x_size = _window->get_sbs_left_x_size();
      int y_size = _window->get_sbs_left_y_size();
      if (x_size > 0){
        _pixel_2d.set_sx(2.0f / (float)x_size);
      }
      _pixel_2d.set_sy(1.0f);
      if (y_size > 0){
        _pixel_2d.set_sz(2.0f / (float)y_size);
      }
    }
  }

  return _pixel_2d;
}

/**
 * Returns the node in the data graph corresponding to the mouse associated
 * with this window.
 */
NodePath WindowFramework::
get_mouse() {
  if (_mouse.is_empty()) {
    NodePath mouse = _panda_framework->get_mouse(_window);

    // Create a MouseWatcher to filter the mouse input.  We do this mainly so
    // we can constrain the mouse input to our particular display region, if
    // we have one.  This means the node we return from get_mouse() is
    // actually a MouseWatcher, but since it presents the same interface as a
    // Mouse, no one should mind.

    // Another advantage to using a MouseWatcher is that the PGTop of aspect2d
    // likes it better.
    PT(MouseWatcher) mw = new MouseWatcher("watcher");

    if (_window->get_side_by_side_stereo()) {
      // If the window has side-by-side stereo enabled, then we should
      // constrain the MouseWatcher to the window's DisplayRegion.  This will
      // enable the MouseWatcher to track the left and right halves of the
      // screen individually.
      mw->set_display_region(_window->get_overlay_display_region());
    }

    _mouse = mouse.attach_new_node(mw);
  }
  return _mouse;
}

/**
 * Returns the node in the data graph corresponding to the ButtonThrower
 * object associated with this window.
 */
NodePath WindowFramework::
get_button_thrower() {
  return _button_thrower;
}

/**
 * Creates a ButtonThrower to listen to button presses and throw them as
 * events.
 */
void WindowFramework::
enable_keyboard() {
  if (_got_keyboard) {
    return;
  }

  if (_window->is_of_type(GraphicsWindow::get_class_type()) &&
      DCAST(GraphicsWindow, _window)->get_num_input_devices() > 0) {
    NodePath mouse = get_mouse();

    // Create a button thrower to listen for our keyboard events and associate
    // this WindowFramework pointer with each one.
    PT(ButtonThrower) bt = new ButtonThrower("kb-events");
    bt->add_parameter(EventParameter(this));
    ModifierButtons mods;
    mods.add_button(KeyboardButton::shift());
    mods.add_button(KeyboardButton::control());
    mods.add_button(KeyboardButton::alt());
    mods.add_button(KeyboardButton::meta());
    bt->set_modifier_buttons(mods);
    _button_thrower = mouse.attach_new_node(bt);
  }

  _got_keyboard = true;
}

/**
 * Sets up the mouse to trackball around the camera.
 */
void WindowFramework::
setup_trackball() {
  if (_got_trackball) {
    return;
  }

  if (_window->is_of_type(GraphicsWindow::get_class_type()) &&
      DCAST(GraphicsWindow, _window)->get_num_input_devices() > 0) {
    NodePath mouse = get_mouse();
    NodePath camera = get_camera_group();

    _trackball = new Trackball("trackball");
    _trackball->set_pos(LVector3::forward() * 50.0);
    mouse.attach_new_node(_trackball);

    PT(Transform2SG) tball2cam = new Transform2SG("tball2cam");
    tball2cam->set_node(camera.node());
    _trackball->add_child(tball2cam);
  }

  _got_trackball = true;
}

/**
 * Centers the trackball on the indicated object, and scales the trackball
 * motion suitably.
 */
void WindowFramework::
center_trackball(const NodePath &object) {
  if (_trackball == nullptr) {
    return;
  }

  PT(BoundingVolume) volume = object.get_bounds();
  // We expect at least a geometric bounding volume around the world.
  nassertv(volume != nullptr);
  nassertv(volume->is_of_type(GeometricBoundingVolume::get_class_type()));
  CPT(GeometricBoundingVolume) gbv = DCAST(GeometricBoundingVolume, volume);

  if (object.has_parent()) {
    CPT(TransformState) net_transform = object.get_parent().get_net_transform();
    PT(GeometricBoundingVolume) new_gbv = DCAST(GeometricBoundingVolume, gbv->make_copy());
    new_gbv->xform(net_transform->get_mat());
    gbv = new_gbv;
  }

  // Determine the bounding sphere around the object.
  if (gbv->is_infinite()) {
    framework_cat.warning()
      << "Infinite bounding volume for " << object << "\n";
    return;
  }

  if (gbv->is_empty()) {
    framework_cat.warning()
      << "Empty bounding volume for " << object << "\n";
    return;
  }

  // The BoundingVolume might be a sphere (it's likely), but since it might
  // not, we'll take no chances and make our own sphere.
  PT(BoundingSphere) sphere = new BoundingSphere(gbv->get_approx_center(), 0.0f);
  if (!sphere->extend_by(gbv)) {
    framework_cat.warning()
      << "Cannot determine bounding volume of " << object << "\n";
    return;
  }

  LPoint3 center = sphere->get_center();
  PN_stdfloat radius = sphere->get_radius();

  PN_stdfloat distance = 50.0f;

  // Choose a suitable distance to view the whole volume in our frame.  This
  // is based on the camera lens in use.  Determine the lens based on the
  // first camera; this will be the default camera.
  Lens *lens = nullptr;
  if (!_cameras.empty()) {
    Cameras::const_iterator ci;
    for (ci = _cameras.begin();
         ci != _cameras.end() && lens == nullptr;
         ++ci) {
      lens = (*ci)->get_lens();
    }
  }

  if (lens != nullptr) {
    LVecBase2 fov = lens->get_fov();
    distance = radius / ctan(deg_2_rad(std::min(fov[0], fov[1]) / 2.0f));

    // Ensure the far plane is far enough back to see the entire object.
    PN_stdfloat ideal_far_plane = distance + radius * 1.5;
    lens->set_far(std::max(lens->get_default_far(), ideal_far_plane));

    // And that the near plane is far enough forward.
    PN_stdfloat ideal_near_plane = distance - radius;
    lens->set_near(std::min(lens->get_default_near(), ideal_near_plane));
  }

  _trackball->set_origin(center);
  _trackball->set_pos(LVector3::forward() * distance);

  // Also set the movement scale on the trackball to be consistent with the
  // size of the model and the lens field-of-view.
  _trackball->set_forward_scale(distance * 0.006);
}

/**
 * Loads up all the model files listed in the indicated argument list.  If
 * first_arg is supplied, it is the first argument in the list to consider.
 *
 * Returns true if all models loaded successfully, or false if at least one of
 * them had an error.
 */
bool WindowFramework::
load_models(const NodePath &parent, int argc, char *argv[], int first_arg) {
  pvector<Filename> files;

  for (int i = first_arg; i < argc && argv[i] != nullptr; i++) {
    files.push_back(Filename::from_os_specific(argv[i]));
  }

  return load_models(parent, files);
}

/**
 * Loads up all the model files listed in the indicated argument list.
 *
 * Returns true if all models loaded successfully, or false if at least one of
 * them had an error.
 */
bool WindowFramework::
load_models(const NodePath &parent, const pvector<Filename> &files) {
  bool all_ok = true;

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

/**
 * Loads up the indicated model and returns the new NodePath, or the empty
 * NodePath if the model could not be loaded.
 */
NodePath WindowFramework::
load_model(const NodePath &parent, Filename filename) {
  framework_cat.info() << "Loading " << filename << "\n";

  // If the filename already exists where it is, or if it is fully qualified,
  // don't search along the model path for it.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  bool search = !(filename.is_fully_qualified() || vfs->exists(filename));

  // We allow loading image files here.  Check to see if it might be an image
  // file, based on the filename extension.
  bool is_image = false;
  string extension = filename.get_extension();
#ifdef HAVE_ZLIB
  if (extension == "pz" || extension == "gz") {
    extension = Filename(filename.get_basename_wo_extension()).get_extension();
  }
#endif  // HAVE_ZLIB
  TexturePool *texture_pool = TexturePool::get_global_ptr();
  LoaderFileType *model_type = nullptr;

  if (!extension.empty()) {
    LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();
    model_type = reg->get_type_from_extension(extension);

    if (model_type == nullptr) {
      // The extension isn't a known model file type; is it a known image file
      // extension?
      TexturePool *texture_pool = TexturePool::get_global_ptr();
      if (texture_pool->get_texture_type(extension) != nullptr) {
        // It is a known image file extension.
        is_image = true;
      }
    }
  }

  LoaderOptions options = PandaFramework::_loader_options;
  if (search) {
    options.set_flags(options.get_flags() | LoaderOptions::LF_search);
  } else {
    options.set_flags(options.get_flags() & ~LoaderOptions::LF_search);
  }

  Loader loader;
  PT(PandaNode) node;
  if (is_image) {
    node = load_image_as_model(filename);
  } else {
    node = loader.load_sync(filename, options);

    // It failed to load.  Is it because the extension isn't recognised?  If
    // so, then we just got done printing out the known scene types, and we
    // should also print out the supported texture types.
    if (node == nullptr && !is_image && model_type == nullptr) {
      texture_pool->write_texture_types(nout, 2);
    }
  }

  if (node == nullptr) {
    nout << "Unable to load " << filename << "\n";
    return NodePath::not_found();
  }

  return parent.attach_new_node(node);
}

/**
 * Loads our favorite blue triangle.  This is intended to provide some default
 * geometry to have *something* to look at for testing, when no other models
 * are provided.
 */
NodePath WindowFramework::
load_default_model(const NodePath &parent) {
  CPT(RenderState) state = RenderState::make_empty();

  state = state->add_attrib(ColorAttrib::make_flat(LColor(0.5, 0.5, 1.0, 1.0)));

  // Get the default texture to apply to the triangle; it's compiled into the
  // code these days.
  string rock_floor_string((const char *)rock_floor, rock_floor_len);
  istringstream rock_floor_strm(rock_floor_string);
  PNMImage rock_floor_pnm;
  if (rock_floor_pnm.read(rock_floor_strm, "rock-floor.rgb")) {
    PT(Texture) tex = new Texture;
    tex->set_name("rock-floor.rgb");
    tex->load(rock_floor_pnm);
    tex->set_minfilter(SamplerState::FT_linear);
    tex->set_magfilter(SamplerState::FT_linear);
    state = state->add_attrib(TextureAttrib::make(tex));
  }

  GeomNode *geomnode = new GeomNode("tri");

  PT(GeomVertexData) vdata = new GeomVertexData
    ("tri", GeomVertexFormat::get_v3n3cpt2(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  vertex.add_data3(LVertex::rfu(0.0, 0.0, 0.0));
  vertex.add_data3(LVertex::rfu(1.0, 0.0, 0.0));
  vertex.add_data3(LVertex::rfu(0.0, 0.0, 1.0));

  normal.add_data3(LNormal::back());
  normal.add_data3(LNormal::back());
  normal.add_data3(LNormal::back());

  texcoord.add_data2(0.0, 0.0);
  texcoord.add_data2(1.0, 0.0);
  texcoord.add_data2(0.0, 1.0);

  PT(GeomTriangles) tri = new GeomTriangles(Geom::UH_static);
  tri->add_consecutive_vertices(0, 3);
  tri->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(tri);

  geomnode->add_geom(geom, state);

  return parent.attach_new_node(geomnode);
}

/**
 * Looks for characters and their matching animation files in the scene graph;
 * binds and loops any matching animations found.
 */
void WindowFramework::
loop_animations(int hierarchy_match_flags) {
  // If we happened to load up both a character file and its matching
  // animation file, attempt to bind them together now and start the
  // animations looping.
  auto_bind(get_render().node(), _anim_controls, hierarchy_match_flags);
  _anim_controls.loop_all(true);
}

/**
 * Walks through all the animations that were bound by loop_animations() and
 * staggers their play rate slightly so that they will not remain perfectly in
 * sync.
 */
void WindowFramework::
stagger_animations() {
  for (int i = 0; i < _anim_controls.get_num_anims(); ++i) {
    AnimControl *control = _anim_controls.get_anim(i);
    double r = (double)rand() / (double)RAND_MAX;
    r = r * 0.2 + 0.9;
    control->set_play_rate(r);
  }
}

/**
 * Rotates the animation controls through all of the available animations.  If
 * the animation controls are not already enabled, enables them at sets to the
 * first animation; if they are already enabled, steps to the next animation;
 * if that is the last animation, disables the animation controls.
 */
void WindowFramework::
next_anim_control() {
  if (_anim_controls_enabled) {
    destroy_anim_controls();

    if (_anim_controls.get_num_anims() == 0) {
      set_anim_controls(false);
      return;
    }

    // Stop the active animation.
    pause_button();
    ++_anim_index;

    if (_anim_index >= _anim_controls.get_num_anims()) {
      set_anim_controls(false);
      _anim_controls.loop_all(true);
    } else {
      create_anim_controls();
      play_button();
    }
  } else {
    _anim_index = 0;
    set_anim_controls(true);
    if (_anim_controls.get_num_anims() > 0) {
      play_button();
    }
  }
}

/**
 * Creates an onscreen animation slider for frame-stepping through the
 * animations.
 */
void WindowFramework::
set_anim_controls(bool enable) {
  _anim_controls_enabled = enable;
  if (_anim_controls_enabled) {
    create_anim_controls();

  } else {
    destroy_anim_controls();
  }
}

/**
 * Reevaluates the dimensions of the window, presumably after the window has
 * been resized by the user or some other force.  Adjusts the render film
 * size, aspect2d scale (aspect ratio) and the dimensionsas of pixel_2d
 * according to the new window shape, or new config setting.
 */
void WindowFramework::
adjust_dimensions() {
  PN_stdfloat this_aspect_ratio = aspect_ratio;

  int x_size = 0, y_size = 0;
  if (_window->has_size()) {
    x_size = _window->get_sbs_left_x_size();
    y_size = _window->get_sbs_left_y_size();
  }

  if (this_aspect_ratio == 0.0f) {
    // An aspect ratio of 0.0 means to try to infer it.
    this_aspect_ratio = 1.0f;
    if (y_size != 0) {
      this_aspect_ratio = (float)x_size / (float)y_size;
    }
  }

  if (!_aspect_2d.is_empty()) {
    _aspect_2d.set_scale(1.0f / this_aspect_ratio, 1.0f, 1.0f);
  }

  if (!_pixel_2d.is_empty()) {
    // Adjust the pixel 2d scale
    if (x_size > 0){
      _pixel_2d.set_sx(2.0f / (float)x_size);
    }
    _pixel_2d.set_sy(1.0f);
    if (y_size > 0){
      _pixel_2d.set_sz(2.0f / (float)y_size);
    }
  }

  Cameras::iterator ci;
  for (ci = _cameras.begin(); ci != _cameras.end(); ++ci) {
    Lens *lens = (*ci)->get_lens();
    if (lens != nullptr) {
      if (y_size != 0) {
        lens->set_film_size(x_size, y_size);
      } else {
        lens->set_aspect_ratio(this_aspect_ratio);
      }
    }
  }
}

/**
 * Divides the window into two display regions, each of which gets its own
 * trackball and keyboard events.  The new window pointer is returned.
 *
 * There is not an interface for recombining divided windows.
 */
WindowFramework *WindowFramework::
split_window(SplitType split_type) {
  DisplayRegion *new_region = nullptr;

  if (split_type == ST_default) {
    // Choose either horizontal or vertical according to the largest
    // dimension.

    if (_display_region_3d->get_pixel_width() >
        _display_region_3d->get_pixel_height()) {
      split_type = ST_horizontal;
    } else {
      split_type = ST_vertical;
    }
  }

  PN_stdfloat left, right, bottom, top;
  _display_region_3d->get_dimensions(left, right, bottom, top);
  new_region = _display_region_3d->get_window()->make_display_region();

  if (split_type == ST_vertical) {
    _display_region_3d->set_dimensions(left, right, bottom, (top + bottom) / 2.0f);
    if (_display_region_2d != nullptr) {
      _display_region_2d->set_dimensions(left, right, bottom, (top + bottom) / 2.0f);
    }

    new_region->set_dimensions(left, right, (top + bottom) / 2.0f, top);

  } else {
    _display_region_3d->set_dimensions(left, (left + right) / 2.0f, bottom, top);
    if (_display_region_2d != nullptr) {
      _display_region_2d->set_dimensions(left, (left + right) / 2.0f, bottom, top);
    }

    new_region->set_dimensions((left + right) / 2.0f, right, bottom, top);
  }

  PT(WindowFramework) wf = new WindowFramework(*this, new_region);
  _panda_framework->_windows.push_back(wf);

  return wf;
}

/**
 * Forces wireframe state (true) or restores default rendering (false).
 */
void WindowFramework::
set_wireframe(bool enable, bool filled) {
  if (enable == _wireframe_enabled && filled == _wireframe_filled) {
    return;
  }

  NodePath render = get_render();

  if (!_two_sided_enabled) {
    render.clear_two_sided();
  }

  if (enable) {
    if (filled) {
      render.set_attrib(RenderModeAttrib::make(
        RenderModeAttrib::M_filled_wireframe,
        1.4f, false, LColor(1, 1, 1, .5f)),
        override_priority);
      // Darken the scene so that the wireframe is clearly visible, even when
      // the scene is completely white.
      render.set_color_scale(LColor(0.7f, 0.7f, 0.7f, 1), override_priority);
    } else {
      render.set_render_mode_wireframe(override_priority);
      render.set_two_sided(true, override_priority);
      render.clear_color_scale();
    }
  } else {
    render.clear_render_mode();
    if (_one_sided_reverse_enabled) {
      CPT(RenderAttrib) attrib = CullFaceAttrib::make_reverse();
      render.node()->set_attrib(attrib);
    }
    render.clear_color_scale();
  }

  _wireframe_enabled = enable;
  _wireframe_filled = filled;
}

/**
 * Forces textures off (false) or restores default rendering (true).
 */
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

/**
 * Forces two-sided rendering (true) or restores default rendering (false).
 */
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
  _one_sided_reverse_enabled = false;
}

/**
 * Toggles one-sided reverse mode.  In this mode, the front sides of one-sided
 * polygons are culled instead of the back side.
 */
void WindowFramework::
set_one_sided_reverse(bool enable) {
  if (enable == _one_sided_reverse_enabled) {
    return;
  }

  NodePath render = get_render();

  if (!_wireframe_enabled) {
    if (enable) {
      CPT(RenderAttrib) attrib = CullFaceAttrib::make_reverse();
      render.node()->set_attrib(attrib);
    } else {
      render.clear_two_sided();
    }
  }

  _two_sided_enabled = false;
  _one_sided_reverse_enabled = enable;
}

/**
 * Turns lighting on (true) or off (false).
 */
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
    render.set_light(_alight);
    render.set_light(_dlight);
  } else {
    render.clear_light();
  }

  _lighting_enabled = enable;
}

/**
 * Turns per-pixel lighting on (true) or off (false).
 */
void WindowFramework::
set_perpixel(bool enable) {
  if (enable == _perpixel_enabled) {
    return;
  }

  NodePath render = get_render();

  if (enable) {
    render.set_shader_auto();
  } else {
    render.set_shader_off();
  }

  _perpixel_enabled = enable;
}

/**
 * Sets the background of the window to one of the pre-canned background types
 * (or to BT_other, which indicates the user intends to set up his own special
 * background mode).
 */
void WindowFramework::
set_background_type(WindowFramework::BackgroundType type) {
  _background_type = type;

  if (_display_region_3d == nullptr) {
    return;
  }

  switch (_background_type) {
  case BT_other:
    break;

  case BT_default:
    _display_region_3d->set_clear_color_active(true);
    _display_region_3d->set_clear_depth_active(true);
    _display_region_3d->set_clear_stencil_active(true);
    _display_region_3d->set_clear_color(_window->get_clear_color());
    _display_region_3d->set_clear_depth(_window->get_clear_depth());
    _display_region_3d->set_clear_stencil(_window->get_clear_stencil());
    break;

  case BT_black:
    _display_region_3d->set_clear_color_active(true);
    _display_region_3d->set_clear_depth_active(true);
    _display_region_3d->set_clear_stencil_active(true);
    _display_region_3d->set_clear_color(LColor(0.0f, 0.0f, 0.0f, 0.0f));
    _display_region_3d->set_clear_depth(1.0f);
    _display_region_3d->set_clear_stencil(0);
    break;

  case BT_gray:
    _display_region_3d->set_clear_color_active(true);
    _display_region_3d->set_clear_depth_active(true);
    _display_region_3d->set_clear_stencil_active(true);
    _display_region_3d->set_clear_color(LColor(0.3, 0.3, 0.3, 0.0f));
    _display_region_3d->set_clear_depth(1.0f);
    _display_region_3d->set_clear_stencil(0);
    break;

  case BT_white:
    _display_region_3d->set_clear_color_active(true);
    _display_region_3d->set_clear_depth_active(true);
    _display_region_3d->set_clear_stencil_active(true);
    _display_region_3d->set_clear_color(LColor(1.0f, 1.0f, 1.0f, 0.0f));
    _display_region_3d->set_clear_depth(1.0f);
    _display_region_3d->set_clear_stencil(0);
    break;

  case BT_none:
    _display_region_3d->set_clear_color_active(false);
    _display_region_3d->set_clear_depth_active(false);
    _display_region_3d->set_clear_stencil_active(false);
    break;
  }
}

/**
 * Returns a font that contains the shuttle controls icons.
 */
TextFont *WindowFramework::
get_shuttle_controls_font() {
  if (_shuttle_controls_font == nullptr) {
    PT(TextFont) font;

    string shuttle_controls_string((const char *)shuttle_controls, shuttle_controls_len);
    istringstream in(shuttle_controls_string);
    BamFile bam_file;
    if (bam_file.open_read(in, "shuttle_controls font stream")) {
      PT(PandaNode) node = bam_file.read_node();
      if (node != nullptr) {
        _shuttle_controls_font = new StaticTextFont(node);
      }
    }
  }

  return _shuttle_controls_font;
}

/**
 * Makes a new 3-d camera for the window.
 */
NodePath WindowFramework::
make_camera() {
  // Finally, we need a camera to associate with the display region.
  PT(Camera) camera = new Camera("camera");
  NodePath camera_np = get_camera_group().attach_new_node(camera);
  _cameras.push_back(camera);

  PT(Lens) lens = new PerspectiveLens;

  if (aspect_ratio != 0.0) {
    // If we're given an explict aspect ratio, use it
    lens->set_aspect_ratio(aspect_ratio);

  } else {
    // Otherwise, infer the aspect ratio from the window size.  This does
    // assume we have square pixels on our output device.
    if (_window->has_size()) {
      int x_size = _window->get_sbs_left_x_size();
      int y_size = _window->get_sbs_left_y_size();
      if (y_size != 0) {
        lens->set_film_size(x_size, y_size);
      }
    }
  }

  camera->set_lens(lens);

  return camera_np;
}

/**
 * Makes light nodes and attaches them to the camera for viewing the scene.
 */
void WindowFramework::
setup_lights() {
  if (_got_lights) {
    return;
  }

  NodePath camera_group = get_camera_group();
  NodePath light_group = camera_group.attach_new_node("lights");

  AmbientLight *alight = new AmbientLight("ambient");
  alight->set_color(LColor(0.2, 0.2, 0.2, 1.0f));
  DirectionalLight *dlight = new DirectionalLight("directional");
  dlight->set_color(LColor(0.8f, 0.8f, 0.8f, 1.0f));

  _alight = light_group.attach_new_node(alight);
  _dlight = light_group.attach_new_node(dlight);
  _dlight.set_hpr(-10, -20, 0);

  _got_lights = true;
}

/**
 * Loads the indicated image file as a texture, and creates a polygon to
 * render it.  Returns the new model.
 */
PT(PandaNode) WindowFramework::
load_image_as_model(const Filename &filename) {
  PT(Texture) tex = TexturePool::load_texture(filename);
  if (tex == nullptr) {
    return nullptr;
  }

  // Yes, it is an image file; make a texture out of it.
  tex->set_minfilter(SamplerState::FT_linear_mipmap_linear);
  tex->set_magfilter(SamplerState::FT_linear);
  tex->set_wrap_u(SamplerState::WM_clamp);
  tex->set_wrap_v(SamplerState::WM_clamp);
  tex->set_wrap_w(SamplerState::WM_clamp);

  // Ok, now make a polygon to show the texture.
  bool has_alpha = true;
  LVecBase2 tex_scale = tex->get_tex_scale();

  // Get the size from the original image (the texture may have scaled it to
  // make a power of 2).
  int x_size = tex->get_orig_file_x_size();
  int y_size = tex->get_orig_file_y_size();

  // Choose the dimensions of the polygon appropriately.
  PN_stdfloat left,right,top,bottom;
  static const PN_stdfloat scale = 10.0;
  if (x_size > y_size) {
    left   = -scale;
    right  =  scale;
    top    =  (scale * y_size) / x_size;
    bottom = -(scale * y_size) / x_size;
  } else if (y_size != 0) {
    left   = -(scale * x_size) / y_size;
    right  =  (scale * x_size) / y_size;
    top    =  scale;
    bottom = -scale;
  } else {
    framework_cat.warning()
      << "Texture size is 0 0: " << *tex << "\n";

    left   = -scale;
    right  =  scale;
    top    =  scale;
    bottom = -scale;
  }

  PT(GeomNode) card_node = new GeomNode("card");
  card_node->set_attrib(TextureAttrib::make(tex));
  if (has_alpha) {
    card_node->set_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  bool is_3d = false;
  if (tex->get_texture_type() == Texture::TT_3d_texture ||
      tex->get_texture_type() == Texture::TT_cube_map) {
    // For a 3-d texture, generate a cube, instead of a plain card.
    is_3d = true;
  }

  CPT(GeomVertexFormat) vformat;
  if (!is_3d) {
    // Vertices and 2-d texture coordinates, all we need.
    vformat = GeomVertexFormat::get_v3t2();

  } else {
    // Vertices and 3-d texture coordinates.
    vformat = GeomVertexFormat::register_format
      (new GeomVertexArrayFormat
       (InternalName::get_vertex(), 3,
        GeomEnums::NT_stdfloat, GeomEnums::C_point,
        InternalName::get_texcoord(), 3,
        GeomEnums::NT_stdfloat, GeomEnums::C_texcoord));
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("card", vformat, Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  if (!is_3d) {
    // A normal 2-d card.
    vertex.add_data3(LVertex::rfu(left, 0.02, top));
    vertex.add_data3(LVertex::rfu(left, 0.02, bottom));
    vertex.add_data3(LVertex::rfu(right, 0.02, top));
    vertex.add_data3(LVertex::rfu(right, 0.02, bottom));

    texcoord.add_data2(0.0f, tex_scale[1]);
    texcoord.add_data2(0.0f, 0.0f);
    texcoord.add_data2(tex_scale[0], tex_scale[1]);
    texcoord.add_data2(tex_scale[0], 0.0f);

  } else {
    // The eight vertices of a 3-d cube.
    vertex.add_data3(-1.0f, -1.0f, 1.0f);   // 0
    vertex.add_data3(-1.0f, -1.0f, -1.0f);  // 1
    vertex.add_data3(1.0f, -1.0f, -1.0f);   // 2
    vertex.add_data3(1.0f, -1.0f, 1.0f);    // 3
    vertex.add_data3(1.0f, 1.0f, 1.0f);     // 4
    vertex.add_data3(1.0f, 1.0f, -1.0f);    // 5
    vertex.add_data3(-1.0f, 1.0f, -1.0f);   // 6
    vertex.add_data3(-1.0f, 1.0f, 1.0f);    // 7

    texcoord.add_data3(-1.0f, -1.0f, 1.0f);   // 0
    texcoord.add_data3(-1.0f, -1.0f, -1.0f);  // 1
    texcoord.add_data3(1.0f, -1.0f, -1.0f);   // 2
    texcoord.add_data3(1.0f, -1.0f, 1.0f);    // 3
    texcoord.add_data3(1.0f, 1.0f, 1.0f);     // 4
    texcoord.add_data3(1.0f, 1.0f, -1.0f);    // 5
    texcoord.add_data3(-1.0f, 1.0f, -1.0f);   // 6
    texcoord.add_data3(-1.0f, 1.0f, 1.0f);    // 7
  }

  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);

  if (!is_3d) {
    // The two triangles that make up a quad.
    strip->add_consecutive_vertices(0, 4);
    strip->close_primitive();

  } else {
    // The twelve triangles (six quads) that make up a cube.
    strip->add_vertex(7);
    strip->add_vertex(0);
    strip->add_vertex(4);
    strip->add_vertex(3);
    strip->close_primitive();

    strip->add_vertex(1);
    strip->add_vertex(6);
    strip->add_vertex(2);
    strip->add_vertex(5);
    strip->close_primitive();

    strip->add_vertex(5);
    strip->add_vertex(4);
    strip->add_vertex(2);
    strip->add_vertex(3);
    strip->add_vertex(1);
    strip->add_vertex(0);
    strip->add_vertex(6);
    strip->add_vertex(7);
    strip->add_vertex(5);
    strip->add_vertex(4);
    strip->close_primitive();
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);

  card_node->add_geom(geom);

  return card_node;
}

/**
 * Creates an onscreen animation slider for frame-stepping through the
 * animations.
 */
void WindowFramework::
create_anim_controls() {
  destroy_anim_controls();

  PT(PGItem) group = new PGItem("anim_controls_group");
  PGFrameStyle style;
  style.set_type(PGFrameStyle::T_flat);
  style.set_color(0.0f, 0.0f, 0.0f, 0.3);
  group->set_frame(-1.0f, 1.0f, 0.0f, 0.2);
  group->set_frame_style(0, style);
  group->set_suppress_flags(MouseWatcherRegion::SF_mouse_button);
  group->set_active(true);

  _anim_controls_group = get_aspect_2d().attach_new_node(group);
  _anim_controls_group.set_pos(0.0f, 0.0f, -0.9f);

  if (_anim_index >= _anim_controls.get_num_anims()) {
    PT(TextNode) label = new TextNode("label");
    label->set_align(TextNode::A_center);
    label->set_text("No animation.");
    NodePath tnp = _anim_controls_group.attach_new_node(label);
    tnp.set_pos(0.0f, 0.0f, 0.07f);
    tnp.set_scale(0.1f);

    return;
  }

  AnimControl *control = _anim_controls.get_anim(_anim_index);
  nassertv(control != nullptr);

  if (control->get_num_frames() <= 1) {
    // Don't show the controls when the animation has only 0 or 1 frames.
    ostringstream text;
    text << _anim_controls.get_anim_name(_anim_index);
    text << " (" << control->get_num_frames() << " frame"
         << ((control->get_num_frames() == 1) ? "" : "s") << ")";

    PT(TextNode) label = new TextNode("label");
    label->set_align(TextNode::A_center);
    label->set_text(text.str());
    NodePath tnp = _anim_controls_group.attach_new_node(label);
    tnp.set_pos(0.0f, 0.0f, 0.07f);
    tnp.set_scale(0.1f);

    return;
  }

  PT(TextNode) label = new TextNode("anim_name");
  label->set_align(TextNode::A_left);
  label->set_text(_anim_controls.get_anim_name(_anim_index));
  NodePath tnp = _anim_controls_group.attach_new_node(label);
  tnp.set_pos(-0.95f, 0.0f, 0.15f);
  tnp.set_scale(0.05f);

  _anim_slider = new PGSliderBar("anim_slider");
  _anim_slider->setup_slider(false, 1.9f, 0.1f, 0.005f);
  _anim_slider->set_suppress_flags(MouseWatcherRegion::SF_mouse_button);
  _anim_slider->get_thumb_button()->set_suppress_flags(MouseWatcherRegion::SF_mouse_button);

  _anim_slider->set_range(0.0f, (PN_stdfloat)(control->get_num_frames() - 1));
  _anim_slider->set_scroll_size(0.0f);
  _anim_slider->set_page_size(1.0f);
  NodePath snp = _anim_controls_group.attach_new_node(_anim_slider);
  snp.set_pos(0.0f, 0.0f, 0.06f);

  _frame_number = new TextNode("frame_number");
  _frame_number->set_text_color(0.0f, 0.0f, 0.0f, 1.0f);
  _frame_number->set_align(TextNode::A_center);
  _frame_number->set_text(format_string(control->get_frame()));
  NodePath fnp = NodePath(_anim_slider->get_thumb_button()).attach_new_node(_frame_number);
  fnp.set_scale(0.05f);
  fnp.set_pos(0.0f, 0.0f, -0.01f);

  _play_rate_slider = new PGSliderBar("play_rate_slider");
  _play_rate_slider->setup_slider(false, 0.4, 0.05f, 0.005f);
  _play_rate_slider->set_suppress_flags(MouseWatcherRegion::SF_mouse_button);
  _play_rate_slider->get_thumb_button()->set_suppress_flags(MouseWatcherRegion::SF_mouse_button);
  _play_rate_slider->set_value(control->get_play_rate());
  NodePath pnp = _anim_controls_group.attach_new_node(_play_rate_slider);
  pnp.set_pos(0.75f, 0.0f, 0.15f);

  // Set up the jogshuttle buttons.  These use symbols from the
  // shuttle_controls_font file.
  setup_shuttle_button("9", 0, st_back_button);
  setup_shuttle_button(";", 1, st_pause_button);
  setup_shuttle_button("4", 2, st_play_button);
  setup_shuttle_button(":", 3, st_forward_button);

  _update_anim_controls_task = new GenericAsyncTask("controls", st_update_anim_controls, (void *)this);
  _panda_framework->get_task_mgr().add(_update_anim_controls_task);
}

/**
 * Removes the previously-created anim controls, if any.
 */
void WindowFramework::
destroy_anim_controls() {
  if (!_anim_controls_group.is_empty()) {
    _anim_controls_group.remove_node();

    _panda_framework->get_event_handler().remove_hooks_with((void *)this);
    if (_update_anim_controls_task != nullptr) {
      _panda_framework->get_task_mgr().remove(_update_anim_controls_task);
      _update_anim_controls_task.clear();
    }
  }
}

/**
 * A per-frame callback to update the anim slider for the current frame.
 */
void WindowFramework::
update_anim_controls() {
  AnimControl *control = _anim_controls.get_anim(_anim_index);
  nassertv(control != nullptr);

  if (_anim_slider != nullptr) {
    if (_anim_slider->is_button_down()) {
      control->pose((int)(_anim_slider->get_value() + 0.5));
    } else {
      _anim_slider->set_value((PN_stdfloat)control->get_frame());
    }
  }

  if (_frame_number != nullptr) {
    _frame_number->set_text(format_string(control->get_frame()));
  }

  control->set_play_rate(_play_rate_slider->get_value());
}

/**
 * Creates a PGButton to implement the indicated shuttle event (play, pause,
 * etc.).
 */
void WindowFramework::
setup_shuttle_button(const string &label, int index,
                     EventHandler::EventCallbackFunction *func) {
  PT(PGButton) button = new PGButton(label);
  button->set_frame(-0.05f, 0.05f, 0.0f, 0.07f);

  PN_stdfloat bevel = 0.005f;

  PGFrameStyle style;
  style.set_color(0.8f, 0.8f, 0.8f, 1.0f);
  style.set_width(bevel, bevel);

  style.set_type(PGFrameStyle::T_bevel_out);
  button->set_frame_style(PGButton::S_ready, style);

  style.set_type(PGFrameStyle::T_bevel_in);
  button->set_frame_style(PGButton::S_depressed, style);

  style.set_color(0.9f, 0.9f, 0.9f, 1.0f);
  style.set_type(PGFrameStyle::T_bevel_out);
  button->set_frame_style(PGButton::S_rollover, style);

  if (get_shuttle_controls_font() != nullptr) {
    PT(TextNode) tn = new TextNode("label");
    tn->set_align(TextNode::A_center);
    tn->set_font(get_shuttle_controls_font());
    tn->set_text(label);
    tn->set_text_color(0.0f, 0.0f, 0.0f, 1.0f);
    LMatrix4 xform = LMatrix4::scale_mat(0.07f);
    xform.set_row(3, LVecBase3(0.0f, 0.0f, 0.016f));
    tn->set_transform(xform);

    button->get_state_def(PGButton::S_ready).attach_new_node(tn);
    button->get_state_def(PGButton::S_depressed).attach_new_node(tn);
    button->get_state_def(PGButton::S_rollover).attach_new_node(tn);
  }

  NodePath np = _anim_controls_group.attach_new_node(button);
  np.set_pos(0.1f * index - 0.15f, 0.0f, 0.12);

  _panda_framework->get_event_handler().add_hook(button->get_click_event(MouseButton::one()), func, (void *)this);
}

/**
 * Handler for a shuttle button.
 */
void WindowFramework::
back_button() {
  AnimControl *control = _anim_controls.get_anim(_anim_index);
  nassertv(control != nullptr);
  control->pose(control->get_frame() - 1);
}

/**
 * Handler for a shuttle button.
 */
void WindowFramework::
pause_button() {
  AnimControl *control = _anim_controls.get_anim(_anim_index);
  nassertv(control != nullptr);
  control->stop();
}

/**
 * Handler for a shuttle button.
 */
void WindowFramework::
play_button() {
  AnimControl *control = _anim_controls.get_anim(_anim_index);
  nassertv(control != nullptr);
  control->loop(false);
}

/**
 * Handler for a shuttle button.
 */
void WindowFramework::
forward_button() {
  AnimControl *control = _anim_controls.get_anim(_anim_index);
  nassertv(control != nullptr);
  control->pose(control->get_frame() + 1);
}


/**
 * The static task function.
 */
AsyncTask::DoneStatus WindowFramework::
st_update_anim_controls(GenericAsyncTask *, void *data) {
  WindowFramework *self = (WindowFramework *)data;
  self->update_anim_controls();
  return AsyncTask::DS_cont;
}


/**
 * The static event handler function.
 */
void WindowFramework::
st_back_button(const Event *, void *data) {
  WindowFramework *self = (WindowFramework *)data;
  self->back_button();
}

/**
 * The static event handler function.
 */
void WindowFramework::
st_pause_button(const Event *, void *data) {
  WindowFramework *self = (WindowFramework *)data;
  self->pause_button();
}

/**
 * The static event handler function.
 */
void WindowFramework::
st_play_button(const Event *, void *data) {
  WindowFramework *self = (WindowFramework *)data;
  self->play_button();
}

/**
 * The static event handler function.
 */
void WindowFramework::
st_forward_button(const Event *, void *data) {
  WindowFramework *self = (WindowFramework *)data;
  self->forward_button();
}
