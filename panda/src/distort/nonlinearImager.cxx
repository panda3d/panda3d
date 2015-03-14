// Filename: nonlinearImager.cxx
// Created by:  drose (12Dec01)
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

#include "nonlinearImager.h"
#include "config_distort.h"

#include "graphicsStateGuardian.h"
#include "matrixLens.h"
#include "graphicsOutput.h"
#include "graphicsEngine.h"
#include "dcast.h"
#include "cPointerCallbackObject.h"
#include "asyncTaskManager.h"
#include "genericAsyncTask.h"

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NonlinearImager::
NonlinearImager() {
  _engine = (GraphicsEngine *)NULL;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NonlinearImager::
~NonlinearImager() {
  remove_all_screens();
  remove_all_viewers();

  if (_recompute_task != (AsyncTask *)NULL) {
    AsyncTaskManager *task_mgr = AsyncTaskManager::get_global_ptr();
    task_mgr->remove(_recompute_task);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::add_screen
//       Access: Published
//  Description: This version of this method is deprecated and will
//               soon be removed.  Use the version that takes two
//               parameters instead.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
add_screen(ProjectionScreen *screen) {
  return add_screen(NodePath(screen), screen->get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::add_screen
//       Access: Published
//  Description: Adds a new ProjectionScreen to the list of screens
//               that will be processed by the NonlinearImager.  Each
//               ProjectionScreen represents a view into the world.
//               It must be based on a linear camera (or whatever kind
//               of camera is respected by the graphics engine).
//
//               Each ProjectionScreen object should already have some
//               screen geometry created.
//
//               As each frame is rendered, an offscreen image will be
//               rendered from the source camera associated with each
//               ProjectionScreen, and the resulting image will be
//               applied to the screen geometry.
//
//               The return value is the index number of the new
//               screen.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
add_screen(const NodePath &screen, const string &name) {
  nassertr(!screen.is_empty() && 
           screen.node()->is_of_type(ProjectionScreen::get_class_type()), -1);

  ProjectionScreen *screen_node = DCAST(ProjectionScreen, screen.node());

  _screens.push_back(Screen());
  Screen &new_screen = _screens.back();
  new_screen._screen = screen;
  new_screen._screen_node = screen_node;
  new_screen._name = name;
  new_screen._buffer = (GraphicsOutput *)NULL;
  new_screen._tex_width = 256;
  new_screen._tex_height = 256;
  new_screen._active = true;

  // Slot a mesh for each viewer.
  size_t vi;
  for (vi = 0; vi < _viewers.size(); ++vi) {
    new_screen._meshes.push_back(Mesh());
    new_screen._meshes[vi]._last_screen = screen_node->get_last_screen();
  }

  _stale = true;

  if (_dark_room.is_empty()) {
    _dark_room = screen.get_top();
  } else {
    nassertr(_dark_room.is_same_graph(screen), _screens.size() - 1);
  }

  return _screens.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::find_screen
//       Access: Published
//  Description: Returns the index number of the first appearance of
//               the indicated screen within the imager's list, or -1
//               if it does not appear.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
find_screen(const NodePath &screen) const {
  for (size_t i = 0; i < _screens.size(); i++) {
    if (_screens[i]._screen == screen) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::remove_screen
//       Access: Published
//  Description: Removes the screen with the indicated index number
//               from the imager.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
remove_screen(int index) {
  nassertv_always(index >= 0 && index < (int)_screens.size());
  Screen &screen = _screens[index];
  for (size_t vi = 0; vi < screen._meshes.size(); vi++) {
    screen._meshes[vi]._mesh.remove_node();
  }
  _screens.erase(_screens.begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::remove_all_screens
//       Access: Published
//  Description: Removes all screens from the imager.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
remove_all_screens() {
  while (!_screens.empty()) {
    remove_screen(_screens.size() - 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_num_screens
//       Access: Published
//  Description: Returns the number of screens that have been added to
//               the imager.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
get_num_screens() const {
  return _screens.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_screen
//       Access: Published
//  Description: Returns the nth screen that has been added to the
//               imager.
////////////////////////////////////////////////////////////////////
NodePath NonlinearImager::
get_screen(int index) const {
  nassertr(index >= 0 && index < (int)_screens.size(), NodePath());
  return _screens[index]._screen;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_buffer
//       Access: Published
//  Description: Returns the offscreen buffer that is automatically
//               created for the nth projection screen.  This may
//               return NULL if the screen is inactive or if it has
//               not been rendered yet.
////////////////////////////////////////////////////////////////////
GraphicsOutput *NonlinearImager::
get_buffer(int index) const {
  nassertr(index >= 0 && index < (int)_screens.size(), (GraphicsOutput *)NULL);
  return _screens[index]._buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_texture_size
//       Access: Published
//  Description: Sets the width and height of the texture used to
//               render the scene for the indicated screen.  This must
//               be less than or equal to the window size, and it
//               should be a power of two.
//
//               In general, the larger the texture, the greater the
//               detail of the rendered scene.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_texture_size(int index, int width, int height) {
  nassertv(index >= 0 && index < (int)_screens.size());

  Screen &screen = _screens[index];

  screen._tex_width = width;
  screen._tex_height = height;

  if (screen._buffer != (GraphicsOutput *)NULL) {
    bool removed = _engine->remove_window(screen._buffer);
    screen._buffer = (GraphicsOutput *)NULL;
    nassertv(removed);
  }

  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_source_camera
//       Access: Published
//  Description: Specifies the camera that will be used to render the
//               image for this particular screen.
//
//               The parameter must be a NodePath whose node is a
//               Camera.  The camera itself indicates the scene that
//               is to be rendered.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_source_camera(int index, const NodePath &source_camera) {
  nassertv(index >= 0 && index < (int)_screens.size());
  nassertv(!source_camera.is_empty() && 
           source_camera.node()->is_of_type(Camera::get_class_type()));
  _screens[index]._source_camera = source_camera;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_screen_active
//       Access: Published
//  Description: Sets the active flag on the indicated screen.  If the
//               active flag is true, the screen will be used;
//               otherwise, it will not appear.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_screen_active(int index, bool active) {
  nassertv(index >= 0 && index < (int)_screens.size());

  Screen &screen = _screens[index];
  screen._active = active;

  if (!active) {
    // If we've just made this screen inactive, remove its meshes.
    for (size_t vi = 0; vi < screen._meshes.size(); vi++) {
      screen._meshes[vi]._mesh.remove_node();
    }

    // Also remove its buffer.
    if (screen._buffer != (GraphicsOutput *)NULL) {
      bool removed = _engine->remove_window(screen._buffer);
      screen._buffer = (GraphicsOutput *)NULL;
      nassertv(removed);
    }

    // Hide the screen in the dark room.  This doesn't really matter,
    // since the dark room isn't normally rendered, but hide it anyway
    // in case the user stuck a camera in there for fun.
    screen._screen.hide();

  } else {
    // If we've just made it active, it needs to be recomputed.
    _stale = true;

    screen._screen.show();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_screen_active
//       Access: Published
//  Description: Returns the active flag on the indicated screen.
////////////////////////////////////////////////////////////////////
bool NonlinearImager::
get_screen_active(int index) const {
  nassertr(index >= 0 && index < (int)_screens.size(), false);
  return _screens[index]._active;
}


////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::add_viewer
//       Access: Published
//  Description: Adds the indicated DisplayRegion as a viewer into the
//               NonlinearImager room.  The camera associated with the
//               DisplayRegion at the time add_viewer() is called is
//               used as the initial viewer camera; it may have a
//               nonlinear lens, like a fisheye or cylindrical lens.
//
//               This sets up a special scene graph for this
//               DisplayRegion alone and sets up the DisplayRegion
//               with a specialty camera.  If future changes to the
//               camera are desired, you should use the
//               set_viewer_camera() interface.
//
//               All viewers must share the same GraphicsEngine.
//
//               The return value is the index of the new viewer.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
add_viewer(DisplayRegion *dr) {
  GraphicsOutput *window = dr->get_window();
  nassertr(window != (GraphicsOutput *)NULL, -1);

  GraphicsStateGuardian *gsg = window->get_gsg();
  nassertr(gsg != (GraphicsStateGuardian *)NULL, -1);

  GraphicsEngine *engine = gsg->get_engine();
  nassertr(engine != (GraphicsEngine *)NULL, -1);

  nassertr(_viewers.empty() || (engine == _engine), -1);
  if (_engine == (GraphicsEngine *)NULL) {
    _engine = engine;
  }

  if (_recompute_task == (AsyncTask *)NULL) {
    _recompute_task = 
      new GenericAsyncTask("nli_recompute", recompute_callback, (void *)this);
    AsyncTaskManager *task_mgr = AsyncTaskManager::get_global_ptr();
    task_mgr->add(_recompute_task);
  }

  int previous_vi = find_viewer(dr);
  if (previous_vi >= 0) {
    return previous_vi;
  }

  size_t vi = _viewers.size();
  _viewers.push_back(Viewer());
  Viewer &viewer = _viewers[vi];

  viewer._dr = dr;

  // Get the current camera off of the DisplayRegion, if any.
  viewer._viewer = dr->get_camera();
  if (viewer._viewer.is_empty()) {
    viewer._viewer_node = (LensNode *)NULL;
  } else {
    viewer._viewer_node = DCAST(LensNode, viewer._viewer.node());
  }

  // The internal camera is an identity-matrix camera that simply
  // views the meshes that represent the user's specified camera.
  viewer._internal_camera = new Camera("internal_camera");
  viewer._internal_camera->set_lens(new MatrixLens);
  viewer._internal_scene = NodePath("internal_screens");
  viewer._internal_camera->set_scene(viewer._internal_scene);

  NodePath camera_np = viewer._internal_scene.attach_new_node(viewer._internal_camera);
  viewer._dr->set_camera(camera_np);

  // Enable face culling on the wireframe mesh.  This will help us to
  // cull out invalid polygons that result from vertices crossing a
  // singularity (for instance, at the back of a fisheye lens).
  viewer._internal_scene.set_two_sided(0);

  // Finally, slot a new mesh for each screen.
  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    Screen &screen = (*si);
    screen._meshes.push_back(Mesh());
    nassertr(screen._meshes.size() == _viewers.size(), -1);
  }

  _stale = true;

  if (_dark_room.is_empty()) {
    _dark_room = viewer._viewer.get_top();
  } else {
    nassertr(_dark_room.is_same_graph(viewer._viewer), vi);
  }

  return vi;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::find_viewer
//       Access: Published
//  Description: Returns the index number of the indicated
//               DisplayRegion within the list of viewers, or -1 if it
//               is not found.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
find_viewer(DisplayRegion *dr) const {
  for (size_t vi = 0; vi < _viewers.size(); vi++) {
    if (_viewers[vi]._dr == dr) {
      return vi;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::remove_viewer
//       Access: Published
//  Description: Removes the viewer with the indicated index number
//               from the imager.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
remove_viewer(int index) {
  nassertv_always(index >= 0 && index < (int)_viewers.size());
  Viewer &viewer = _viewers[index];
  viewer._internal_camera->set_scene(NodePath());
  viewer._dr->set_camera(viewer._viewer);

  // Also remove the corresponding mesh from each screen.
  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    Screen &screen = (*si);
    nassertv(index < (int)screen._meshes.size());
    screen._meshes[index]._mesh.remove_node();
    screen._meshes.erase(screen._meshes.begin() + index);
  }

  _viewers.erase(_viewers.begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::remove_all_viewers
//       Access: Published
//  Description: Removes all viewers from the imager.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
remove_all_viewers() {
  while (!_viewers.empty()) {
    remove_viewer(_viewers.size() - 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_viewer_camera
//       Access: Published
//  Description: Specifies the LensNode that is to serve as the
//               viewer for this screen.  The relative position of
//               the LensNode to the NonlinearImager, as well as the
//               properties of the lens associated with the LensNode,
//               determines the UV's that will be assigned to the
//               geometry within the NonlinearImager.
//
//               It is not necessary to call this except to change the
//               camera after a viewer has been added, since the
//               default is to use whatever camera is associated with
//               the DisplayRegion at the time the viewer is added.
//
//               The NodePath must refer to a LensNode (or a Camera).
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_viewer_camera(int index, const NodePath &viewer_camera) {
  nassertv(index >= 0 && index < (int)_viewers.size());
  nassertv(!viewer_camera.is_empty() && 
           viewer_camera.node()->is_of_type(LensNode::get_class_type()));
  Viewer &viewer = _viewers[index];
  viewer._viewer = viewer_camera;
  viewer._viewer_node = DCAST(LensNode, viewer_camera.node());
  _stale = true;

  if (_dark_room.is_empty()) {
    _dark_room = viewer._viewer.get_top();
  } else {
    nassertv(_dark_room.is_same_graph(viewer._viewer));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_viewer_camera
//       Access: Published
//  Description: Returns the NodePath to the LensNode that is to serve
//               as nth viewer for this screen.
////////////////////////////////////////////////////////////////////
NodePath NonlinearImager::
get_viewer_camera(int index) const {
  nassertr(index >= 0 && index < (int)_viewers.size(), NodePath());
  return _viewers[index]._viewer;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_viewer_scene
//       Access: Published
//  Description: Returns a pointer to the root node of the internal
//               scene graph for the nth viewer, which is used to
//               render all of the screen meshes for this viewer.
//
//               This is the scene graph in which the screen meshes
//               within the dark room have been flattened into the
//               appropriate transformation according to the viewer's
//               lens properties (and position relative to the
//               screens).  It is this scene graph that is finally
//               rendered to the window.
////////////////////////////////////////////////////////////////////
NodePath NonlinearImager::
get_viewer_scene(int index) const {
  nassertr(index >= 0 && index < (int)_viewers.size(), NodePath());
  return _viewers[index]._internal_scene;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_num_viewers
//       Access: Published
//  Description: Returns the number of viewers that have been added to
//               the imager.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
get_num_viewers() const {
  return _viewers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_viewer
//       Access: Published
//  Description: Returns the nth viewer's DisplayRegion that has been
//               added to the imager.
////////////////////////////////////////////////////////////////////
DisplayRegion *NonlinearImager::
get_viewer(int index) const {
  nassertr(index >= 0 && index < (int)_viewers.size(), (DisplayRegion *)NULL);
  return _viewers[index]._dr;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_dark_room
//       Access: Published
//  Description: Returns the NodePath to the root of the dark room
//               scene.  This is the scene in which all of the
//               ProjectionScreens and the viewer cameras reside.
//               It's a standalone scene with a few projection screens
//               arranged artfully around one or more viewers; it's so
//               named because it's a little virtual theater.
//
//               Normally this scene is not rendered directly; it only
//               exists as an abstract concept, and to define the
//               relation between the ProjectionScreens and the
//               viewers.  But it may be rendered to help visualize
//               the NonlinearImager's behavior.
////////////////////////////////////////////////////////////////////
NodePath NonlinearImager::
get_dark_room() const {
  return _dark_room;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_graphics_engine
//       Access: Published
//  Description: Returns the GraphicsEngine that all of the viewers
//               added to the NonlinearImager have in common.
////////////////////////////////////////////////////////////////////
GraphicsEngine *NonlinearImager::
get_graphics_engine() const {
  return _engine;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::recompute
//       Access: Published
//  Description: Forces a regeneration of all the mesh objects, etc.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
recompute() {
  size_t vi;
  for (vi = 0; vi < _viewers.size(); ++vi) {
    Viewer &viewer = _viewers[vi];

    Screens::iterator si;
    for (si = _screens.begin(); si != _screens.end(); ++si) {
      Screen &screen = (*si);
      if (screen._active) {
        recompute_screen(screen, vi);
      }
    }

    if (viewer._viewer_node != (LensNode *)NULL && 
        viewer._viewer_node->get_lens() != (Lens *)NULL) {
      viewer._viewer_lens_change = 
        viewer._viewer_node->get_lens()->get_last_change();
    }
  }

  _stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::recompute_callback
//       Access: Private, Static
//  Description: This function is added as a task, to ensure that all
//               frames are up-to-date.
////////////////////////////////////////////////////////////////////
AsyncTask::DoneStatus NonlinearImager::
recompute_callback(GenericAsyncTask *, void *data) {
  NonlinearImager *self = (NonlinearImager *)data;
  self->recompute_if_stale();
  return AsyncTask::DS_cont;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::recompute_if_stale
//       Access: Private
//  Description: Calls recompute() if it needs to be called.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
recompute_if_stale() {
  if (_stale) {
    recompute();
  } else {
    size_t vi;
    for (vi = 0; vi < _viewers.size(); ++vi) {
      Viewer &viewer = _viewers[vi];
      if (viewer._viewer_node != (LensNode *)NULL) {
        UpdateSeq lens_change = 
          viewer._viewer_node->get_lens()->get_last_change();
        if (lens_change != viewer._viewer_lens_change) {
          // The viewer has changed, so we need to recompute all screens
          // on this viewer.
          Screens::iterator si;
          for (si = _screens.begin(); si != _screens.end(); ++si) {
            Screen &screen = (*si);
            if (screen._active) {
              recompute_screen(screen, vi);
            }
          }
          
        } else {
          // We may not need to recompute all screens, but maybe some of
          // them.
          Screens::iterator si;
          for (si = _screens.begin(); si != _screens.end(); ++si) {
            Screen &screen = (*si);
            if (screen._active && 
                screen._meshes[vi]._last_screen != screen._screen_node->get_last_screen()) {
              recompute_screen(screen, vi);
            } else {
              screen._screen_node->recompute_if_stale(screen._screen);
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::recompute_screen
//       Access: Private
//  Description: Regenerates the mesh objects just for the indicated
//               screen.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
recompute_screen(NonlinearImager::Screen &screen, size_t vi) {
  nassertv(vi < screen._meshes.size());
  screen._meshes[vi]._mesh.remove_node();
  if (!screen._active) {
    return;
  }

  screen._screen_node->recompute_if_stale(screen._screen);

  Viewer &viewer = _viewers[vi];
  PT(PandaNode) mesh = 
    screen._screen_node->make_flat_mesh(screen._screen, viewer._viewer);
  if (mesh != (PandaNode *)NULL) {
    screen._meshes[vi]._mesh = viewer._internal_scene.attach_new_node(mesh);
  }

  if (screen._buffer == (GraphicsOutput *)NULL) {
    GraphicsOutput *win = viewer._dr->get_window();
    GraphicsOutput *buffer = win->make_texture_buffer
      (screen._name, screen._tex_width, screen._tex_height, NULL, false);

    if (buffer != (GraphicsOutput *)NULL) {
      screen._buffer = buffer;
      DisplayRegion *dr = buffer->make_display_region();
      dr->set_camera(screen._source_camera);

    } else {
      screen._meshes[vi]._mesh.clear_texture();
    }
  }

  if (screen._buffer != (GraphicsOutput *)NULL) {
    screen._meshes[vi]._mesh.set_texture(screen._buffer->get_texture());

    // We don't really need to set the texture on the dark room
    // screen, since that's normally not rendered, but we do anyway
    // just for debugging purposes (in case the user does try to
    // render it, to see what's going on).
    screen._screen.set_texture(screen._buffer->get_texture());
  }

  screen._meshes[vi]._last_screen = screen._screen_node->get_last_screen();
}
