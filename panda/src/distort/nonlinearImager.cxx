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
#include "graphicsWindow.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::Constructor
//       Access: Published
//  Description: The NonlinearImager is associated with a particular
//               DisplayRegion when it is created.  It will throw away
//               whatever camera is currently associated with the
//               DisplayRegion, and create a speciality camera for
//               itself.
////////////////////////////////////////////////////////////////////
NonlinearImager::
NonlinearImager(DisplayRegion *dr) {
  _dr = dr;

  _internal_camera = new Camera("NonlinearImager");
  _internal_camera->set_lens(new MatrixLens);
  _internal_scene_node = new PandaNode("screens");
  _internal_scene = NodePath(_internal_scene_node);
  _internal_camera->set_scene(_internal_scene);

  NodePath camera_np = _internal_scene.attach_new_node(_internal_camera);
  _dr->set_camera(camera_np);

  // Enable face culling on the wireframe mesh.  This will help us to
  // cull out invalid polygons that result from vertices crossing a
  // singularity (for instance, at the back of a fisheye lens).
  _internal_scene.set_two_sided(0);

  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
NonlinearImager::
~NonlinearImager() {
  _internal_camera->set_scene(NodePath());
  _dr->set_camera(NodePath());
  remove_all_screens();
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
//               screen.  See set_size().
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
  new_screen._last_screen = screen->get_last_screen();
  new_screen._active = true;

  // If the LensNode associated with the ProjectionScreen is an actual
  // Camera, then it has a scene associated.  Otherwise, the user will
  // have to specify the scene later.
  LensNode *projector = screen->get_projector();
  if (projector->is_of_type(Camera::get_class_type())) {
    Camera *camera = DCAST(Camera, projector);
    new_screen._scene = camera->get_scene();
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
  screen._mesh.remove_node();
  _screens.erase(_screens.begin() + index);
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::remove_all_screens
//       Access: Published
//  Description: Removes all screens from the imager.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
remove_all_screens() {
  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    Screen &screen = (*si);
    screen._mesh.remove_node();
  }

  _screens.clear();
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
//     Function: NonlinearImager::set_size
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
set_size(int index, int width, int height) {
  nassertv(index >= 0 && index < (int)_screens.size());
  _screens[index]._tex_width = width;
  _screens[index]._tex_height = height;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_source
//       Access: Published
//  Description: Specifies the camera and root of the scene that will
//               be used to render the image for this particular
//               screen.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_source(int index, LensNode *source, const NodePath &scene) {
  nassertv(index >= 0 && index < (int)_screens.size());
  _screens[index]._source = source;
  _screens[index]._scene = scene;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_source
//       Access: Published
//  Description: Specifies the camera and root of the scene that will
//               be used to render the image for this particular
//               screen.
//
//               Since this flavor accepts a Camera node, instead of
//               just a LensNode, the scene is specified within the
//               Camera itself.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_source(int index, Camera *source) {
  nassertv(index >= 0 && index < (int)_screens.size());
  _screens[index]._source = source;
  _screens[index]._scene = source->get_scene();
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::set_active
//       Access: Published
//  Description: Sets the active flag on the indicated screen.  If the
//               active flag is true, the screen will be used;
//               otherwise, it will not appear.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
set_active(int index, bool active) {
  nassertv(index >= 0 && index < (int)_screens.size());
  _screens[index]._active = active;

  if (!active) {
    Screen &screen = _screens[index];
    // If we've just made this screen inactive, remove its mesh.
    screen._mesh.remove_node();
    screen._texture.clear();
  } else {
    // If we've just made it active, it needs to be recomputed.
    _stale = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::get_active
//       Access: Published
//  Description: Returns the active flag on the indicated screen.
////////////////////////////////////////////////////////////////////
bool NonlinearImager::
get_active(int index) const {
  nassertr(index >= 0 && index < (int)_screens.size(), false);
  return _screens[index]._active;
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::recompute
//       Access: Published
//  Description: Forces a regeneration of all the mesh objects, etc.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
recompute() {
  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    if ((*si)._active) {
      recompute_screen(*si);
    }
  }

  if (_camera != (LensNode *)NULL && _camera->get_lens() != (Lens *)NULL) {
    _camera_lens_change = _camera->get_lens()->get_last_change();
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
render() {
  recompute_if_stale();

  Screens::iterator si;
  for (si = _screens.begin(); si != _screens.end(); ++si) {
    if ((*si)._active) {
      render_screen(*si);
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
  if (_camera != (LensNode *)NULL && 
      _camera->get_lens() != (Lens *)NULL) {
    UpdateSeq lens_change = _camera->get_lens()->get_last_change();
    if (_stale || lens_change != _camera_lens_change) {
      recompute();
    } else {
      // We're not overall stale, but maybe we need to recompute one
      // or more of our screens.
      Screens::iterator si;
      for (si = _screens.begin(); si != _screens.end(); ++si) {
        Screen &screen = (*si);
        if (screen._active && 
            screen._last_screen != screen._screen->get_last_screen()) {
          recompute_screen(screen);
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
recompute_screen(NonlinearImager::Screen &screen) {
  screen._mesh.remove_node();
  screen._texture.clear();
  if (_camera == (LensNode *)NULL || !screen._active) {
    // Not much we can do without a camera.
    return;
  }

  PT(PandaNode) mesh = screen._screen->make_flat_mesh(_camera);
  screen._mesh = _internal_scene.attach_new_node(mesh);

  PT(Texture) texture = new Texture;
  texture->set_minfilter(Texture::FT_linear);
  texture->set_magfilter(Texture::FT_linear);
  texture->set_wrapu(Texture::WM_clamp);
  texture->set_wrapv(Texture::WM_clamp);
  texture->_pbuffer->set_xsize(screen._tex_width);
  texture->_pbuffer->set_ysize(screen._tex_height);

  screen._texture = texture;
  screen._mesh.set_texture(texture);
  screen._last_screen = screen._screen->get_last_screen();
}

////////////////////////////////////////////////////////////////////
//     Function: NonlinearImager::render_screen
//       Access: Private
//  Description: Renders the scene just for the indicated screen, into
//               the screen's own texture.
////////////////////////////////////////////////////////////////////
void NonlinearImager::
render_screen(NonlinearImager::Screen &screen) {
  if (screen._source == (LensNode *)NULL) {
    distort_cat.error()
      << "No source lens specified for screen " << screen._screen->get_name()
      << "\n";
    return;
  }

  if (screen._scene.is_empty()) {
    distort_cat.error()
      << "No scene specified for screen " << screen._screen->get_name()
      << "\n";
    return;
  }

  // Got to update this to new scene graph.

  /*
  GraphicsStateGuardian *gsg = _dr->get_window()->get_gsg();

  // Make a display region of the proper size and clear it to prepare for
  // rendering the scene.
  PT(DisplayRegion) scratch_region =
    gsg->get_window()->make_scratch_display_region(screen._tex_width, screen._tex_height);
  gsg->clear(gsg->get_render_buffer(RenderBuffer::T_back |
                                    RenderBuffer::T_depth), 
             scratch_region);

  DisplayRegionStack old_dr = gsg->push_display_region(scratch_region);
  gsg->prepare_display_region();
  gsg->render_scene(screen._scene, screen._source);

  // Copy the results of the render from the frame buffer into the
  // screen's texture.
  screen._texture->copy(gsg, scratch_region, 
                        gsg->get_render_buffer(RenderBuffer::T_back));
  
  // Restore the original display region.
  gsg->pop_display_region(old_dr);
  */
}
