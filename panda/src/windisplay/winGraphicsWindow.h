/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winGraphicsWindow.h
 * @author drose
 * @date 2002-12-20
 */

#ifndef WINGRAPHICSWINDOW_H
#define WINGRAPHICSWINDOW_H

#include "pandabase.h"
#include "graphicsWindow.h"
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

class WinGraphicsPipe;

#define PM_ACTIVE   (WM_APP+123)

#define PM_INACTIVE  (WM_APP+124)

#define MAX_TOUCHES 20

typedef struct {
  int x;
  int y;
  int width;
  int height;
} WINDOW_METRICS;

#if WINVER < 0x0601
// Not used on Windows XP, but we still need to define it.
typedef struct tagTOUCHINPUT {
  LONG x;
  LONG y;
  HANDLE hSource;
  DWORD dwID;
  DWORD dwFlags;
  DWORD dwMask;
  DWORD dwTime;
  ULONG_PTR dwExtraInfo;
  DWORD cxContact;
  DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
#endif

/**
 * An abstract base class for glGraphicsWindow and dxGraphicsWindow (and, in
 * general, graphics windows that interface with the Microsoft Windows API).
 *
 * This class includes all the code for manipulating windows themselves:
 * opening them, closing them, responding to user keyboard and mouse input,
 * and so on.  It does not make any 3-D rendering calls into the window; that
 * is the province of the GraphicsStateGuardian.
 */
class EXPCL_PANDAWIN WinGraphicsWindow : public GraphicsWindow {
public:
  WinGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~WinGraphicsWindow();

  virtual MouseData get_pointer(int device) const;
  virtual bool move_pointer(int device, int x, int y);

  virtual void close_ime();

  virtual void begin_flip();

  virtual void process_events();
  virtual void set_properties_now(WindowProperties &properties);
  void receive_windows_message(unsigned int msg, int wparam, int lparam);
  virtual LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  virtual bool handle_mouse_motion(int x, int y);
  virtual void handle_mouse_exit();

  INLINE HWND get_ime_hwnd();

  virtual void add_window_proc( const GraphicsWindowProc* wnd_proc_object );
  virtual void remove_window_proc( const GraphicsWindowProc* wnd_proc_object );
  virtual void clear_window_procs();
  virtual bool supports_window_procs() const;

  virtual bool is_touch_event(GraphicsWindowProcCallbackData* callbackData);
  virtual int get_num_touches();
  virtual TouchInfo get_touch_info(int index);

protected:
  void trigger_flip();
  virtual void close_window();
  virtual bool open_window();
  virtual void fullscreen_minimized(WindowProperties &properties);
  virtual void fullscreen_restored(WindowProperties &properties);

  virtual bool do_reshape_request(int x_origin, int y_origin, bool has_origin,
                                  int x_size, int y_size);

  virtual void handle_reshape();
  virtual bool do_fullscreen_resize(int x_size, int y_size);

  virtual bool do_fullscreen_switch();
  virtual bool do_windowed_switch();
  virtual bool do_fullscreen_enable();
  virtual bool do_fullscreen_disable();

  virtual bool calculate_metrics(bool fullscreen, DWORD style,
                                 WINDOW_METRICS &metrics, bool &has_origin);

  DWORD make_style(const WindowProperties &properties);

  virtual void reconsider_fullscreen_size(DWORD &x_size, DWORD &y_size,
                                          DWORD &bitdepth);

  virtual void support_overlay_window(bool flag);

private:
  bool open_graphic_window();
  void adjust_z_order();
  void adjust_z_order(WindowProperties::ZOrder last_z_order,
                      WindowProperties::ZOrder this_z_order);
  void initialize_input_devices();
  void handle_raw_input(HRAWINPUT hraw);
  void track_mouse_leaving(HWND hwnd);
  bool confine_cursor();
  void set_focus();

  static void process_1_event();

  void handle_keypress(ButtonHandle key, int x, int y, double time);
  void handle_keyresume(ButtonHandle key, double time);
  void handle_keyrelease(ButtonHandle key, double time);
  void handle_raw_keypress(ButtonHandle key, double time);
  void handle_raw_keyrelease(ButtonHandle key, double time);
  ButtonHandle lookup_key(WPARAM wparam) const;
  ButtonHandle lookup_raw_key(LPARAM lparam) const;
  virtual ButtonMap *get_keyboard_map() const;
  INLINE int translate_mouse(int pos) const;
  INLINE void set_cursor_in_window();
  INLINE void set_cursor_out_of_window();

  INLINE static double get_message_time();

  void resend_lost_keypresses();
  void release_all_buttons();
  static void update_cursor_window(WinGraphicsWindow *to_window);
  static void hide_or_show_cursor(bool hide_cursor);

  static bool find_acceptable_display_mode(DWORD dwWidth, DWORD dwHeight,
                                           DWORD bpp, DEVMODE &dm);
  static void show_error_message(DWORD message_id = 0);

protected:
  HWND _hWnd;
  HWND _hparent;

private:
  HWND _ime_hWnd;
  bool _ime_open;
  bool _ime_active;
  bool _tracking_mouse_leaving;
  bool _bCursor_in_WindowClientArea;
  HANDLE _input_device_handle[32];
  HCURSOR _cursor;
  DEVMODE _fullscreen_display_mode;

  bool _lost_keypresses;

  // These are used to store the status of the individual left and right
  // shift, control, and alt keys.  Keyboard events are not sent for these
  // individual keys, but for each pair as a whole.  The status of each key
  // must be checked as keypress and keyrelease events are received.
  bool _lshift_down;
  bool _rshift_down;
  bool _lcontrol_down;
  bool _rcontrol_down;
  bool _lalt_down;
  bool _ralt_down;

  GraphicsWindowInputDevice *_input;

  // following adds support platform specfic window processing functions.
  typedef pset<GraphicsWindowProc*> WinProcClasses;
  WinProcClasses _window_proc_classes;

  UINT _num_touches;
  TOUCHINPUT _touches[MAX_TOUCHES];

private:
  // We need this map to support per-window calls to window_proc().
  typedef std::map<HWND, WinGraphicsWindow *> WindowHandles;
  static WindowHandles _window_handles;

  // And we need a static pointer to the current WinGraphicsWindow we are
  // creating at the moment, since CreateWindow() starts generating window
  // events before it gives us the window handle.
  static WinGraphicsWindow *_creating_window;

/*
 * This tracks the current GraphicsWindow whose client area contains the
 * mouse.  There will only be one of these at a time, and storing the pointer
 * here allows us to handle ambiguities in the order in which messages are
 * passed from Windows to the various windows we manage.  This pointer is used
 * by set_cursor_in_window() to determine when it is time to call
 * update_cursor() to hide the cursor (or do other related operations).
 */
  static WinGraphicsWindow *_cursor_window;
  static bool _cursor_hidden;
  static bool _got_saved_params;
  static int _saved_mouse_trails;
  static BOOL _saved_cursor_shadow;
  static BOOL _saved_mouse_vanish;

  // The mouse constraints before applying mouse mode M_confined.
  static RECT _mouse_unconfined_cliprect;

  // Since the Panda API requests icons and cursors by filename, we need a
  // table mapping filenames to handles, so we can avoid re-reading the file
  // each time we change icons.
  typedef pmap<Filename, HANDLE> IconFilenames;
  static IconFilenames _icon_filenames;
  static IconFilenames _cursor_filenames;

  static HICON get_icon(const Filename &filename);
  static HCURSOR get_cursor(const Filename &filename);

  // The table of window classes we have registered.  We need to register a
  // different window class for each different window icon (the cursor we can
  // specify dynamically, later).  We might have other requirements too,
  // later.
  class WindowClass {
  public:
    INLINE WindowClass(const WindowProperties &props);
    INLINE bool operator < (const WindowClass &other) const;

    std::wstring _name;
    HICON _icon;
  };

  typedef pset<WindowClass> WindowClasses;
  static WindowClasses _window_classes;
  static int _window_class_index;

  static const WindowClass &register_window_class(const WindowProperties &props);
private:
  // This subclass of WindowHandle is stored in _window_handle to represent
  // this particular window.  We use it to add hooks for communicating with
  // the parent window, in particular to receive keyboard events from the
  // parent window when necessary.
  class WinWindowHandle : public WindowHandle {
  public:
    WinWindowHandle(WinGraphicsWindow *window,
                    const WindowHandle &copy);
    void clear_window();

  protected:
    virtual void receive_windows_message(unsigned int msg, int wparam, int lparam);

  private:
    // Not reference-counted, to avoid a circular reference count.
    WinGraphicsWindow *_window;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      WindowHandle::init_type();
      register_type(_type_handle, "WinWindowHandle",
                    WindowHandle::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "WinGraphicsWindow",
                  GraphicsWindow::get_class_type());
    WinWindowHandle::init_type();
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
