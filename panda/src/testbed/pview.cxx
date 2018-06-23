/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pview.cxx
 * @author drose
 * @date 2002-02-25
 */

#include "pandaFramework.h"
#include "pandaSystem.h"
#include "textNode.h"
#include "configVariableBool.h"
#include "texturePool.h"
#include "multitexReducer.h"
#include "sceneGraphReducer.h"
#include "partGroup.h"
#include "cardMaker.h"
#include "bamCache.h"
#include "virtualFileSystem.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"
#include "graphicsPipeSelection.h"
#include "asyncTaskManager.h"
#include "asyncTask.h"
#include "boundingSphere.h"

using std::cerr;
using std::endl;

PandaFramework framework;

ConfigVariableBool pview_test_hack
("pview-test-hack", false,
 "Enable the '0' key in pview to run whatever hacky test happens to be in "
 "there right now.");

bool
output_screenshot(Filename &fn)
{
  Thread *current_thread = Thread::get_current_thread();

  // Only one frame crashes.
  framework.do_frame(current_thread);
  framework.do_frame(current_thread);

  WindowFramework *wf = framework.get_window(0);
  bool ok = wf->get_graphics_output()->save_screenshot(fn, "from pview");
  if (!ok) {
    cerr << "Could not generate screenshot " << fn << "\n";
  }
  return ok;
}

void
event_W(const Event *, void *) {
  // shift-W: open a new window on the same scene.

  // If we already have a window, use the same GSG.
  GraphicsPipe *pipe = nullptr;
  GraphicsStateGuardian *gsg = nullptr;

  if (framework.get_num_windows() > 0) {
    WindowFramework *old_window = framework.get_window(0);
    GraphicsOutput *win = old_window->get_graphics_output();
    pipe = win->get_pipe();
    // gsg = win->get_gsg();
  }

  WindowFramework *window = framework.open_window(pipe, gsg);
  if (window != nullptr) {
    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());
  }
}

void
event_F(const Event *, void *) {
  // shift-F: flatten the model hierarchy.
  framework.get_models().flatten_strong();
}

void
event_Enter(const Event *, void *) {
  // alt-enter: toggle between windowfullscreen in the same scene.

  // If we already have a window, use the same GSG.
  GraphicsPipe *pipe = nullptr;
  GraphicsStateGuardian *gsg = nullptr;

  WindowProperties props;

  for (int i = 0; i < framework.get_num_windows(); ++i) {
    WindowFramework *old_window = framework.get_window(i);
    GraphicsWindow *win = old_window->get_graphics_window();
    if (win != nullptr) {
      pipe = win->get_pipe();
      gsg = win->get_gsg();
      props = win->get_properties();
      framework.close_window(old_window);
      break;
    }
  }

  // set the toggle
  props.set_fullscreen(!props.get_fullscreen());
  int flags = GraphicsPipe::BF_require_window;

  WindowFramework *window = framework.open_window(props, flags, pipe, gsg);
  if (window != nullptr) {
    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());
  }
}

void
event_2(const Event *event, void *) {
  // 2: split the window into two display regions.

  EventParameter param = event->get_parameter(0);
  WindowFramework *wf;
  DCAST_INTO_V(wf, param.get_ptr());

  WindowFramework *split = wf->split_window();
  if (split != nullptr) {
    split->enable_keyboard();
    split->setup_trackball();
    framework.get_models().instance_to(split->get_render());
  }
}

void
event_0(const Event *event, void *) {
  // 0: run hacky test.
  EventParameter param = event->get_parameter(0);
  WindowFramework *wf;
  DCAST_INTO_V(wf, param.get_ptr());

  // Create a new offscreen buffer.
  GraphicsOutput *win = wf->get_graphics_output();
  PT(GraphicsOutput) buffer = win->make_texture_buffer("tex", 256, 256);
  cerr << buffer->get_type() << "\n";

  // Set the offscreen buffer to render the same scene as the main camera.
  DisplayRegion *dr = buffer->make_display_region();
  dr->set_camera(NodePath(wf->get_camera(0)));

  // Make the clear color on the buffer be yellow, so it's obviously different
  // from the main scene's background color.
  buffer->set_clear_color(LColor(1, 1, 0, 0));

  // Apply the offscreen buffer's texture to a card in the main window.
  CardMaker cm("card");
  cm.set_frame(0, 1, 0, 1);
  NodePath card_np(cm.generate());

  card_np.reparent_to(wf->get_render_2d());
  card_np.set_texture(buffer->get_texture());
}

void
usage() {
  cerr <<
    "\n"
    "Usage: pview [opts] model [model ...]\n"
    "       pview -h\n\n";
}

