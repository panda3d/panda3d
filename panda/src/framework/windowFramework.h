// Filename: windowFramework.h
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

#ifndef WINDOWFRAMEWORK_H
#define WINDOWFRAMEWORK_H

#include "pandabase.h"
#include "qpnodePath.h"
#include "qpcamera.h"
#include "graphicsWindow.h"
#include "animControlCollection.h"
#include "filename.h"
#include "pointerTo.h"
#include "pvector.h"

class PandaFramework;
class AmbientLight;
class DirectionalLight;

////////////////////////////////////////////////////////////////////
//       Class : WindowFramework
// Description : This encapsulates the data that is normally
//               associated with a single window that we've opened.
////////////////////////////////////////////////////////////////////
class EXPCL_FRAMEWORK WindowFramework {
protected:
  WindowFramework(PandaFramework *panda_framework);
  virtual ~WindowFramework();

  GraphicsWindow *open_window(const GraphicsWindow::Properties &props,
                              GraphicsPipe *pipe);
  void close_window();

public:
  INLINE PandaFramework *get_panda_framework() const;
  INLINE GraphicsWindow *get_graphics_window() const;
  const qpNodePath &get_camera_group();

  INLINE int get_num_cameras() const;
  INLINE qpCamera *get_camera(int n) const;

  const qpNodePath &get_render();
  const qpNodePath &get_render_2d();
  const qpNodePath &get_mouse();

  void enable_keyboard();
  void setup_trackball();

  bool load_models(const qpNodePath &parent,
                   int argc, char *argv[], int first_arg = 1);
  bool load_models(const qpNodePath &parent,
                   const pvector<Filename> &files);
  qpNodePath load_model(const qpNodePath &parent, Filename filename);
  qpNodePath load_default_model(const qpNodePath &parent);
  void loop_animations();

  void set_wireframe(bool enable);
  void set_texture(bool enable);
  void set_two_sided(bool enable);
  void set_lighting(bool enable);

  INLINE bool get_wireframe() const;
  INLINE bool get_texture() const;
  INLINE bool get_two_sided() const;
  INLINE bool get_lighting() const;

protected:
  PT(qpCamera) make_camera();
  void setup_lights();

private:
  PandaFramework *_panda_framework;
  PT(GraphicsWindow) _window;

  qpNodePath _camera_group;
  typedef pvector< PT(qpCamera) > Cameras;
  Cameras _cameras;

  qpNodePath _render;
  qpNodePath _render_2d;
  AnimControlCollection _anim_controls;

  qpNodePath _mouse;

  AmbientLight *_alight;
  DirectionalLight *_dlight;
  
  bool _got_keyboard;
  bool _got_trackball;
  bool _got_lights;

  bool _wireframe_enabled;
  bool _texture_enabled;
  bool _two_sided_enabled;
  bool _lighting_enabled;

  friend class PandaFramework;
};

#include "windowFramework.I"

#endif
