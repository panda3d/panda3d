// Filename: wglGraphicsWindow.h
// Created by:  mike (09Jan97)
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
#ifndef WGLGRAPHICSWINDOW_H
#define WGLGRAPHICSWINDOW_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

// include win32 defns for everything up to XP, and assume I'm smart enough to
// use GetProcAddress for backward compat on w95/w98 for newer fns
#define _WIN32_WINNT 0x0501

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

#include <pandabase.h>
#include <graphicsWindow.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class wglGraphicsPipe;

#if 0
#define GLX_USE_GL              1       /* support GLX rendering */
#define GLX_BUFFER_SIZE         2       /* depth of the color buffer */
#define GLX_LEVEL               3       /* level in plane stacking */
#define GLX_RGBA                4       /* true if RGBA mode */
#define GLX_DOUBLEBUFFER        5       /* double buffering supported */
#define GLX_STEREO              6       /* stereo buffering supported */
#define GLX_AUX_BUFFERS         7       /* number of aux buffers */
#define GLX_RED_SIZE            8       /* number of red component bits */
#define GLX_GREEN_SIZE          9       /* number of green component bits */
#define GLX_BLUE_SIZE           10      /* number of blue component bits */
#define GLX_ALPHA_SIZE          11      /* number of alpha component bits */
#define GLX_DEPTH_SIZE          12      /* number of depth bits */
#define GLX_STENCIL_SIZE        13      /* number of stencil bits */
#define GLX_ACCUM_RED_SIZE      14      /* number of red accum bits */
#define GLX_ACCUM_GREEN_SIZE    15      /* number of green accum bits */
#define GLX_ACCUM_BLUE_SIZE     16      /* number of blue accum bits */
#define GLX_ACCUM_ALPHA_SIZE    17      /* number of alpha accum bits */
#endif

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL wglGraphicsWindow : public GraphicsWindow {
public:
  wglGraphicsWindow(GraphicsPipe* pipe);
  wglGraphicsWindow(GraphicsPipe* pipe,
             const GraphicsWindow::Properties& props);
  virtual ~wglGraphicsWindow();

  virtual bool supports_update() const;
  virtual void update();
  virtual void end_frame();
  virtual void swap();
  virtual int get_depth_bitwidth();

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_wglGraphicsWindow(const FactoryParams &params);

public:
  virtual void make_current();
  virtual void unmake_current();
  void handle_mouse_motion(int x, int y);
  void handle_mouse_exit(void);
  INLINE void track_mouse_leaving(HWND hwnd);
  void handle_keypress(ButtonHandle key, int x, int y);
  void handle_keyrelease(ButtonHandle key);

protected:
  int choose_visual(void);
  int find_pixfmtnum(bool bLookforHW);
  virtual void config();
  void setup_colormap(void);
  void handle_reshape();
  void process_events();

public:
  HWND              _mwindow;

private:
  HGLRC             _context;
  HDC               _hdc;
  PIXELFORMATDESCRIPTOR  _pixelformat;
  HPALETTE          _colormap;
  HCURSOR           _hMouseCursor;
  HWND              _hOldForegroundWindow;
  UINT_PTR          _PandaPausedTimer;

  DEVMODE           *_pCurrent_display_settings;
  bool              _bIsLowVidMemCard;
  bool              _bLoadedCustomCursor;

  bool              _window_inactive;
  bool              _active_minimized_fullscreen;
  bool              _return_control_to_app;
  bool              _exiting_window;
  bool              _cursor_in_windowclientarea;
  bool              _tracking_mouse_leaving;
  bool              _ime_open;
  bool              _ime_active;
  bool              _ime_composition_w;

  // vars for frames/sec meter
  DWORD _start_time;
  DWORD _start_frame_count;
  DWORD _cur_frame_count;
  float _current_fps;

  string _extensions_str;

  typedef BOOL (WINAPI* PFN_TRACKMOUSEEVENT)(LPTRACKMOUSEEVENT);
  PFN_TRACKMOUSEEVENT _pfnTrackMouseEvent;

public:
  static TypeHandle get_class_type();
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  LONG WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  ButtonHandle lookup_key(WPARAM wparam) const;
  void DestroyMe(bool bAtExitFnCalled);
  virtual void deactivate_window();
  virtual void reactivate_window();

  virtual bool resize(unsigned int xsize,unsigned int ysize);
  virtual unsigned int verify_window_sizes(unsigned int numsizes,unsigned int *dimen);

protected:
  virtual void do_close_window();
  void check_for_color_cursor_support();

private:
  static TypeHandle _type_handle;
};

extern void set_global_parameters();
extern void restore_global_parameters();

#endif