void
help() {
  usage();
  cerr <<
    "pview opens a quick Panda window for viewing one or more models and/or\n"
    "animations.\n\n"

    "Options:\n\n"

    "  -a\n"
    "      Convert and play animations, if loading an external file type\n"
    "      (like .mb) directly and if the converter supports animations.\n"
    "      Also implicitly enables the animation controls.\n\n"

    "  -c\n"
    "      Automatically center models within the viewing window on startup.\n"
    "      This can also be achieved with the 'c' hotkey at runtime.\n\n"

    "  -l\n"
    "      Open the window before loading any models with the text \"Loading\"\n"
    "      displayed in the window.  The default is not to open the window\n"
    "      until all models are loaded.\n\n"

    "  -i\n"
    "      Ignore bundle/group names.  Normally, the <group> name must match\n"
    "      the <bundle> name, or the animation will not be used.\n\n"

    "  -s filename\n"
    "      After displaying the models, immediately take a screenshot and\n"
    "      exit.\n\n"

    "  -D\n"
    "      Delete the model files after loading them (presumably this option\n"
    "      will only be used when loading a temporary model file).\n\n"

    "  -L\n"
    "      Enable lighting in the scene.  This can also be achieved with\n"
    "      the 'l' hotkey at runtime.\n\n"

    "  -P <pipe>\n"
    "      Select the given graphics pipe for the window, rather than using\n"
    "      the platform default.  The allowed values for <pipe> are those\n"
    "      from the Config.prc variables 'load-display' and 'aux-display'.\n\n"

    "  -V\n"
    "      Report the current version of Panda, and exit.\n\n"

    "  -h\n"
    "      Display this help text.\n\n";
}

void
report_version() {
  nout << "\n";
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->write(nout);
  nout << "\n";
}

// Task that dynamically adjusts the camera len's near/far clipping
// planes to ensure the user can zoom in as close as needed to a model.
//
// Code adapted from WindowFramework::center_trackball(), but
// without moving the camera.  When the camera is inside the model,
// the near clip is set to near-zero.
//
class AdjustCameraClipPlanesTask : public AsyncTask {
public:
  AdjustCameraClipPlanesTask(const std::string &name, Camera *camera) :
    AsyncTask(name), _camera(camera), _lens(camera->get_lens(0)), _sphere(nullptr)
  {
    NodePath np = framework.get_models();
    PT(BoundingVolume) volume = np.get_bounds();

    // We expect at least a geometric bounding volume around the world.
    nassertv(volume != nullptr);
    nassertv(volume->is_of_type(GeometricBoundingVolume::get_class_type()));
    CPT(GeometricBoundingVolume) gbv = DCAST(GeometricBoundingVolume, volume);

    if (np.has_parent()) {
      CPT(TransformState) net_transform = np.get_parent().get_net_transform();
      PT(GeometricBoundingVolume) new_gbv = DCAST(GeometricBoundingVolume, gbv->make_copy());
      new_gbv->xform(net_transform->get_mat());
      gbv = new_gbv;
    }

    // Determine the bounding sphere around the object.
    if (gbv->is_infinite()) {
      framework_cat.warning()
        << "Infinite bounding volume for " << np << "\n";
      return;
    }

    if (gbv->is_empty()) {
      framework_cat.warning()
        << "Empty bounding volume for " << np << "\n";
      return;
    }

    // The BoundingVolume might be a sphere (it's likely), but since it
    // might not, we'll take no chances and make our own sphere.
    _sphere = new BoundingSphere(gbv->get_approx_center(), 0.0f);
    if (!_sphere->extend_by(gbv)) {
      framework_cat.warning()
        << "Cannot determine bounding volume of " << np << "\n";
      return;
    }
  }
  ALLOC_DELETED_CHAIN(AdjustCameraClipPlanesTask);

  virtual DoneStatus do_task() {
    if (!_sphere) {
      return DS_done;
    }

    if (framework.get_num_windows() == 0) {
      return DS_cont;
    }

    WindowFramework *wf = framework.get_window(0);
    if (!wf) {
      return DS_cont;
    }

    // Get current camera position.
    NodePath cameraNP = wf->get_camera_group();
    LPoint3 pos = cameraNP.get_pos();

    // See how far or close the camera is
    LPoint3 center = _sphere->get_center();
    PN_stdfloat radius = _sphere->get_radius();

    PN_stdfloat min_distance = 0.001 * radius;

    // Choose a suitable distance to view the whole volume in our frame.
    // This is based on the camera lens in use.
    PN_stdfloat distance;
    CPT(GeometricBoundingVolume) gbv = DCAST(GeometricBoundingVolume, _sphere);
    if (gbv->contains(pos)) {
      // See as up-close to the model as possible
      distance = min_distance;
    } else {
      // View from a distance
      distance = (center - pos).length();
    }

    // Ensure the far plane is far enough back to see the entire object.
    PN_stdfloat ideal_far_plane = distance + radius * 1.5;
    _lens->set_far(std::max(_lens->get_default_far(), ideal_far_plane));

    // And that the near plane is far enough forward, but if inside
    // the sphere, keep above 0.
    PN_stdfloat ideal_near_plane = std::max(min_distance * 10, distance - radius);
    _lens->set_near(std::min(_lens->get_default_near(), ideal_near_plane));

    return DS_cont;
  }

