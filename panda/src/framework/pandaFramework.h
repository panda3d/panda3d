// Filename: pandaFramework.h
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

#ifndef PANDAFRAMEWORK_H
#define PANDAFRAMEWORK_H

#include "pandabase.h"

#include "windowFramework.h"

#include "nodePath.h"
#include "eventHandler.h"
#include "graphicsPipe.h"
#include "graphicsEngine.h"
#include "graphicsWindow.h"
#include "recorderController.h"
#include "pointerTo.h"

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : PandaFramework
// Description : This class serves to provide a high-level framework
//               for basic applications that use Panda in simple ways
//               (like opening a window to view models, etc.).
////////////////////////////////////////////////////////////////////
class EXPCL_FRAMEWORK PandaFramework {
public:
  PandaFramework();
  virtual ~PandaFramework();

  void open_framework(int &argc, char **&argv);
  void close_framework();

  GraphicsPipe *get_default_pipe();
  INLINE GraphicsEngine *get_graphics_engine();
  INLINE const NodePath &get_data_root() const;
  INLINE EventHandler &get_event_handler();
  NodePath get_mouse(GraphicsWindow *window);
  void remove_mouse(const GraphicsWindow *window);

  void define_key(const string &event_name, 
                  const string &description,
                  EventHandler::EventCallbackFunction *function,
                  void *data);

  INLINE void set_window_title(const string &title);
  virtual void get_default_window_props(WindowProperties &props);

  WindowFramework *open_window(GraphicsPipe *pipe = NULL,
                               GraphicsStateGuardian *gsg = NULL);
  WindowFramework *open_window(const WindowProperties &props,
			       GraphicsPipe *pipe = NULL,
                               GraphicsStateGuardian *gsg = NULL);

  INLINE int get_num_windows() const;
  INLINE WindowFramework *get_window(int n) const;
  int find_window(const GraphicsWindow *win) const;
  int find_window(const WindowFramework *wf) const;
  void close_window(int n);
  INLINE void close_window(WindowFramework *wf);
  void close_all_windows();
  bool all_windows_closed() const;

  const NodePath &get_models();

  void report_frame_rate(ostream &out) const;
  void reset_frame_rate();

  void set_wireframe(bool enable);
  void set_texture(bool enable);
  void set_two_sided(bool enable);
  void set_lighting(bool enable);
  void set_background_type(WindowFramework::BackgroundType type);

  INLINE bool get_wireframe() const;
  INLINE bool get_texture() const;
  INLINE bool get_two_sided() const;
  INLINE bool get_lighting() const;
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

  virtual bool do_frame();
  void main_loop();

  INLINE void set_exit_flag();
  INLINE void clear_exit_flag();

protected:
  virtual PT(WindowFramework) make_window_framework();
  virtual void make_default_pipe();
  virtual void do_enable_default_keys();
  bool clear_text();

  static void event_esc(CPT_Event, void *data);
  static void event_f(CPT_Event, void *data);
  static void event_w(CPT_Event, void *data);
  static void event_t(CPT_Event, void *data);
  static void event_b(CPT_Event, void *data);
  static void event_i(CPT_Event, void *data);
  static void event_l(CPT_Event, void *data);
  static void event_c(CPT_Event, void *data);
  static void event_C(CPT_Event, void *data);
  static void event_B(CPT_Event, void *data);
  static void event_L(CPT_Event, void *data);
  static void event_h(CPT_Event, void *data);
  static void event_arrow_up(CPT_Event, void *data);
  static void event_arrow_down(CPT_Event, void *data);
  static void event_arrow_left(CPT_Event, void *data);
  static void event_arrow_right(CPT_Event, void *data);
  static void event_S(CPT_Event, void *data);
  static void event_f9(CPT_Event, void *data);
  static void event_comma(CPT_Event, void *data);
  static void event_question(CPT_Event event, void *data);
  static void event_window_event(CPT_Event, void *data);


private:
  bool _is_open;
  bool _made_default_pipe;

  string _window_title;

  PT(GraphicsPipe) _default_pipe;
  GraphicsEngine _engine;

  NodePath _data_root;
  EventHandler _event_handler;

  typedef pvector< PT(WindowFramework) > Windows;
  Windows _windows;

  typedef pmap< const GraphicsWindow *, NodePath > Mouses;
  Mouses _mouses;

  NodePath _models;

  // For counting frame rate.
  double _start_time;
  int _frame_count;

  bool _wireframe_enabled;
  bool _texture_enabled;
  bool _two_sided_enabled;
  bool _lighting_enabled;
  WindowFramework::BackgroundType _background_type;

  NodePath _highlight;
  NodePath _highlight_wireframe;

  bool _default_keys_enabled;

  bool _exit_flag;
  
  class KeyDefinition {
  public:
    string _event_name;
    string _description;
  };
  typedef pvector<KeyDefinition> KeyDefinitions;
  KeyDefinitions _key_definitions;

  NodePath _help_text;
  NodePath _screenshot_text;
  double _screenshot_clear_time;

  PT(RecorderController) _recorder;

  friend class WindowFramework;
};

#include "pandaFramework.I"

#endif
