// Filename: nonlinearImager.h
// Created by:  drose (12Dec01)
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

#ifndef NONLINEARIMAGER_H
#define NONLINEARIMAGER_H

#include "pandabase.h"

#include "projectionScreen.h"
#include "displayRegion.h"
#include "graphicsOutput.h"
#include "camera.h"
#include "texture.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "pointerTo.h"
#include "pvector.h"

class GraphicsEngine;
class GraphicsStateGuardian;
class GraphicsOutput;

////////////////////////////////////////////////////////////////////
//       Class : NonlinearImager
// Description : This class object combines the rendered output of a
//               3-d from one or more linear (e.g. perspective)
//               cameras, as seen through a single, possibly nonlinear
//               camera.
//
//               This can be used to generate real-time imagery of a
//               3-d scene using a nonlinear camera, for instance a
//               fisheye camera, even though the underlying graphics
//               engine may only support linear cameras.  It can also
//               pre-distort imagery to compensate for off-axis
//               projectors, and/or curved screens of any complexity.
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
//               There also exist in the room one or more (possibly
//               nonlinear) cameras, called viewers, that observe
//               these screens.  The image of the projection screens
//               seen by each viewer is finally displayed on the
//               viewer's associated DisplayRegion.  By placing the
//               viewer(s) appropriately relative to the screens, and
//               by choosing suitable lens properties for the
//               viewer(s), you can achieve a wide variety of
//               distortion effects.
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
//               Finally, the cameras that view the whole
//               configuration of screens are called viewers; each of
//               these is associated with a DisplayRegion, and they
//               are set via set_viewer_camera().
//
//               Of all these lenses, only the source cameras must use
//               linear (that is, perspective or orthographic) lenses.
//               The projectors and viewers may be any arbitrary lens,
//               linear or otherwise.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX NonlinearImager {
PUBLISHED:
  NonlinearImager();
  ~NonlinearImager();

  int add_screen(ProjectionScreen *screen, const string &name = string());
  int find_screen(ProjectionScreen *screen) const;
  void remove_screen(int index);
  void remove_all_screens();

  int get_num_screens() const;
  ProjectionScreen *get_screen(int index) const;
  GraphicsOutput *get_buffer(int index) const;
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

public:
  static void recompute_callback(void *data);
  void recompute_if_stale();

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
    string _name;
    PT(GraphicsOutput) _buffer;
    NodePath _source_camera;
    int _tex_width, _tex_height;
    bool _active;

    // One mesh per viewer.
    Meshes _meshes;
  };
  typedef pvector<Screen> Screens;

  void recompute_screen(Screen &screen, size_t vi);
  void render_screen(GraphicsEngine *engine, Screen &screen);

  Viewers _viewers;
  Screens _screens;

  GraphicsEngine *_engine;

  bool _stale;
};

#include "nonlinearImager.I"

#endif
