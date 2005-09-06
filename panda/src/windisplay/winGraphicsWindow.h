// Filename: winGraphicsWindow.h
// Created by:  drose (20Dec02)
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

#ifndef WINGRAPHICSWINDOW_H
#define WINGRAPHICSWINDOW_H

#include "pandabase.h"
#include "graphicsWindow.h"

#include <windows.h>

class WinGraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : WinGraphicsWindow
// Description : An abstract base class for glGraphicsWindow and
//               dxGraphicsWindow (and, in general, graphics windows
//               that interface with the Microsoft Windows API).
//
//               This class includes all the code for manipulating
//               windows themselves: opening them, closing them,
//               responding to user keyboard and mouse input, and so
//               on.  It does not make any 3-D rendering calls into
//               the window; that is the province of the
//               GraphicsStateGuardian.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAWIN WinGraphicsWindow : public GraphicsWindow {
public:
  WinGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    const string &name);
  virtual ~WinGraphicsWindow();

  virtual bool move_pointer(int device, int x, int y);

  virtual void close_ime();

  virtual void begin_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);
  virtual LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual bool handle_mouse_motion(int x, int y);
  virtual void handle_mouse_exit();

  INLINE HWND get_ime_hwnd();


protected:
  virtual void close_window();
  virtual bool open_window();
  virtual void fullscreen_minimized(WindowProperties &properties);
  virtual void fullscreen_restored(WindowProperties &properties);

  virtual bool do_reshape_request(int x_origin, int y_origin, 
                                  int x_size, int y_size);

  virtual void handle_reshape();
  virtual bool do_fullscreen_resize(int x_size, int y_size);

  virtual void reconsider_fullscreen_size(DWORD &x_size, DWORD &y_size, 
                                          DWORD &bitdepth);

  virtual void support_overlay_window(bool flag);

private:
  bool open_fullscreen_window();
  bool open_regular_window();
  void adjust_z_order();
  void adjust_z_order(WindowProperties::ZOrder last_z_order,
                      WindowProperties::ZOrder this_z_order);
  void initialize_input_devices();
  void handle_raw_input(HRAWINPUT hraw);
  void track_mouse_leaving(HWND hwnd);

  static void process_1_event();

  INLINE void handle_keypress(ButtonHandle key, int x, int y, double time);
  INLINE void handle_keyresume(ButtonHandle key, double time);
  INLINE void handle_keyrelease(ButtonHandle key, double time);
  ButtonHandle lookup_key(WPARAM wparam) const;
  INLINE int translate_mouse(int pos) const;
  INLINE void set_cursor_in_window();
  INLINE void set_cursor_out_of_window();

  INLINE static double get_message_time();

  void resend_lost_keypresses();
  static void update_cursor_window(WinGraphicsWindow *to_window);
  static void hide_or_show_cursor(bool hide_cursor);

  static bool find_acceptable_display_mode(DWORD dwWidth, DWORD dwHeight,
                                           DWORD bpp, DEVMODE &dm);
  static void show_error_message(DWORD message_id = 0);

protected:
  HWND _hWnd;

private:
  HWND _ime_hWnd;
  bool _ime_open;
  bool _ime_active;
  bool _ime_composition_w;
  bool _tracking_mouse_leaving;
  bool _maximized;
  bool _bCursor_in_WindowClientArea;
  HANDLE _input_device_handle[32];
  HCURSOR _cursor;
  DEVMODE _fullscreen_display_mode;

  // This is used to remember the state of the keyboard when keyboard
  // focus is lost.
  enum { num_virtual_keys = 256 };
  // You might be wondering why the above is an enum. Originally the line
  // read "static const int num_virtual_keys = 256"
  // but in trying to support the MSVC6 compiler, we found that you
  // were not allowed to define the value of a const within a class like
  // that. Defining the value outside the class helps, but then we can't
  // use the value to define the length of the _keyboard_state array, and
  // also it creates multiply defined symbol errors when you link, because
  // other files include this header file. This enum is a clever solution
  // to work around the problem.

  BYTE _keyboard_state[num_virtual_keys];
  bool _lost_keypresses;

  // These are used to store the status of the individual left and right
  // shift, control, and alt keys.  Keyboard events are not sent for
  // these individual keys, but for each pair as a whole.  The status
  // of each key must be checked as keypress and keyrelease events are
  // received.
  bool _lshift_down;
  bool _rshift_down;
  bool _lcontrol_down;
  bool _rcontrol_down;
  bool _lalt_down;
  bool _ralt_down;

private:
  // We need this map to support per-window calls to window_proc().
  typedef map<HWND, WinGraphicsWindow *> WindowHandles;
  static WindowHandles _window_handles;

  // And we need a static pointer to the current WinGraphicsWindow we
  // are creating at the moment, since CreateWindow() starts
  // generating window events before it gives us the window handle.
  static WinGraphicsWindow *_creating_window;

  // This tracks the current GraphicsWindow whose client area contains
  // the mouse.  There will only be one of these at a time, and
  // storing the pointer here allows us to handle ambiguities in the
  // order in which messages are passed from Windows to the various
  // windows we manage.  This pointer is used by
  // set_cursor_in_window() to determine when it is time to call
  // update_cursor() to hide the cursor (or do other related
  // operations).
  static WinGraphicsWindow *_cursor_window;
  static bool _cursor_hidden;
  static bool _got_saved_params;
  static int _saved_mouse_trails;
  static BOOL _saved_cursor_shadow;
  static BOOL _saved_mouse_vanish;

  // Since the Panda API requests icons and cursors by filename, we
  // need a table mapping filenames to handles, so we can avoid
  // re-reading the file each time we change icons.
  typedef pmap<Filename, HANDLE> IconFilenames;
  static IconFilenames _icon_filenames;
  static IconFilenames _cursor_filenames;

  static HICON get_icon(const Filename &filename);
  static HCURSOR get_cursor(const Filename &filename);

  // The table of window classes we have registered.  We need to
  // register a different window class for each different window icon
  // (the cursor we can specify dynamically, later).  We might have
  // other requirements too, later.
  class WindowClass {
  public:
    INLINE WindowClass(const WindowProperties &props);
    INLINE bool operator < (const WindowClass &other) const;

    string _name;
    HICON _icon;
  };

  typedef pset<WindowClass> WindowClasses;
  static WindowClasses _window_classes;
  static int _window_class_index;

  static const WindowClass &register_window_class(const WindowProperties &props);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "WinGraphicsWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#define PRINT_LAST_ERROR 0
extern EXPCL_PANDAWIN void PrintErrorMessage(DWORD msgID);
extern EXPCL_PANDAWIN void ClearToBlack(HWND hWnd, const WindowProperties &props);
extern EXPCL_PANDAWIN void get_client_rect_screen(HWND hwnd, RECT *view_rect);

#include "winGraphicsWindow.I"

#endif
