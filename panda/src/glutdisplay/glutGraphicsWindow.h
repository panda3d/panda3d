// Filename: glutGraphicsWindow.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef GLUTGRAPHICSWINDOW_H
#define GLUTGRAPHICSWINDOW_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <graphicsWindow.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class glutGraphicsPipe;

////////////////////////////////////////////////////////////////////
//       Class : glutGraphicsWindow
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGLUT glutGraphicsWindow : public GraphicsWindow
{
public:
  glutGraphicsWindow( GraphicsPipe* pipe );
  glutGraphicsWindow( GraphicsPipe* pipe,
		      const GraphicsWindow::Properties& props );
  virtual ~glutGraphicsWindow( void );

  virtual void config( void );
  virtual void end_frame( void );

public:
  // context setting
  virtual void make_current(void);
  virtual void unmake_current(void);

protected:
  // Window routines
  static void glut_handle_reshape( int w, int h ) {
    _global_window->handle_reshape( w, h );
  }
  void handle_reshape( int w, int h );

  // Mouse routines
  static void glut_handle_mouse_input( int button, int state, int x,
				       int y ) {
    _global_window->handle_mouse_input( button, state, x, y );
  }
  void handle_mouse_input( int button, int state, int x, int y );
  static void glut_handle_mouse_motion( int x, int y ) {
    _global_window->handle_mouse_motion( x, y );
  }
  void handle_mouse_motion( int x, int y );
  static void glut_handle_mouse_entry( int state ) {
    _global_window->handle_mouse_entry( state );
  }
  void handle_mouse_entry( int state );

  // Keyboard routines
  static void glut_handle_ascii_keyboard_input( uchar key, int x, int y ) {
    _global_window->handle_ascii_keyboard_input( key, x, y );
  }
  void handle_ascii_keyboard_input( uchar key, int, int );
  static void glut_handle_special_keyboard_input( int key, int x, int y ) {
    _global_window->handle_special_keyboard_input( key, x, y );
  }
  void handle_special_keyboard_input( int key, int, int );

protected:
  static glutGraphicsWindow*	_global_window;
  int                             _handle;

public:
  virtual void flag_redisplay(void);
  virtual void register_draw_function(GraphicsWindow::vfn);
  virtual void register_idle_function(GraphicsWindow::vfn);
  virtual void register_resize_function(GraphicsWindow::vfnii);
  virtual void main_loop(void);

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_GlutGraphicsWindow(const FactoryParams &params);

public:
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
