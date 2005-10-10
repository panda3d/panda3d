// Filename: pandaFramework.cxx
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

#include "pandaFramework.h"
#include "clockObject.h"
#include "pStatClient.h"
#include "eventQueue.h"
#include "dataGraphTraverser.h"
#include "collisionNode.h"
#include "config_framework.h"
#include "graphicsPipeSelection.h"
#include "nodePathCollection.h"
#include "textNode.h"
#include "mouseAndKeyboard.h"
#include "mouseRecorder.h"
#include "throw_event.h"

LoaderOptions PandaFramework::_loader_options;

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PandaFramework::
PandaFramework() :
  _event_handler(EventQueue::get_global_event_queue())
{
  _is_open = false;
  _made_default_pipe = false;
  _window_title = string();
  _engine = (GraphicsEngine *)NULL;
  _start_time = 0.0;
  _frame_count = 0;
  _wireframe_enabled = false;
  _texture_enabled = true;
  _two_sided_enabled = false;
  _lighting_enabled = false;
  _background_type = WindowFramework::BT_default;
  _default_keys_enabled = false;
  _exit_flag = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PandaFramework::
~PandaFramework() {
  if (_is_open) {
    close_framework();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::open_framework
//       Access: Public
//  Description: Should be called once at the beginning of the
//               application to initialize Panda (and the framework)
//               for use.  The command-line arguments should be passed
//               in so Panda can remove any arguments that it
//               recognizes as special control parameters.
////////////////////////////////////////////////////////////////////
void PandaFramework::
open_framework(int &argc, char **&argv) {
  if (_is_open) {
    return;
  }

  _is_open = true;

  reset_frame_rate();

  _data_root = NodePath("data");
  _highlight_wireframe = NodePath("wireframe");
  _highlight_wireframe.set_render_mode_wireframe(1);
  _highlight_wireframe.set_color(1.0f, 0.0f, 0.0f, 1.0f, 1);

  if (!playback_session.empty()) {
    // If the config file so indicates, create a recorder and start it
    // playing.
    _recorder = new RecorderController;
    _recorder->begin_playback(Filename::from_os_specific(playback_session));
    
  } else if (!record_session.empty()) {
    // If the config file so indicates, create a recorder and start it
    // recording.
    _recorder = new RecorderController;
    _recorder->begin_record(Filename::from_os_specific(record_session));
  } 

  _event_handler.add_hook("window-event", event_window_event, this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::close_framework
//       Access: Public
//  Description: Should be called at the end of an application to
//               close Panda.  This is optional, as the destructor
//               will do the same thing.
////////////////////////////////////////////////////////////////////
void PandaFramework::
close_framework() {
  if (!_is_open) {
    return;
  }

  close_all_windows();
  // Also close down any other windows that might have been opened.
  if (_engine != (GraphicsEngine *)NULL) {
    delete _engine;
    _engine = NULL;
  }

  _event_handler.remove_all_hooks();

  _is_open = false;
  _made_default_pipe = false;
  _default_pipe.clear();

  _start_time = 0.0;
  _frame_count = 0;
  _wireframe_enabled = false;
  _two_sided_enabled = false;
  _lighting_enabled = false;
  _default_keys_enabled = false;
  _exit_flag = false;

  _recorder = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::get_default_pipe
//       Access: Public
//  Description: Returns the default pipe.  This is the GraphicsPipe
//               that all windows in the framework will be created on,
//               unless otherwise specified in open_window().  It is
//               usually the primary graphics interface on the local
//               machine.
//
//               If the default pipe has not yet been created, this
//               creates it.
//
//               The return value is the default pipe, or NULL if no
//               default pipe could be created.
////////////////////////////////////////////////////////////////////
GraphicsPipe *PandaFramework::
get_default_pipe() {
  nassertr(_is_open, NULL);
  if (!_made_default_pipe) {
    make_default_pipe();
    _made_default_pipe = true;
  }
  return _default_pipe;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::get_mouse
//       Access: Public
//  Description: Returns a NodePath to the MouseAndKeyboard associated
//               with the indicated GraphicsWindow object.  If there's
//               not yet a mouse associated with the window, creates
//               one.
//
//               This allows multiple WindowFramework objects that
//               represent different display regions of the same
//               GraphicsWindow to share the same mouse.
////////////////////////////////////////////////////////////////////
NodePath PandaFramework::
get_mouse(GraphicsWindow *window) {
  Mouses::iterator mi = _mouses.find(window);
  if (mi != _mouses.end()) {
    return (*mi).second;
  }

  NodePath mouse;

  NodePath data_root = get_data_root();
  MouseAndKeyboard *mouse_node = new MouseAndKeyboard(window, 0, "mouse");
  mouse = data_root.attach_new_node(mouse_node);

  RecorderController *recorder = get_recorder();
  if (recorder != (RecorderController *)NULL) {
    // If we're in recording or playback mode, associate a recorder.
    MouseRecorder *mouse_recorder = new MouseRecorder("mouse");
    mouse = mouse.attach_new_node(mouse_recorder);
    recorder->add_recorder("mouse", mouse_recorder);
  }

  _mouses[window] = mouse;

  return mouse;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::remove_mouse
//       Access: Public
//  Description: Removes the mouse that may have been created by an
//               earlier call to get_mouse().
////////////////////////////////////////////////////////////////////
void PandaFramework::
remove_mouse(const GraphicsWindow *window) {
  Mouses::iterator mi = _mouses.find(window);
  if (mi != _mouses.end()) {
    (*mi).second.remove_node();
    _mouses.erase(mi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::define_key
//       Access: Public
//  Description: Sets up a handler for the indicated key.  When the
//               key is pressed in a window, the given callback will
//               be called.  The description is a one-line description
//               of the function of the key, for display to the user.
////////////////////////////////////////////////////////////////////
void PandaFramework::
define_key(const string &event_name, const string &description,
           EventHandler::EventCallbackFunction *function,
           void *data) {
  if (_event_handler.has_hook(event_name)) {
    // If there is already a hook for the indicated keyname, we're
    // most likely replacing a previous definition of a key.  Search
    // for the old definition and remove it.
    KeyDefinitions::iterator di;
    di = _key_definitions.begin();
    while (di != _key_definitions.end() && (*di)._event_name != event_name) {
      ++di;
    }
    if (di != _key_definitions.end()) {
      _key_definitions.erase(di);
    }
  }

  // Now add a new hook for the keyname, and also add the new
  // description.
  _event_handler.add_hook(event_name, function, data);

  if (!description.empty()) {
    KeyDefinition keydef;
    keydef._event_name = event_name;
    keydef._description = description;
    _key_definitions.push_back(keydef);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::get_default_window_props
//       Access: Public, Virtual
//  Description: Fills in the indicated window properties structure
//               according to the normal window properties for this
//               application.
////////////////////////////////////////////////////////////////////
void PandaFramework::
get_default_window_props(WindowProperties &props) {
  // This function is largely vestigial and will be removed soon.  We
  // have moved the default window properties into
  // WindowProperties::get_default().

  props.add_properties(WindowProperties::get_default());
  if (!_window_title.empty()) {
    props.set_title(_window_title);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::open_window
//       Access: Public
//  Description: Opens a window on the default graphics pipe.  If the
//               default graphics pipe can't open a window for some
//               reason, automatically fails over to the next
//               available graphics pipe, and updates _default_pipe
//               accordingly.  Returns NULL only if all graphics pipes
//               fail.
////////////////////////////////////////////////////////////////////
WindowFramework *PandaFramework::
open_window() {
  GraphicsPipe *pipe = get_default_pipe();
  if (pipe == (GraphicsPipe *)NULL) {
    // Can't get a pipe.
    return NULL;
  }

  WindowFramework *wf = open_window(pipe, NULL);
  if (wf == (WindowFramework *)NULL) {
    // Ok, the default graphics pipe failed; try a little harder.
    GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
    selection->load_aux_modules();

    int num_pipe_types = selection->get_num_pipe_types();
    for (int i = 0; i < num_pipe_types; i++) {
      TypeHandle pipe_type = selection->get_pipe_type(i);
      if (pipe_type != _default_pipe->get_type()) {
        PT(GraphicsPipe) new_pipe = selection->make_pipe(pipe_type);
        if (new_pipe != (GraphicsPipe *)NULL) {
          wf = open_window(new_pipe, NULL);
          if (wf != (WindowFramework *)NULL) {
            // Here's the winner!
            _default_pipe = new_pipe;
            return wf;
          }
        }
      }
    }

    // Too bad; none of the pipes could open a window.  Fall through
    // and return NULL.
  }

  return wf;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::open_window
//       Access: Public
//  Description: Opens a new window on the indicated pipe, using the
//               default parameters.  Returns the new WindowFramework
//               if successful, or NULL if not.
////////////////////////////////////////////////////////////////////
WindowFramework *PandaFramework::
open_window(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) {
  nassertr(_is_open, NULL);

  WindowProperties props;
  get_default_window_props(props);

  return open_window(props, pipe, gsg);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::open_window
//       Access: Public
//  Description: Opens a new window using the indicated properties.
//               (You may initialize the properties to their default
//               values by calling get_default_window_props() first.)
//
//               Returns the new WindowFramework if successful, or
//               NULL if not.
////////////////////////////////////////////////////////////////////
WindowFramework *PandaFramework::
open_window(const WindowProperties &props, GraphicsPipe *pipe,
            GraphicsStateGuardian *gsg) {
  if (pipe == (GraphicsPipe *)NULL) {
    pipe = get_default_pipe();
    if (pipe == (GraphicsPipe *)NULL) {
      // Can't get a pipe.
      return NULL;
    }
  }

  nassertr(_is_open, NULL);
  PT(WindowFramework) wf = make_window_framework();
  wf->set_wireframe(get_wireframe());
  wf->set_texture(get_texture());
  wf->set_two_sided(get_two_sided());
  wf->set_lighting(get_lighting());
  wf->set_background_type(get_background_type());

  GraphicsWindow *win = wf->open_window(props, get_graphics_engine(), 
                                        pipe, gsg);
  _engine->open_windows();
  if (win != (GraphicsWindow *)NULL && !win->is_valid()) {
    // The window won't open.
    _engine->remove_window(win);
    wf->close_window();
    win = NULL;
  }

  if (win == (GraphicsWindow *)NULL) {
    // Oops, couldn't make an actual window.
    framework_cat.error()
      << "Unable to create window.\n";
    return NULL;
  }

  _windows.push_back(wf);
  return wf;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::find_window
//       Access: Public
//  Description: Returns the index of the first WindowFramework object
//               found that references the indicated GraphicsWindow
//               pointer, or -1 if none do.
////////////////////////////////////////////////////////////////////
int PandaFramework::
find_window(const GraphicsWindow *win) const {
  int n;
  for (n = 0; n < (int)_windows.size(); n++) {
    if (_windows[n]->get_graphics_window() == win) {
      return n;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::find_window
//       Access: Public
//  Description: Returns the index of the given WindowFramework
//               object, or -1 if the object does not represent a
//               window opened with this PandaFramework.
////////////////////////////////////////////////////////////////////
int PandaFramework::
find_window(const WindowFramework *wf) const {
  int n;
  for (n = 0; n < (int)_windows.size(); n++) {
    if (_windows[n] == wf) {
      return n;
    }
  }

  return -1;
}


////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::close_window
//       Access: Public
//  Description: Closes the nth window and removes it from the list.
////////////////////////////////////////////////////////////////////
void PandaFramework::
close_window(int n) {
  nassertv(n >= 0 && n < (int)_windows.size());
  WindowFramework *wf = _windows[n];

  GraphicsWindow *win = wf->get_graphics_window();
  if (win != (GraphicsWindow *)NULL) {
    _engine->remove_window(win);
  }
  
  wf->close_window();
  _windows.erase(_windows.begin() + n);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::close_all_windows
//       Access: Public
//  Description: Closes all currently open windows and empties the
//               list of windows.
////////////////////////////////////////////////////////////////////
void PandaFramework::
close_all_windows() {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);

    GraphicsWindow *win = wf->get_graphics_window();
    if (win != (GraphicsWindow *)NULL) {
      _engine->remove_window(win);
    }
    
    wf->close_window();
  }

  Mouses::iterator mi;
  for (mi = _mouses.begin(); mi != _mouses.end(); ++mi) {
    (*mi).second.remove_node();
  }

  _windows.clear();
  _mouses.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::all_windows_closed
//       Access: Public
//  Description: Returns true if all of the opened windows have been
//               closed by the user, false otherwise.
////////////////////////////////////////////////////////////////////
bool PandaFramework::
all_windows_closed() const {
  Windows::const_iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);
    if (wf->get_graphics_window()->get_properties().get_open()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::get_models
//       Access: Public
//  Description: Returns the root of the scene graph normally reserved
//               for parenting models and such.  This scene graph may
//               be instanced to each window's render tree as the
//               window is created.
////////////////////////////////////////////////////////////////////
NodePath &PandaFramework::
get_models() {
  if (_models.is_empty()) {
    _models = NodePath("models");
  }
  return _models;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::report_frame_rate
//       Access: Public
//  Description: Reports the currently measured average frame rate to
//               the indicated ostream.
////////////////////////////////////////////////////////////////////
void PandaFramework::
report_frame_rate(ostream &out) const {
  double now = ClockObject::get_global_clock()->get_frame_time();
  double delta = now - _start_time;
  
  int frame_count = ClockObject::get_global_clock()->get_frame_count();
  int num_frames = frame_count - _frame_count;
  if (num_frames > 0) {
    out << num_frames << " frames in " << delta << " seconds.\n";
    double fps = ((double)num_frames) / delta;
    out << fps << " fps average (" << 1000.0 / fps << "ms)\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::reset_frame_rate
//       Access: Public
//  Description: Resets the frame rate computation.
////////////////////////////////////////////////////////////////////
void PandaFramework::
reset_frame_rate() {
  _start_time = ClockObject::get_global_clock()->get_frame_time();
  _frame_count = ClockObject::get_global_clock()->get_frame_count();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::set_wireframe
//       Access: Public
//  Description: Sets the wireframe state on all windows.
////////////////////////////////////////////////////////////////////
void PandaFramework::
set_wireframe(bool enable) {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);
    wf->set_wireframe(enable);
  }

  _wireframe_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::set_texture
//       Access: Public
//  Description: Sets the texture state on all windows.
////////////////////////////////////////////////////////////////////
void PandaFramework::
set_texture(bool enable) {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);
    wf->set_texture(enable);
  }

  _texture_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::set_two_sided
//       Access: Public
//  Description: Sets the two_sided state on all windows.
////////////////////////////////////////////////////////////////////
void PandaFramework::
set_two_sided(bool enable) {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);
    wf->set_two_sided(enable);
  }

  _two_sided_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::set_lighting
//       Access: Public
//  Description: Sets the lighting state on all windows.
////////////////////////////////////////////////////////////////////
void PandaFramework::
set_lighting(bool enable) {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);
    wf->set_lighting(enable);
  }

  _lighting_enabled = enable;
}

////////////////////////////////////////////////////////////////////
//     Function: BackgroundFramework::set_background_type
//       Access: Public
//  Description: Sets the background type of all windows.
////////////////////////////////////////////////////////////////////
void PandaFramework::
set_background_type(WindowFramework::BackgroundType type) {
  Windows::iterator wi;
  for (wi = _windows.begin(); wi != _windows.end(); ++wi) {
    WindowFramework *wf = (*wi);
    wf->set_background_type(type);
  }

  _background_type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::hide_collision_solids
//       Access: Public
//  Description: Hides any collision solids which are visible in the
//               indicated scene graph.  Returns the number of
//               collision solids hidden.
////////////////////////////////////////////////////////////////////
int PandaFramework::
hide_collision_solids(NodePath node) {
  int num_changed = 0;

  if (node.node()->is_of_type(CollisionNode::get_class_type())) {
    if (!node.is_hidden()) {
      node.hide();
      num_changed++;
    }
  }

  int num_children = node.get_num_children();
  for (int i = 0; i < num_children; i++) {
    num_changed += hide_collision_solids(node.get_child(i));
  }

  return num_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::show_collision_solids
//       Access: Public
//  Description: Shows any collision solids which are directly hidden
//               in the indicated scene graph.  Returns the number of
//               collision solids shown.
////////////////////////////////////////////////////////////////////
int PandaFramework::
show_collision_solids(NodePath node) {
  int num_changed = 0;

  if (node.node()->is_of_type(CollisionNode::get_class_type())) {
    if (node.get_hidden_ancestor() == node) {
      node.show();
      num_changed++;
    }
  }

  int num_children = node.get_num_children();
  for (int i = 0; i < num_children; i++) {
    num_changed += show_collision_solids(node.get_child(i));
  }

  return num_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::set_highlight
//       Access: Public
//  Description: Sets the indicated node (normally a node within the
//               get_models() tree) up as the highlighted node.
//               Certain operations affect the highlighted node only.
////////////////////////////////////////////////////////////////////
void PandaFramework::
set_highlight(const NodePath &node) {
  clear_highlight();
  _highlight = node;
  if (!_highlight.is_empty()) {
    framework_cat.info(false) << _highlight << "\n";
    _highlight.show_bounds();

    // Also create a new instance of the highlighted geometry, as a
    // sibling of itself, under the special highlight property.
    if (_highlight.has_parent()) {
      _highlight_wireframe.reparent_to(_highlight.get_parent());
      _highlight.instance_to(_highlight_wireframe);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::clear_highlight
//       Access: Public
//  Description: Unhighlights the currently highlighted node, if any.
////////////////////////////////////////////////////////////////////
void PandaFramework::
clear_highlight() {
  if (!_highlight.is_empty()) {
    _highlight.hide_bounds();
    _highlight = NodePath();

    // Clean up the special highlight instance.
    _highlight_wireframe.detach_node();
    _highlight_wireframe.get_children().detach();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::enable_default_keys
//       Access: Public
//  Description: Sets callbacks on the event handler to handle all of
//               the normal viewer keys, like t to toggle texture, ESC
//               or q to quit, etc.
////////////////////////////////////////////////////////////////////
void PandaFramework::
enable_default_keys() {
  if (!_default_keys_enabled) {
    do_enable_default_keys();
    _default_keys_enabled = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::do_frame
//       Access: Public, Virtual
//  Description: Renders one frame and performs all associated
//               processing.  Returns true if we should continue
//               rendering, false if we should exit.  This is normally
//               called only from main_loop().
////////////////////////////////////////////////////////////////////
bool PandaFramework::
do_frame() {
  nassertr(_is_open, false);
  DataGraphTraverser dg_trav;
  dg_trav.traverse(_data_root.node());

  throw_event("NewFrame");
  _event_handler.process_events();

  if (!_screenshot_text.is_empty()) {
    double now = ClockObject::get_global_clock()->get_frame_time();
    if (now >= _screenshot_clear_time) {
      _screenshot_text.remove_node();
    }
  }

  if (_recorder != (RecorderController *)NULL) {
    _recorder->record_frame();
  }
  
  if (_engine != (GraphicsEngine *)NULL) {
    _engine->render_frame();
  }

  if (_recorder != (RecorderController *)NULL) {
    _recorder->play_frame();
  }

  return !_exit_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::main_loop
//       Access: Public
//  Description: Called to yield control to the panda framework.  This
//               function does not return until set_exit_flag() has
//               been called.
////////////////////////////////////////////////////////////////////
void PandaFramework::
main_loop() {
  while (do_frame()) {
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::make_window_framework
//       Access: Protected, Virtual
//  Description: Creates a new WindowFramework object.  This is
//               provided as a hook so derived PandaFramework classes
//               can create custom WindowFramework objects.
////////////////////////////////////////////////////////////////////
PT(WindowFramework) PandaFramework::
make_window_framework() {
  return new WindowFramework(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::make_default_pipe
//       Access: Protected, Virtual
//  Description: Creates the default GraphicsPipe that will contain
//               all windows that are not opened on a specific pipe.
////////////////////////////////////////////////////////////////////
void PandaFramework::
make_default_pipe() {
  // This depends on the shared library or libraries (DLL's to you
  // Windows folks) that have been loaded in at runtime from the
  // load-display and/or aux-display Configrc variables.
  GraphicsPipeSelection *selection = GraphicsPipeSelection::get_global_ptr();
  selection->print_pipe_types();
  _default_pipe = selection->make_default_pipe();

  if (_default_pipe == (GraphicsPipe*)NULL) {
    nout << "No graphics pipe is available!\n"
         << "Your Config.prc file must name at least one valid panda display\n"
         << "library via load-display or aux-display.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::do_enable_default_keys
//       Access: Protected, Virtual
//  Description: The implementation of enable_default_keys().
////////////////////////////////////////////////////////////////////
void PandaFramework::
do_enable_default_keys() {
  define_key("escape", "close window", event_esc, this);
  define_key("q", "close window", event_esc, this);
  define_key("f", "report frame rate", event_f, this);
  define_key("w", "toggle wireframe mode", event_w, this);
  define_key("t", "toggle texturing", event_t, this);
  define_key("b", "toggle backface (double-sided) rendering", event_b, this);
  define_key("i", "invert (reverse) single-sided faces", event_i, this);
  define_key("l", "toggle lighting", event_l, this);
  define_key("c", "recenter view on object", event_c, this);
  define_key("a", "toggle animation controls", event_a, this);
  define_key("shift-c", "toggle collision surfaces", event_C, this);
  define_key("shift-b", "report bounding volume", event_B, this);
  define_key("shift-l", "list hierarchy", event_L, this);
  define_key("h", "highlight node", event_h, this);
  define_key("arrow_up", "move highlight to parent", event_arrow_up, this);
  define_key("arrow_down", "move highlight to child", event_arrow_down, this);
  define_key("arrow_left", "move highlight to sibling", event_arrow_left, this);
  define_key("arrow_right", "move highlight to sibling", event_arrow_right, this);
  define_key("shift-s", "activate PStats", event_S, this);
  define_key("f9", "Take screenshot", event_f9, this);
  define_key(",", "change background color", event_comma, this);
  define_key("?", "", event_question, this);
  define_key("shift-/", "", event_question, this);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::clear_text
//       Access: Protected
//  Description: Removes any onscreen text (like help text or
//               screenshot filename).  Returns true if there was any
//               text in the first place, false otherwise.
////////////////////////////////////////////////////////////////////
bool PandaFramework::
clear_text() {
  bool any_text = false;
  if (!_screenshot_text.is_empty()) {
    _screenshot_text.remove_node();
    any_text = true;
  }

  if (!_help_text.is_empty()) {
    _help_text.remove_node();
    any_text = true;
  }

  return any_text;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_esc
//       Access: Public, Static
//  Description: Default handler for ESC or q key: close the current
//               window (and exit the application if that was the last
//               window).
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_esc(CPT_Event event, void *data) { 
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    PT(GraphicsWindow) win = wf->get_graphics_window();

    PandaFramework *self = (PandaFramework *)data;
    self->close_window(wf);

    // Also close any other WindowFrameworks on the same window.
    int window_index = self->find_window(win);
    while (window_index != -1) {
      self->close_window(window_index);
      window_index = self->find_window(win);
    }

    // Free up the mouse for that window.
    self->remove_mouse(win);

    // If we closed the last window, shut down.
    if (self->all_windows_closed()) {
      self->_exit_flag = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_f
//       Access: Public, Static
//  Description: Default handler for f key: report and reset frame
//               rate.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_f(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;
  self->report_frame_rate(nout);
  self->reset_frame_rate();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_w
//       Access: Public, Static
//  Description: Default handler for w key: toggle wireframe.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_w(CPT_Event event, void *) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    wf->set_wireframe(!wf->get_wireframe());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_t
//       Access: Public, Static
//  Description: Default handler for t key: toggle texture.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_t(CPT_Event event, void *) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    wf->set_texture(!wf->get_texture());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_b
//       Access: Public, Static
//  Description: Default handler for b key: toggle backface (two-sided
//               rendering).
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_b(CPT_Event event, void *) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    wf->set_two_sided(!wf->get_two_sided());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_i
//       Access: Public, Static
//  Description: Default handler for i key: invert one-sided faces.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_i(CPT_Event event, void *) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    wf->set_one_sided_reverse(!wf->get_one_sided_reverse());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_l
//       Access: Public, Static
//  Description: Default handler for l key: toggle lighting.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_l(CPT_Event event, void *) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    wf->set_lighting(!wf->get_lighting());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_c
//       Access: Public, Static
//  Description: Default handler for c key: center the trackball over
//               the scene, or over the highlighted part of the scene.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_c(CPT_Event event, void *data) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    PandaFramework *self = (PandaFramework *)data;

    NodePath node = self->get_highlight();
    if (node.is_empty()) {
      node = self->get_models();
    }
    wf->center_trackball(node);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_a
//       Access: Public, Static
//  Description: Default handler for a key: toggle the animation
//               controls.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_a(CPT_Event event, void *data) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    wf->next_anim_control();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_C
//       Access: Public, Static
//  Description: Default handler for shift-C key: toggle the showing
//               of collision solids.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_C(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  NodePath node = self->get_highlight();
  if (node.is_empty()) {
    node = self->get_models();
  }

  if (self->hide_collision_solids(node) == 0) {
    self->show_collision_solids(node);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_B
//       Access: Public, Static
//  Description: Default handler for shift-B key: describe the
//               bounding volume of the currently selected object, or
//               the entire scene.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_B(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  NodePath node = self->get_highlight();
  if (node.is_empty()) {
    node = self->get_models();
  }

  node.get_bounds()->write(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_L
//       Access: Public, Static
//  Description: Default handler for shift-L key: list the contents of
//               the scene graph, or the highlighted node.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_L(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  NodePath node = self->get_highlight();
  if (node.is_empty()) {
    node = self->get_models();
  }

  node.ls();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_h
//       Access: Public, Static
//  Description: Default handler for h key: toggle highlight mode.  In
//               this mode, you can walk the scene graph with the
//               arrow keys to highlight different nodes.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_h(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;
  
  if (self->has_highlight()) {
    self->clear_highlight();
  } else {
    self->set_highlight(self->get_models());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_arrow_up
//       Access: Public, Static
//  Description: Default handler for up arrow key: in highlight mode,
//               move the highlight to the node's parent.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_arrow_up(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  if (self->has_highlight()) {
    NodePath node = self->get_highlight();
    if (node.has_parent() && node != self->get_models()) {
      self->set_highlight(node.get_parent());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_arrow_down
//       Access: Public, Static
//  Description: Default handler for up arrow key: in highlight mode,
//               move the highlight to the node's first child.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_arrow_down(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  if (self->has_highlight()) {
    NodePath node = self->get_highlight();
    if (node.get_num_children() > 0) {
      self->set_highlight(node.get_child(0));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_arrow_left
//       Access: Public, Static
//  Description: Default handler for up arrow key: in highlight mode,
//               move the highlight to the node's nearest sibling on
//               the left.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_arrow_left(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  if (self->has_highlight()) {
    NodePath node = self->get_highlight();
    NodePath parent = node.get_parent();
    if (node.has_parent() && node != self->get_models()) {
      int index = parent.node()->find_child(node.node());
      nassertv(index >= 0);
      int sibling = index - 1;

      if (sibling >= 0 && 
          parent.node()->get_child(sibling) == self->_highlight_wireframe.node()) {
        // Skip over the special highlight node.
        sibling--;
      }

      if (sibling >= 0) {
        self->set_highlight(NodePath(parent, parent.node()->get_child(sibling)));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_arrow_right
//       Access: Public, Static
//  Description: Default handler for up arrow key: in highlight mode,
//               move the highlight to the node's nearest sibling on
//               the right.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_arrow_right(CPT_Event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  if (self->has_highlight()) {
    NodePath node = self->get_highlight();
    NodePath parent = node.get_parent();
    if (node.has_parent() && node != self->get_models()) {
      int index = parent.node()->find_child(node.node());
      nassertv(index >= 0);
      int num_children = parent.node()->get_num_children();
      int sibling = index + 1;

      if (sibling < num_children && 
          parent.node()->get_child(sibling) == self->_highlight_wireframe.node()) {
        // Skip over the special highlight node.
        sibling++;
      }
      
      if (sibling < num_children) {
        self->set_highlight(NodePath(parent, parent.node()->get_child(sibling)));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_S
//       Access: Public, Static
//  Description: Default handler for shift-S key: activate stats.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_S(CPT_Event, void *) {
#ifdef DO_PSTATS
  nout << "Connecting to stats host" << endl;
  PStatClient::connect();
#else
  nout << "Stats host not supported." << endl;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_f9
//       Access: Public, Static
//  Description: Default handler for f9 key: take screenshot.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_f9(CPT_Event event, void *data) {
  PandaFramework *self = (PandaFramework *)data;

  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    if (self->clear_text()) {
      // Render one more frame to remove the text.
      self->_engine->render_frame();
    }

    Filename filename = wf->get_graphics_window()->save_screenshot_default();
    string text;
    if (filename.empty()) {
      text = "Screenshot failed";
    } else {
      text = filename;
    }

    TextNode *text_node = new TextNode("screenshot");
    self->_screenshot_text = NodePath(text_node);
    text_node->set_align(TextNode::A_center);
    text_node->set_shadow_color(0.0f, 0.0f, 0.0f, 1.0f);
    text_node->set_shadow(0.04f, 0.04f);
    text_node->set_text(text);
    self->_screenshot_text.set_scale(0.06);
    self->_screenshot_text.set_pos(0.0, 0.0, -0.7);
    self->_screenshot_text.reparent_to(wf->get_aspect_2d());

    double now = ClockObject::get_global_clock()->get_frame_time();
    self->_screenshot_clear_time = now + 2.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_comma
//       Access: Public, Static
//  Description: Default handler for comma key: rotate background color.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_comma(CPT_Event event, void *) {
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    switch (wf->get_background_type()) {
    case WindowFramework::BT_other:
    case WindowFramework::BT_none:
      break;
      
    case WindowFramework::BT_white:
      wf->set_background_type(WindowFramework::BT_default);
      break;
      
    default:
      wf->set_background_type((WindowFramework::BackgroundType)(wf->get_background_type() + 1));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_question
//       Access: Public, Static
//  Description: Default handler for ? key: show the available keys.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_question(CPT_Event event, void *data) {
  PandaFramework *self = (PandaFramework *)data;
  if (event->get_num_parameters() == 1) {
    EventParameter param = event->get_parameter(0);
    WindowFramework *wf;
    DCAST_INTO_V(wf, param.get_ptr());

    self->_screenshot_text.remove_node();

    if (!self->_help_text.is_empty()) {
      self->_help_text.remove_node();
      // This key is a toggle; remove the help text and do nothing else.

    } else {
      // Build up a string to display.
      ostringstream help;
      KeyDefinitions::const_iterator ki;
      for (ki = self->_key_definitions.begin(); 
           ki != self->_key_definitions.end(); 
           ++ki) {
        const KeyDefinition &keydef = (*ki);
        help << keydef._event_name << "\t" << keydef._description << "\n";
      }

      string help_text = help.str();

      TextNode *text_node = new TextNode("help");
      self->_help_text = NodePath(text_node);
      text_node->set_align(TextNode::A_left);
      text_node->set_shadow_color(0.0f, 0.0f, 0.0f, 1.0f);
      text_node->set_shadow(0.04f, 0.04f);
      text_node->set_text(help_text);

      LVecBase4f frame = text_node->get_frame_actual();

      float height = frame[3] - frame[2];
      float scale = min(0.06, 1.8 / height);
      self->_help_text.set_scale(scale);

      float pos_scale = scale / -2.0;
      self->_help_text.set_pos((frame[0] + frame[1]) * pos_scale,
                               0.0,
                               (frame[2] + frame[3]) * pos_scale);

      self->_help_text.reparent_to(wf->get_aspect_2d());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaFramework::event_window_event
//       Access: Public, Static
//  Description: Default handler for window events: window resized or
//               closed, etc.
////////////////////////////////////////////////////////////////////
void PandaFramework::
event_window_event(CPT_Event event, void *data) {
  PandaFramework *self = (PandaFramework *)data;
  if (event->get_num_parameters() == 1) {
    // The parameter of the window event is the window itself, rather
    // than the window framework object (which is the parameter of all
    // of the keyboard events).
    EventParameter param = event->get_parameter(0);
    const GraphicsWindow *win;
    DCAST_INTO_V(win, param.get_ptr());

    // Is this a window we've heard about?
    int window_index = self->find_window(win);
    if (window_index == -1) {
      framework_cat.debug()
        << "Ignoring message from unknown window.\n";

    } else {
      if (!win->get_properties().get_open()) {
        int window_index = self->find_window(win);
        while (window_index != -1) {
          self->close_window(window_index);
          window_index = self->find_window(win);
        }
        
        // Free up the mouse for that window.
        self->remove_mouse(win);

        // If the last window was closed, exit the application.
        if (self->all_windows_closed() && !self->_exit_flag) {
          framework_cat.info()
            << "Last window was closed by user.\n";
          self->_exit_flag = true;
        }
      }
    }
  }
}