  Camera *_camera;
  Lens *_lens;
  PT(BoundingSphere) _sphere;
};


int
main(int argc, char **argv) {
  preprocess_argv(argc, argv);
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");

  bool anim_controls = false;
  bool auto_center = false;
  bool show_loading = false;
  bool auto_screenshot = false;
  int hierarchy_match_flags = PartGroup::HMF_ok_part_extra |
                              PartGroup::HMF_ok_anim_extra;
  Filename screenshotfn;
  bool delete_models = false;
  bool apply_lighting = false;
  PointerTo<GraphicsPipe> pipe = nullptr;

  extern char *optarg;
  extern int optind;
  static const char *optflags = "acls:DVhiLP:";
  int flag = getopt(argc, argv, optflags);

  while (flag != EOF) {
    switch (flag) {
    case 'a':
      anim_controls = true;
      PandaFramework::_loader_options.set_flags(PandaFramework::_loader_options.get_flags() | LoaderOptions::LF_convert_anim);
      break;

    case 'c':
      auto_center = true;
      break;

    case 'l':
      show_loading = true;
      break;

    case 'i':
      hierarchy_match_flags |= PartGroup::HMF_ok_wrong_root_name;
      break;

    case 's':
      auto_screenshot = true;
      screenshotfn = optarg;
      break;

    case 'D':
      delete_models = true;
      break;

    case 'L':
      apply_lighting = true;
      break;

    case 'P': {
      pipe = GraphicsPipeSelection::get_global_ptr()->make_module_pipe(optarg);
      if (!pipe) {
        cerr << "No such pipe '" << optarg << "' available." << endl;
        return 1;
      }
      break;
    }

    case 'V':
      report_version();
      return 1;

    case 'h':
      help();
      return 1;

    case '?':
      usage();
      return 1;

    default:
      cerr << "Unhandled switch: " << flag << endl;
      break;
    }
    flag = getopt(argc, argv, optflags);
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  WindowFramework *window = framework.open_window(pipe, nullptr);
  if (window != nullptr) {
    // We've successfully opened a window.

    NodePath loading_np;

    if (show_loading) {
      // Put up a "loading" message for the user's benefit.
      NodePath aspect_2d = window->get_aspect_2d();
      PT(TextNode) loading = new TextNode("loading");
      loading_np = aspect_2d.attach_new_node(loading);
      loading_np.set_scale(0.125f);
      loading->set_text_color(1.0f, 1.0f, 1.0f, 1.0f);
      loading->set_shadow_color(0.0f, 0.0f, 0.0f, 1.0f);
      loading->set_shadow(0.04, 0.04);
      loading->set_align(TextNode::A_center);
      loading->set_text("Loading...");

      // Allow a couple of frames to go by so the window will be fully created
      // and the text will be visible.
      Thread *current_thread = Thread::get_current_thread();
      framework.do_frame(current_thread);
      framework.do_frame(current_thread);
    }

    window->enable_keyboard();
    window->setup_trackball();
    framework.get_models().instance_to(window->get_render());
    if (argc < 2) {
      // If we have no arguments, get that trusty old triangle out.
      window->load_default_model(framework.get_models());
    } else {
      window->load_models(framework.get_models(), argc, argv);

      if (delete_models) {
        VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
        for (int i = 1; i < argc && argv[i] != nullptr; i++) {
          Filename model = Filename::from_os_specific(argv[i]);
          if (vfs->exists(model)) {
            nout << "Deleting " << model << "\n";
            vfs->delete_file(model);
          }
        }
      }
    }
    window->loop_animations(hierarchy_match_flags);

    // Make sure the textures are preloaded.
    framework.get_models().prepare_scene(window->get_graphics_output()->get_gsg());

    loading_np.remove_node();

    if (apply_lighting) {
      window->set_lighting(true);
    }

    if (auto_center) {
      window->center_trackball(framework.get_models());
    }

    if (auto_screenshot) {
      return(output_screenshot(screenshotfn) ? 0:1);
    }

    if (anim_controls) {
      window->set_anim_controls(true);
    }

    PT(AdjustCameraClipPlanesTask) task = new AdjustCameraClipPlanesTask("Adjust Camera Bounds", window->get_camera(0));
    framework.get_task_mgr().add(task);

    framework.enable_default_keys();
    framework.define_key("shift-w", "open a new window", event_W, nullptr);
    framework.define_key("shift-f", "flatten hierarchy", event_F, nullptr);
    framework.define_key("alt-enter", "toggle between window/fullscreen", event_Enter, nullptr);
    framework.define_key("2", "split the window", event_2, nullptr);
    if (pview_test_hack) {
      framework.define_key("0", "run quick hacky test", event_0, nullptr);
    }
    framework.main_loop();
    framework.report_frame_rate(nout);
  }

  framework.close_framework();
  return (0);
}
