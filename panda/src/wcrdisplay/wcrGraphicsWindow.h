// Filename: wcrGraphicsWindow.h
// Created by:  skyler, based on wgl* file.
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
#ifndef WCRGRAPHICSWINDOW_H
#define WCRGRAPHICSWINDOW_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include "graphicsWindow.h"
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class wcrGraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : wcrGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDACR wcrGraphicsWindow : public GraphicsWindow {
public:
  wcrGraphicsWindow(GraphicsPipe* pipe);
  wcrGraphicsWindow(GraphicsPipe* pipe,
             const GraphicsWindow::Properties& props);
  virtual ~wcrGraphicsWindow();

  virtual bool supports_update() const;
  virtual void update();
  virtual void end_frame();
  virtual void swap();
  virtual int get_depth_bitwidth();

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_wcrGraphicsWindow(const FactoryParams &params);

public:
  virtual void make_current();
  virtual void unmake_current();

  INLINE bool mouse_entry_enabled() { return _mouse_entry_enabled; }
  INLINE bool mouse_motion_enabled() { return _mouse_motion_enabled; }
  INLINE bool mouse_passive_motion_enabled() {
    return _mouse_passive_motion_enabled;
  }
//  void handle_reshape(int w, int h);

  void handle_mouse_motion(int x, int y);
  void handle_mouse_entry(int state);
  void handle_keypress(ButtonHandle key, int x, int y);
  void handle_keyrelease(ButtonHandle key);

protected:
//  PIXELFORMATDESCRIPTOR* try_for_visual(wcrGraphicsPipe *pipe,
//                              int mask, int want_depth_bits = 1, int want_color_bits = 1);
//  static void get_config(PIXELFORMATDESCRIPTOR* visual, int attrib, int *value);
  int choose_visual();
  virtual void config();
  void setup_colormap();

  void enable_mouse_input(bool val);
  void enable_mouse_motion(bool val);
  void enable_mouse_passive_motion(bool val);
  void enable_mouse_entry(bool val);

  void handle_reshape();
  void process_events();

public:
  HWND   _mwindow;

private:
  // TODO:skyler HGLRC  _context;
  int  _context;
  HDC    _hdc;
  PIXELFORMATDESCRIPTOR  _pixelformat;
  HPALETTE _colormap;
  HCURSOR  _hMouseCursor;
  HWND     _hOldForegroundWindow;
  UINT_PTR _PandaPausedTimer;

  DEVMODE *_pCurrent_display_settings;
  bool    _bIsLowVidMemCard;
  bool    _bLoadedCustomCursor;

  bool    _window_inactive;
  bool    _active_minimized_fullscreen;
  bool    _return_control_to_app;
  bool    _exiting_window;

  bool    _mouse_input_enabled;
  bool    _mouse_motion_enabled;
  bool    _mouse_passive_motion_enabled;
  bool    _mouse_entry_enabled;
  bool    _ime_open;

  // vars for frames/sec meter
  DWORD _start_time;
  DWORD _start_frame_count;
  DWORD _cur_frame_count;
  float _current_fps;

  string _extensions_str;

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

#endif
