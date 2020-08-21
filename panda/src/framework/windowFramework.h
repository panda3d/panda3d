/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowFramework.h
 * @author drose
 * @date 2002-04-02
 */

#ifndef WINDOWFRAMEWORK_H
#define WINDOWFRAMEWORK_H

#include "pandabase.h"
#include "nodePath.h"
#include "camera.h"
#include "graphicsOutput.h"
#include "graphicsWindow.h"
#include "animControlCollection.h"
#include "trackball.h"
#include "filename.h"
#include "frameRateMeter.h"
#include "sceneGraphAnalyzerMeter.h"
#include "pointerTo.h"
#include "partGroup.h"
#include "pvector.h"
#include "typedWritableReferenceCount.h"
#include "loaderOptions.h"
#include "pgSliderBar.h"
#include "textNode.h"
#include "eventHandler.h"
#include "genericAsyncTask.h"

class PandaFramework;
class AmbientLight;
class DirectionalLight;
class GraphicsEngine;
class GraphicsPipe;
class DisplayRegion;

/**
 * This encapsulates the data that is normally associated with a single
 * window, or with a single display region within a window.  (In the case
 * where a window has been subdivided with split_window(), there may be
 * multiple WindowFrameworks objects that share the same GraphicsOutput
 * pointer, but reference different display regions within that window).
 */
class EXPCL_FRAMEWORK WindowFramework : public TypedWritableReferenceCount {
protected:
  WindowFramework(PandaFramework *panda_framework);
  WindowFramework(const WindowFramework &copy, DisplayRegion *display_region);

public:
  virtual ~WindowFramework();

protected:
  GraphicsOutput *open_window(const WindowProperties &props, int flags,
                              GraphicsEngine *engine, GraphicsPipe *pipe,
                              GraphicsStateGuardian *gsg = nullptr,
                              const FrameBufferProperties &fbprops =
                                    FrameBufferProperties::get_default());
  void close_window();

public:
  INLINE PandaFramework *get_panda_framework() const;
  INLINE GraphicsWindow *get_graphics_window() const;
  INLINE GraphicsOutput *get_graphics_output() const;
  NodePath get_camera_group();

  INLINE int get_num_cameras() const;
  INLINE Camera *get_camera(int n) const;

  INLINE DisplayRegion *get_display_region_2d() const;
  INLINE DisplayRegion *get_display_region_3d() const;

  NodePath get_render();
  NodePath get_render_2d();
  NodePath get_aspect_2d();
  NodePath get_pixel_2d();
  NodePath get_mouse();
  NodePath get_button_thrower();

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
  void stagger_animations();
  void next_anim_control();
  void set_anim_controls(bool enable);
  INLINE bool get_anim_controls() const;
  void adjust_dimensions();

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

  void set_wireframe(bool enable, bool filled=false);
  void set_texture(bool enable);
  void set_two_sided(bool enable);
  void set_one_sided_reverse(bool enable);
  void set_lighting(bool enable);
  void set_perpixel(bool enable);
  void set_background_type(BackgroundType type);

  INLINE bool get_wireframe() const;
  INLINE bool get_wireframe_filled() const;
  INLINE bool get_texture() const;
  INLINE bool get_two_sided() const;
  INLINE bool get_one_sided_reverse() const;
  INLINE bool get_lighting() const;
  INLINE bool get_perpixel() const;
  INLINE BackgroundType get_background_type() const;

  static TextFont *get_shuttle_controls_font();
  NodePath make_camera();

protected:
  void setup_lights();

private:
  PT(PandaNode) load_image_as_model(const Filename &filename);
  void create_anim_controls();
  void destroy_anim_controls();
  void update_anim_controls();

  void setup_shuttle_button(const std::string &label, int index,
                            EventHandler::EventCallbackFunction *func);
  void back_button();
  void pause_button();
  void play_button();
  void forward_button();

  static AsyncTask::DoneStatus st_update_anim_controls(GenericAsyncTask *task, void *data);

  static void st_back_button(const Event *, void *data);
  static void st_pause_button(const Event *, void *data);
  static void st_play_button(const Event *, void *data);
  static void st_forward_button(const Event *, void *data);

private:
  PandaFramework *_panda_framework;
  PT(GraphicsOutput) _window;
  PT(DisplayRegion) _display_region_2d;
  PT(DisplayRegion) _display_region_3d;

  NodePath _camera_group;
  typedef pvector< PT(Camera) > Cameras;
  Cameras _cameras;

  NodePath _render;
  NodePath _render_2d;
  NodePath _aspect_2d;
  NodePath _pixel_2d;

  AnimControlCollection _anim_controls;
  bool _anim_controls_enabled;
  int _anim_index;
  NodePath _anim_controls_group;
  PT(PGSliderBar) _anim_slider;
  PT(PGSliderBar) _play_rate_slider;
  PT(TextNode) _frame_number;
  PT(GenericAsyncTask) _update_anim_controls_task;

  NodePath _mouse;
  NodePath _button_thrower;
  PT(Trackball) _trackball;

  NodePath _alight;
  NodePath _dlight;

  bool _got_keyboard;
  bool _got_trackball;
  bool _got_lights;

  bool _wireframe_enabled;
  bool _wireframe_filled;
  bool _texture_enabled;
  bool _two_sided_enabled;
  bool _one_sided_reverse_enabled;
  bool _lighting_enabled;
  bool _perpixel_enabled;

  PT(FrameRateMeter) _frame_rate_meter;
  PT(SceneGraphAnalyzerMeter) _scene_graph_analyzer_meter;

  BackgroundType _background_type;

  static PT(TextFont) _shuttle_controls_font;

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
