// Filename: wdxGraphicsWindow.h
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
#ifndef WDXGRAPHICSWINDOW_H
#define WDXGRAPHICSWINDOW_H
//#define WBD_GL_MODE 1    // if setting this, do it in wdxGraphicsStateGuardian.h too
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsWindow.h>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#include <d3d.h>


////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class wdxGraphicsPipe;

const int WDXWIN_CONFIGURE =    4;
const int WDXWIN_EVENT =    8;

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsWindow : public GraphicsWindow {

friend class DXGraphicsStateGuardian;

public:
  wdxGraphicsWindow(GraphicsPipe* pipe);
  wdxGraphicsWindow(GraphicsPipe* pipe,
             const GraphicsWindow::Properties& props);
  virtual ~wdxGraphicsWindow(void);

  virtual bool supports_update() const;
  virtual void update(void);
  virtual void end_frame( void );

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_wdxGraphicsWindow(const FactoryParams &params);

  LONG window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  void process_events(void);

  INLINE bool mouse_entry_enabled(void) { return _mouse_entry_enabled; }
  INLINE bool mouse_motion_enabled(void) { return _mouse_motion_enabled; }
  INLINE bool mouse_passive_motion_enabled(void) {
    return _mouse_passive_motion_enabled;
  }
  void handle_window_move( int x, int y );
  void handle_mouse_motion( int x, int y );
  void handle_mouse_entry( int state, HCURSOR hMouseCursor );
  void handle_keypress( ButtonHandle key, int x, int y );
  void handle_keyrelease( ButtonHandle key, int x, int y );
  void dx_setup();
  virtual void begin_frame( void );
  void show_frame();
  DXGraphicsStateGuardian *_dxgsg;

protected:
  ButtonHandle lookup_key(WPARAM wparam) const;
  virtual void config( void );
  void setup_colormap(void);

  void enable_mouse_input(bool val);
  void enable_mouse_motion(bool val);
  void enable_mouse_passive_motion(bool val);
  void enable_mouse_entry(bool val);

public:
  HWND              _mwindow;
  HWND              _hOldForegroundWindow;  

private:
  HDC               _hdc;
  HPALETTE          _colormap;
  typedef enum { NotAdjusting,MovingOrResizing,Resizing } WindowAdjustType;
  WindowAdjustType _WindowAdjustingType;
  HCURSOR _hMouseCursor;
  bool    _bSizeIsMaximized;
  bool              _mouse_input_enabled;
  bool              _mouse_motion_enabled;
  bool              _mouse_passive_motion_enabled;
  bool              _mouse_entry_enabled;
  int               _entry_state;
  bool              _ignore_key_repeat;
  bool              _exiting_window;
  bool              _window_inactive;

public:
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  void DestroyMe(bool bAtExitFnCalled);
  virtual void do_close_window();

private:
  static TypeHandle _type_handle;
};

#endif
