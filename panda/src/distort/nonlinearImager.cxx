// Filename: nonlinearImager.cxx
// Created by:  drose (12Dec01)
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

#include "nonlinearImager.h"
#include "config_distort.h"

#include "graphicsStateGuardian.h"
#include "matrixLens.h"
#include "graphicsOutput.h"
#include "graphicsEngine.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
NonlinearImager::
NonlinearImager() {
  _gsg = (GraphicsStateGuardian *)NULL;
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
//               width and height indicate the size of the texture
//               that will be created to render the scene for the
//               screen.  See set_texture_size().
//
//               Each ProjectionScreen object should already have some
//               screen geometry created.
//
//               When render() is called, the graphics state guardian
//               will be used to render a scene for each
//               ProjectionScreen object, and then each resulting
//               image will be applied to a mesh to be rendered to the
//               screen.
//
//               The return value is the index number of the new
//               screen.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
add_screen(ProjectionScreen *screen) {
  _screens.push_back(Screen());
  Screen &new_screen = _screens.back();
  new_screen._screen = screen;
  new_screen._texture = (Texture *)NULL;
  new_screen._tex_width = 256;
  new_screen._tex_height = 256;
  new_screen._active = true;

  // Slot a mesh for each viewer.
  size_t vi;
  for (vi = 0; vi < _viewers.size(); ++vi) {
    new_screen._meshes.push_back(Mesh());
    new_screen._meshes[vi]._last_screen = screen->get_last_screen();
  }

  _stale = true;
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
find_screen(ProjectionScreen *screen) const {
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
ProjectionScreen *NonlinearImager::
get_screen(int index) const {
  nassertr(index >= 0 && index < (int)_screens.size(), (ProjectionScreen *)NULL);
  return _screens[index]._screen;
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
  _screens[index]._tex_width = width;
  _screens[index]._tex_height = height;
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
  _screens[index]._active = active;

  if (!active) {
    Screen &screen = _screens[index];
    // If we've just made this screen inactive, remove its meshes.
    for (size_t vi = 0; vi < screen._meshes.size(); vi++) {
      screen._meshes[vi]._mesh.remove_node();
    }
    screen._texture.clear();
  } else {
    // If we've just made it active, it needs to be recomputed.
    _stale = true;
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
//               All viewers must share the same
//               GraphicsStateGuardian.
//
//               The return value is the index of the new viewer.
////////////////////////////////////////////////////////////////////
int NonlinearImager::
add_viewer(DisplayRegion *dr) {
  GraphicsOutput *win = dr->get_window();
  GraphicsStateGuardian *gsg = win->get_gsg();
  nassertr(_viewers.empty() || (gsg == _gsg && win == _win), -1);
  _gsg = gsg;
  _win = win;

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
  viewer._internal_camera = new Camera("NonlinearImager");
  viewer._internal_camera->set_lens(new MatrixLens);
  viewer._internal_scene = NodePath("screens");
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
//     Function: NonlinearImager::get_internal_scene
//       Access: Published
//  Description: Returns a pointer to the root node of the internal
//               scene graph for the nth viewer, which is used to
//               render all of the screen meshes for this viewer.
////////////////////////////////////////////////////////////////////
NodePath NonlinearImager::
get_internal_scene(int index) const {
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
//     Function: NonlinearImager::recompute
//       Access: Published
//  Description: Forces a regeneration of all the mesh objects, etc.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
recompute() {
  // First, force all the textures to clear.
  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    Screen &screen = (*si);
    screen._texture.clear();
  }

  size_t vi;
  for (vi = 0; vi < _viewers.size(); ++vi) {
    Viewer &viewer = _viewers[vi];

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
//     Function: NonlinearImager::render
//       Access: Published
//  Description: Uses the DisplayRegion's GSG to render a scene for
//               each ProjectionScreen, and makes our DisplayRegion
//               ready to render the combined results.  This will
//               destroy the contents of the frame buffer; it should
//               be done before any of the actual frame has started
//               rendering.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
render(GraphicsEngine *engine) {
  recompute_if_stale();

  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    if ((*si)._active) {
      render_screen(engine, *si);
    }
  }
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
                screen._meshes[vi]._last_screen != screen._screen->get_last_screen()) {
              recompute_screen(screen, vi);
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

  Viewer &viewer = _viewers[vi];
  PT(PandaNode) mesh = screen._screen->make_flat_mesh(viewer._viewer);
  if (mesh != (PandaNode *)NULL) {
    screen._meshes[vi]._mesh = viewer._internal_scene.attach_new_node(mesh);
  }

  if (screen._texture == (Texture *)NULL ||
      screen._texture->_pbuffer == (PixelBuffer *)NULL ||
      screen._texture->_pbuffer->get_xsize() != screen._tex_width ||
      screen._texture->_pbuffer->get_ysize() != screen._tex_height) {
    PT(Texture) texture = new Texture;
    texture->set_minfilter(Texture::FT_linear);
    texture->set_magfilter(Texture::FT_linear);
    texture->set_wrapu(Texture::WM_clamp);
    texture->set_wrapv(Texture::WM_clamp);
    texture->_pbuffer->set_xsize(screen._tex_width);
    texture->_pbuffer->set_ysize(screen._tex_height);

    screen._texture = texture;
  }

  screen._meshes[vi]._mesh.set_texture(screen._texture);
  screen._meshes[vi]._last_screen = screen._screen->get_last_screen();
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::render_screen
//       Access: Private
//  Description: Renders the scene just for the indicated screen, into
//               the screen's own texture.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
render_screen(GraphicsEngine *engine, NonlinearImager::Screen &screen) {
  if (screen._source_camera.is_empty()) {
    distort_cat.error()
      << "No source camera specified for screen " << screen._screen->get_name()
      << "\n";
    return;
  }

  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  screen._screen->recompute_if_stale();

  // Make a display region of the proper size and clear it to prepare for
  // rendering the scene.
  PT(DisplayRegion) scratch_region =
    _win->make_scratch_display_region(screen._tex_width, screen._tex_height);
  scratch_region->set_camera(screen._source_camera);
  engine->render_subframe(_gsg, scratch_region, true);

  // Copy the results of the render from the frame buffer into the
  // screen's texture.
  screen._texture->copy(_gsg, scratch_region, 
                        _gsg->get_render_buffer(RenderBuffer::T_back));

  // It might be nice if we didn't throw away the scratch region every
  // time, which prevents us from preserving cull state from one frame
  // to the next.
}
