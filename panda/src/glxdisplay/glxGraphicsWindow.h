// Filename: glxGraphicsWindow.h
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
#ifndef GLXGRAPHICSWINDOW_H
#define GLXGRAPHICSWINDOW_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsWindow.h>
#include <pStatCollector.h>

#include <X11/Xlib.h>
#include <GL/glx.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class glxGraphicsPipe;

const int GLXWIN_PROCESSING =   1;
const int GLXWIN_REDISPLAY =    2;
const int GLXWIN_CONFIGURE =    4;
const int GLXWIN_EVENT =        8;

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class glxGraphicsWindow : public GraphicsWindow
{
public:
  glxGraphicsWindow( GraphicsPipe* pipe );
  glxGraphicsWindow( GraphicsPipe* pipe,
                     const GraphicsWindow::Properties& props );
  virtual ~glxGraphicsWindow( void );

  virtual bool supports_update() const;
  virtual void update(void);
  virtual void end_frame( void );

  virtual void swap(void);

  INLINE Window get_xwindow(void) { return _xwindow; }

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_GlxGraphicsWindow(const FactoryParams &params);

public:
  virtual void make_current(void);
  virtual void unmake_current(void);

protected:
  void choose_visual(void);
  virtual void config( void );
  void setup_colormap(void);
  void setup_properties(void);

  void enable_mouse_input(bool val);
  void enable_mouse_motion(bool val);
  void enable_mouse_passive_motion(bool val);
  void enable_mouse_entry(bool val);

  void handle_reshape( int w, int h );
  void handle_mouse_motion( int x, int y );
  void handle_mouse_entry( int state );
  void handle_keypress( ButtonHandle key, int x, int y );
  void handle_keyrelease( ButtonHandle key, int x, int y );
  ButtonHandle lookup_key(XEvent event);

  int interruptible_xnextevent(Display* display, XEvent* event);
  bool glx_supports(const char* extension);

  void handle_changes(void);
  void process_event(XEvent);
  void process_events(void);
  void idle_wait(void);

  INLINE void add_event_mask(long event_mask);
  INLINE void remove_event_mask(long event_mask);

private:
  Display*                      _display;
  Window                                _xwindow;
  GLXContext                    _context;
  XVisualInfo*                  _visual;
  Colormap                      _colormap;
  int                           _connection_fd;
  uint                          _change_mask;
  int                           _configure_mask;
  long                          _event_mask;

  static const char*            _glx_extensions;
  bool                          _mouse_input_enabled;
  bool                          _mouse_motion_enabled;
  bool                          _mouse_passive_motion_enabled;
  bool                          _mouse_entry_enabled;
  int                           _entry_state;
  bool                          _ignore_key_repeat;

  // fps meter stuff
  double _start_time;
  long _start_frame_count;
  long _cur_frame_count;
  float _current_fps;

public:
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsWindow.I"

#endif
