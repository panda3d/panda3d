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

////////////////////////////////////////////////////////////////////
//       Class : NonlinearImager
// Description : This class object combines the rendered output of a
//               3-d from one or more linear cameras, as seen through
//               a single, possibly non-linear camera.
//
//               This can be used to generate real-time imagery of a
//               3-d scene using a nonlinear camera, for instance a
//               fisheye camera, even though the 3-d graphics engine
//               only supports linear cameras.
//
//               The NonlinearImager collects together a number of
//               ProjectionScreens, each of which has a standard,
//               linear Camera.  Each frame, the Imager renders each
//               scene into a texture and then maps that texture onto
//               a mesh which is presented to the graphics engine for
//               rendering the final, non-linear output.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAFX NonlinearImager {
PUBLISHED:
  NonlinearImager(DisplayRegion *dr);
  ~NonlinearImager();

  int add_screen(ProjectionScreen *screen);
  int find_screen(ProjectionScreen *screen) const;
  void remove_screen(int index);
  void remove_all_screens();

  int get_num_screens() const;
  ProjectionScreen *get_screen(int index) const;
  void set_size(int index, int width, int height);
  void set_source(int index, LensNode *source, const NodePath &scene);
  void set_source(int index, Camera *source);

  void set_active(int index, bool active);
  bool get_active(int index) const;

  INLINE void set_camera(LensNode *camera);
  INLINE LensNode *get_camera() const;

  INLINE NodePath get_internal_scene() const;

  void recompute();
  void render();

private:
  class Screen {
  public:
    PT(ProjectionScreen) _screen;
    NodePath _mesh;
    PT(Texture) _texture;
    PT(LensNode) _source;
    NodePath _scene;
    int _tex_width, _tex_height;
    UpdateSeq _last_screen;
    bool _active;
  };

  void recompute_if_stale();
  void recompute_screen(Screen &screen);
  void render_screen(Screen &screen);

  PT(DisplayRegion) _dr;

  typedef pvector<Screen> Screens;
  Screens _screens;

  PT(LensNode) _camera;

  PT(Camera) _internal_camera;
  NodePath _internal_scene;
  PT(PandaNode) _internal_scene_node;

  bool _stale;
  UpdateSeq _camera_lens_change;
};

#include "nonlinearImager.I"

#endif
