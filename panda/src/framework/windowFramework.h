// Filename: windowFramework.h
// Created by:  drose (02Apr02)
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

#ifndef WINDOWFRAMEWORK_H
#define WINDOWFRAMEWORK_H

#include "pandabase.h"
#include "nodePath.h"
#include "camera.h"
#include "graphicsWindow.h"
#include "animControlCollection.h"
#include "trackball.h"
#include "filename.h"
#include "frameRateMeter.h"
#include "pointerTo.h"
#include "partGroup.h"
#include "pvector.h"
#include "typedWritableReferenceCount.h"
#include "graphicsWindow.h"
#include "loaderOptions.h"
#include "pgSliderBar.h"
#include "textNode.h"

class PandaFramework;
class AmbientLight;
class DirectionalLight;
class GraphicsEngine;
class GraphicsPipe;
class DisplayRegion;

////////////////////////////////////////////////////////////////////
//       Class : WindowFramework
// Description : This encapsulates the data that is normally
//               associated with a single window, or with a single
//               display region within a window.  (In the case where a
//               window has been subdivided with split_window(), there
//               may be multiple WindowFrameworks objects that share
//               the same GraphicsWindow pointer, but reference
//               different display regions within that window).
////////////////////////////////////////////////////////////////////
class EXPCL_FRAMEWORK WindowFramework : public TypedWritableReferenceCount {
protected:
  WindowFramework(PandaFramework *panda_framework);
  WindowFramework(const WindowFramework &copy, DisplayRegion *display_region);

public:
  virtual ~WindowFramework();

protected:
  GraphicsWindow *open_window(const WindowProperties &props,
                              GraphicsEngine *engine, GraphicsPipe *pipe,
                              GraphicsStateGuardian *gsg = NULL);
  void close_window();

public:
  INLINE PandaFramework *get_panda_framework() const;
  INLINE GraphicsWindow *get_graphics_window() const;
  INLINE DisplayRegion *get_display_region() const;
  const NodePath &get_camera_group();

  INLINE int get_num_cameras() const;
  INLINE Camera *get_camera(int n) const;

  const NodePath &get_render();
  const NodePath &get_render_2d();
  const NodePath &get_aspect_2d();
  const NodePath &get_mouse();

  void enable_keyboard();
  void setup_trackball();
  void center_trackball(const NodePath &object);

  bool load_models(const NodePath &parent,
                   int argc, char *argv[], int first_arg = 1);
  bool load_models(const NodePath &parent,
                   const pvector<Filename> &files);
  NodePath load_model(const NodePath &parent, Filename filename);
  NodePath load_default_model(const NodePath &parent);
  void loop_animations(int hierarchy_match_flags = 
                       PartGroup::HMF_ok_part_extra |
                       PartGroup::HMF_ok_anim_extra);
  void next_anim_control();
  void set_anim_controls(bool enable);
  INLINE bool get_anim_controls() const;

  enum BackgroundType {
    BT_other = 0,
    BT_default,
    BT_black,
    BT_gray,
    BT_white,
    BT_none
  };

  enum SplitType {
    ST_default,
    ST_horizontal,
    ST_vertical,
  };
  WindowFramework *split_window(SplitType split_type = ST_default);

  void set_wireframe(bool enable);
  void set_texture(bool enable);
  void set_two_sided(bool enable);
  void set_one_sided_reverse(bool enable);
  void set_lighting(bool enable);
  void set_background_type(BackgroundType type);

  INLINE bool get_wireframe() const;
  INLINE bool get_texture() const;
  INLINE bool get_two_sided() const;
  INLINE bool get_one_sided_reverse() const;
  INLINE bool get_lighting() const;
  INLINE BackgroundType get_background_type() const;
  
protected:
  PT(Camera) make_camera();
  void setup_lights();

private:
  PT(PandaNode) load_image_as_model(const Filename &filename);
  void create_anim_controls();
  void destroy_anim_controls();
  void update_anim_controls();
  static void st_update_anim_controls(CPT_Event, void *data);

private:
  PandaFramework *_panda_framework;
  PT(GraphicsWindow) _window;
  PT(DisplayRegion) _display_region_2d;
  PT(DisplayRegion) _display_region_3d;

  NodePath _camera_group;
  typedef pvector< PT(Camera) > Cameras;
  Cameras _cameras;

  NodePath _render;
  NodePath _render_2d;
  NodePath _aspect_2d;

  AnimControlCollection _anim_controls;
  bool _anim_controls_enabled;
  int _anim_index;
  NodePath _anim_controls_group;
  PT(PGSliderBar) _anim_slider;
  PT(TextNode) _frame_number;

  NodePath _mouse;
  PT(Trackball) _trackball;

  NodePath _alight;
  NodePath _dlight;
  
  bool _got_keyboard;
  bool _got_trackball;
  bool _got_lights;

  bool _wireframe_enabled;
  bool _texture_enabled;
  bool _two_sided_enabled;
  bool _one_sided_reverse_enabled;
  bool _lighting_enabled;

  PT(FrameRateMeter) _frame_rate_meter;

  BackgroundType _background_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "WindowFramework",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PandaFramework;
};

#include "windowFramework.I"

#endif
