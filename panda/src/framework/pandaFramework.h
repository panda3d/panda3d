/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaFramework.h
 * @author drose
 * @date 2002-04-02
 */

#ifndef PANDAFRAMEWORK_H
#define PANDAFRAMEWORK_H

#include "pandabase.h"
#include "config_framework.h"

#include "windowFramework.h"

#include "nodePath.h"
#include "eventHandler.h"
#include "graphicsPipe.h"
#include "graphicsEngine.h"
#include "graphicsWindow.h"
#include "recorderController.h"
#include "pointerTo.h"
#include "asyncTaskManager.h"
#include "genericAsyncTask.h"

#include "pvector.h"

/**
 * This class serves to provide a high-level framework for basic applications
 * that use Panda in simple ways (like opening a window to view models, etc.).
 */
class EXPCL_FRAMEWORK PandaFramework {
public:
  PandaFramework();
  virtual ~PandaFramework();

  void open_framework();
  void open_framework(int &argc, char **&argv);
  void close_framework();

  GraphicsPipe *get_default_pipe();
  INLINE GraphicsEngine *get_graphics_engine();
  INLINE const NodePath &get_data_root() const;
  INLINE EventHandler &get_event_handler();
  INLINE AsyncTaskManager &get_task_mgr();
  NodePath get_mouse(GraphicsOutput *window);
  void remove_mouse(const GraphicsOutput *window);

  void define_key(const std::string &event_name,
                  const std::string &description,
                  EventHandler::EventCallbackFunction *function,
                  void *data);

  INLINE void set_window_title(const std::string &title);
  virtual void get_default_window_props(WindowProperties &props);

  WindowFramework *open_window();
  WindowFramework *open_window(GraphicsPipe *pipe,
                               GraphicsStateGuardian *gsg = nullptr);
  WindowFramework *open_window(const WindowProperties &props, int flags,
                               GraphicsPipe *pipe = nullptr,
                               GraphicsStateGuardian *gsg = nullptr);

  INLINE int get_num_windows() const;
  INLINE WindowFramework *get_window(int n) const;
  int find_window(const GraphicsOutput *win) const;
  int find_window(const WindowFramework *wf) const;
  void close_window(int n);
  INLINE void close_window(WindowFramework *wf);
  void close_all_windows();
  bool all_windows_closed() const;

  NodePath &get_models();

  void report_frame_rate(std::ostream &out) const;
  void reset_frame_rate();

  void set_wireframe(bool enable);
  void set_texture(bool enable);
  void set_two_sided(bool enable);
  void set_lighting(bool enable);
  void set_perpixel(bool enable);
  void set_background_type(WindowFramework::BackgroundType type);

  INLINE bool get_wireframe() const;
  INLINE bool get_texture() const;
  INLINE bool get_two_sided() const;
  INLINE bool get_lighting() const;
  INLINE bool get_perpixel() const;
  INLINE WindowFramework::BackgroundType get_background_type() const;

  static int hide_collision_solids(NodePath node);
  static int show_collision_solids(NodePath node);

  void set_highlight(const NodePath &node);
  void clear_highlight();
  INLINE bool has_highlight() const;
  INLINE const NodePath &get_highlight() const;

  INLINE RecorderController *get_recorder() const;
  INLINE void set_recorder(RecorderController *recorder);

  void enable_default_keys();

  virtual bool do_frame(Thread *current_thread);
  void main_loop();

  INLINE void set_exit_flag();
  INLINE void clear_exit_flag();

public:
  static LoaderOptions _loader_options;

protected:
  virtual PT(WindowFramework) make_window_framework();
  virtual void make_default_pipe();
  virtual void do_enable_default_keys();
  bool clear_text();

public:
  static void event_esc(const Event *, void *data);
  static void event_f(const Event *, void *data);
  static void event_w(const Event *, void *data);
  static void event_t(const Event *, void *data);
  static void event_b(const Event *, void *data);
  static void event_i(const Event *, void *data);
  static void event_l(const Event *, void *data);
  static void event_p(const Event *, void *data);
  static void event_c(const Event *, void *data);
  static void event_a(const Event *, void *data);
  static void event_C(const Event *, void *data);
  static void event_B(const Event *, void *data);
  static void event_L(const Event *, void *data);
  static void event_A(const Event *, void *data);
  static void event_h(const Event *, void *data);
  static void event_arrow_up(const Event *, void *data);
  static void event_arrow_down(const Event *, void *data);
  static void event_arrow_left(const Event *, void *data);
  static void event_arrow_right(const Event *, void *data);
  static void event_S(const Event *, void *data);
  static void event_f9(const Event *, void *data);
  static void event_comma(const Event *, void *data);
  static void event_question(const Event * event, void *data);
  static void event_window_event(const Event *, void *data);


  static AsyncTask::DoneStatus task_data_loop(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus task_event(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus task_clear_screenshot_text(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus task_igloop(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus task_record_frame(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus task_play_frame(GenericAsyncTask *task, void *data);

  static AsyncTask::DoneStatus task_clear_text(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus task_garbage_collect(GenericAsyncTask *task, void *data);

private:
  bool _is_open;
  bool _made_default_pipe;

  std::string _window_title;

  PT(GraphicsPipe) _default_pipe;
  PT(GraphicsEngine) _engine;

  NodePath _data_root;
  EventHandler &_event_handler;
  AsyncTaskManager &_task_mgr;

  typedef pvector< PT(WindowFramework) > Windows;
  Windows _windows;

  typedef pmap< const GraphicsOutput *, NodePath > Mouses;
  Mouses _mouses;

  NodePath _models;

  // For counting frame rate.
  double _start_time;
  int _frame_count;

  bool _wireframe_enabled;
  bool _texture_enabled;
  bool _two_sided_enabled;
  bool _lighting_enabled;
  bool _perpixel_enabled;
  WindowFramework::BackgroundType _background_type;

  NodePath _highlight;
  NodePath _highlight_wireframe;

  bool _default_keys_enabled;

  bool _exit_flag;

  class KeyDefinition {
  public:
    std::string _event_name;
    std::string _description;
  };
  typedef pvector<KeyDefinition> KeyDefinitions;
  KeyDefinitions _key_definitions;

  NodePath _help_text;
  NodePath _screenshot_text;

  PT(RecorderController) _recorder;

  friend class WindowFramework;
};

#include "pandaFramework.I"

#endif
