// Filename: nonlinearImager.h
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

#ifndef NONLINEARIMAGER_H
#define NONLINEARIMAGER_H

#include "pandabase.h"

#include "projectionScreen.h"
#include "displayRegion.h"
#include "camera.h"
#include "texture.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "pointerTo.h"
#include "pvector.h"

class GraphicsEngine;
class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : NonlinearImager
// Description : This class object combines the rendered output of a
//               3-d from one or more linear (e.g. perspective)
//               cameras, as seen through a single, possibly nonlinear
//               camera.
//
//               This can be used to generate real-time imagery of a
//               3-d scene using a nonlinear camera, for instance a
//               fisheye camera, even though the 3-d graphics engine
//               only supports linear cameras.
//
//               
//               A NonlinearImager may be visualized as a dark room
//               into which a number of projection screens have been
//               placed, of arbitrary size and shape and at any
//               arbitrary position and orientation to each other.
//               Onto each of these screens is projected the view as
//               seen by a normal perspective camera that exists in
//               the world (that is, under render).
//
//               There also exists in the theater one or more
//               (possibly nonlinear) cameras, called viewers, that
//               observe these screens.  Each of these viewers is
//               associated with a single DisplayRegion, where the
//               final results are presented.
//
//
//               There are several different LensNode (Camera) objects
//               involved at each stage in the process.  To help keep
//               them all straight, different words are used to refer
//               to each different kind of Camera used within this
//               object.  The camera(s) under render, that capture the
//               original view of the world to be projected onto the
//               screens, are called source cameras, and are set per
//               screen via set_source_camera().  The LensNode that is
//               associated with each screen to project the image as
//               seen from the screen's source camera is called a
//               projector; these are set via the
//               ProjectionScreen::set_projector() interface.
//               Finally, the (possibly nonlinear) cameras that view
//               the whole configuration of screens are called
//               viewers; each of these is associated with a
//               DisplayRegion, and they are set via set_viewer_camera().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX NonlinearImager {
PUBLISHED:
  NonlinearImager();
  ~NonlinearImager();

  int add_screen(ProjectionScreen *screen);
  int find_screen(ProjectionScreen *screen) const;
  void remove_screen(int index);
  void remove_all_screens();

  int get_num_screens() const;
  ProjectionScreen *get_screen(int index) const;
  void set_texture_size(int index, int width, int height);
  void set_source_camera(int index, const NodePath &source_camera);

  void set_screen_active(int index, bool active);
  bool get_screen_active(int index) const;

  int add_viewer(DisplayRegion *dr);
  int find_viewer(DisplayRegion *dr) const;
  void remove_viewer(int index);
  void remove_all_viewers();

  void set_viewer_camera(int index, const NodePath &viewer_camera);
  NodePath get_viewer_camera(int index) const;

  NodePath get_internal_scene(int index) const;

  int get_num_viewers() const;
  DisplayRegion *get_viewer(int index) const;

  void recompute();
  void render(GraphicsEngine *engine);

private:
  class Viewer {
  public:
    PT(DisplayRegion) _dr;
    PT(Camera) _internal_camera;
    NodePath _internal_scene;
    NodePath _viewer;
    PT(LensNode) _viewer_node;
    UpdateSeq _viewer_lens_change;
  };
  typedef pvector<Viewer> Viewers;

  class Mesh {
  public:
    NodePath _mesh;
    UpdateSeq _last_screen;
  };
  typedef pvector<Mesh> Meshes;

  class Screen {
  public:
    PT(ProjectionScreen) _screen;
    PT(Texture) _texture;
    NodePath _source_camera;
    int _tex_width, _tex_height;
    bool _active;

    // One mesh per viewer.
    Meshes _meshes;
  };
  typedef pvector<Screen> Screens;

  void recompute_if_stale();
  void recompute_screen(Screen &screen, size_t vi);
  void render_screen(GraphicsEngine *engine, Screen &screen);

  Viewers _viewers;
  Screens _screens;
  GraphicsStateGuardian *_gsg;

  bool _stale;
};

#include "nonlinearImager.I"

#endif
