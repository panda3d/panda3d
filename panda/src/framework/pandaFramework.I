/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaFramework.I
 * @author drose
 * @date 2002-04-02
 */

/**
 * Returns the GraphicsEngine that is used to render all the windows in the
 * framework.  Normally there's no reason for user code to mess with this.
 */
INLINE GraphicsEngine *PandaFramework::
get_graphics_engine() {
  if (_engine == nullptr) {
    _engine = GraphicsEngine::get_global_ptr();
    PT(GenericAsyncTask) task = new GenericAsyncTask("igloop", task_igloop, this);
    task->set_sort(50);
    _task_mgr.add(task);
  }
  return _engine;
}

/**
 * Returns the root of the data graph.  This is the graph of nodes that is
 * traversed to control the inputs from user devices like the mouse and
 * keyboard.
 */
INLINE const NodePath &PandaFramework::
get_data_root() const {
  return _data_root;
}

/**
 * Returns the EventHandler object that serves events in the framework.  This
 * is primarily used to dispatch on keypresses and such.
 */
INLINE EventHandler &PandaFramework::
get_event_handler() {
  return _event_handler;
}

/**
 * Returns the Task Manager object that manages tasks in the framework.
 */
INLINE AsyncTaskManager &PandaFramework::
get_task_mgr() {
  return _task_mgr;
}

/**
 * Specifies the title that is set for all subsequently created windows.
 */
INLINE void PandaFramework::
set_window_title(const std::string &title) {
  _window_title = title;
}

/**
 * Returns the number of windows that are currently open.
 */
INLINE int PandaFramework::
get_num_windows() const {
  return _windows.size();
}

/**
 * Returns the nth window currently open.
 */
INLINE WindowFramework *PandaFramework::
get_window(int n) const {
  nassertr(n >= 0 && n < (int)_windows.size(), nullptr);
  return _windows[n];
}

/**
 * Closes the indicated WindowFramework window and removes it from the list.
 */
INLINE void PandaFramework::
close_window(WindowFramework *wf) {
  int n = find_window(wf);
  if (n >= 0) {
    close_window(n);
  }
}

/**
 * Returns the current state of the wireframe flag.
 */
INLINE bool PandaFramework::
get_wireframe() const {
  return _wireframe_enabled;
}

/**
 * Returns the current state of the texture flag.
 */
INLINE bool PandaFramework::
get_texture() const {
  return _texture_enabled;
}

/**
 * Returns the current state of the two_sided flag.
 */
INLINE bool PandaFramework::
get_two_sided() const {
  return _two_sided_enabled;
}

/**
 * Returns the current state of the lighting flag.
 */
INLINE bool PandaFramework::
get_lighting() const {
  return _lighting_enabled;
}

/**
 * Returns the current state of the perpixel flag.
 */
INLINE bool PandaFramework::
get_perpixel() const {
  return _perpixel_enabled;
}

/**
 * Returns the current background type setting.
 */
INLINE WindowFramework::BackgroundType PandaFramework::
get_background_type() const {
  return _background_type;
}

/**
 * Returns true if any node is highlighted, false otherwise.
 */
INLINE bool PandaFramework::
has_highlight() const {
  return !_highlight.is_empty();
}

/**
 * Returns the currently highlighted node, if any, or an empty NodePath if no
 * node is highlighted.
 */
INLINE const NodePath &PandaFramework::
get_highlight() const {
  return _highlight;
}

/**
 * Returns the RecorderController that has been associated with the
 * PandaFramework, if any, or NULL if none has (the normal case).
 *
 * If a RecorderController is associated, it will presumably be used for
 * recording user input to a session file, or for playing back the user input
 * from a previously-recorded session.
 */
INLINE RecorderController *PandaFramework::
get_recorder() const {
  return _recorder;
}

/**
 * Assigns a RecorderController with the PandaFramework.  This should be
 * called before any windows are opened.  The subsequently opened windows will
 * register their user inputs with the recorder.
 *
 * If a RecorderController is associated, it will presumably be used for
 * recording user input to a session file, or for playing back the user input
 * from a previously-recorded session.
 */
INLINE void PandaFramework::
set_recorder(RecorderController *recorder) {
  _recorder = recorder;
}

/**
 * Sets the flag that indicates it is time for the application to exit.  The
 * application will actually exit at the end of the current frame.
 */
INLINE void PandaFramework::
set_exit_flag() {
  _exit_flag = true;
}

/**
 * Resets the exit flag after it has previously been set.
 */
INLINE void PandaFramework::
clear_exit_flag() {
  _exit_flag = false;
}
