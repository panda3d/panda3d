// Filename: glutGraphicsWindow.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "glutGraphicsWindow.h"
#include "glutGraphicsPipe.h"
#include "config_glutdisplay.h"

#include <glGraphicsStateGuardian.h>
#include <mouseButton.h>
#include <keyboardButton.h>
#include <pStatTimer.h>

#ifdef PENV_WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#endif
#include <GL/glut.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle glutGraphicsWindow::_type_handle;
glutGraphicsWindow* glutGraphicsWindow::_global_window;

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
glutGraphicsWindow::glutGraphicsWindow( GraphicsPipe* pipe ) : 
	GraphicsWindow( pipe )
{
    _global_window = this;
    config();
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
glutGraphicsWindow::glutGraphicsWindow( GraphicsPipe* pipe, const 
	GraphicsWindow::Properties& props ) : GraphicsWindow( pipe, props )
{
    _global_window = this;
    config();
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
glutGraphicsWindow::~glutGraphicsWindow(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: config 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::config( void )
{
  glutInitDisplayMode( _props._mask );
  glutInitWindowPosition( _props._xorg, _props._yorg );
  glutInitWindowSize( _props._xsize, _props._ysize );
  _handle = glutCreateWindow( _props._title.c_str() );
  glutReshapeFunc( glut_handle_reshape );
  
  // Mouse callbacks
  glutMouseFunc( glut_handle_mouse_input );
  glutMotionFunc( glut_handle_mouse_motion );
  glutPassiveMotionFunc( glut_handle_mouse_motion );
  
  // Keyboard callbacks
  glutKeyboardFunc( glut_handle_ascii_keyboard_input );
  glutSpecialFunc( glut_handle_special_keyboard_input );

  // Indicate that we have our keyboard/mouse device ready.
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);

  // Create a GSG to manage the graphics
  make_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: end_frame 
//       Access:
//  Description: Swaps the front and back buffers.
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::end_frame( void )
{
  {
    PStatTimer timer(_swap_pcollector);
    glutSwapBuffers();
  }
  glutPostRedisplay();
  PStatClient::main_tick();
}

////////////////////////////////////////////////////////////////////
//     Function: handle_reshape 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::handle_reshape( int w, int h )
{
    _props._xsize = w;
    _props._ysize = h;
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_input
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::handle_mouse_input( int button, int state, 
	int x, int y )
{
  _input_devices[0].set_pointer_in_window(x, y);

  ButtonHandle handle;
  switch (button) {
  case GLUT_LEFT_BUTTON:
    handle = MouseButton::one();
    break;

  case GLUT_MIDDLE_BUTTON:
    handle = MouseButton::two();
    break;

  case GLUT_RIGHT_BUTTON:
    handle = MouseButton::three();
    break;
  }

  if (handle != ButtonHandle::none()) {
    if (state == GLUT_DOWN) {
      _input_devices[0].button_down(handle);
    } else {
      _input_devices[0].button_up(handle);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::handle_mouse_motion( int x, int y )
{
  _input_devices[0].set_pointer_in_window(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::handle_mouse_entry( int state )
{
  if (state == GLUT_LEFT) {
    _input_devices[0].set_pointer_out_of_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_ascii_keyboard_input
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::handle_ascii_keyboard_input( uchar key, int x, int y )
{
  _input_devices[0].set_pointer_in_window(x, y);

  ButtonHandle handle = KeyboardButton::ascii_key(key);
  if (handle != ButtonHandle::none()) {
    _input_devices[0].button_down(handle);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_special_keyboard_input
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glutGraphicsWindow::handle_special_keyboard_input( int key, int x, int y )
{
  _input_devices[0].set_pointer_in_window(x, y);

  ButtonHandle handle = ButtonHandle::none();

  switch ( key ) {
  case GLUT_KEY_F1: handle = KeyboardButton::f1(); break;
  case GLUT_KEY_F2: handle = KeyboardButton::f2(); break;
  case GLUT_KEY_F3: handle = KeyboardButton::f3(); break;
  case GLUT_KEY_F4: handle = KeyboardButton::f4(); break;
  case GLUT_KEY_F5: handle = KeyboardButton::f5(); break;
  case GLUT_KEY_F6: handle = KeyboardButton::f6(); break;
  case GLUT_KEY_F7: handle = KeyboardButton::f7(); break;
  case GLUT_KEY_F8: handle = KeyboardButton::f8(); break;
  case GLUT_KEY_F9: handle = KeyboardButton::f9(); break;
  case GLUT_KEY_F10: handle = KeyboardButton::f10(); break;
  case GLUT_KEY_F11: handle = KeyboardButton::f11(); break;
  case GLUT_KEY_F12: handle = KeyboardButton::f12(); break;
  case GLUT_KEY_LEFT: handle = KeyboardButton::left(); break;
  case GLUT_KEY_RIGHT: handle = KeyboardButton::right(); break;
  case GLUT_KEY_UP: handle = KeyboardButton::up(); break;
  case GLUT_KEY_DOWN: handle = KeyboardButton::down(); break;
  case GLUT_KEY_PAGE_UP: handle = KeyboardButton::page_up(); break;
  case GLUT_KEY_PAGE_DOWN: handle = KeyboardButton::page_down(); break;
  case GLUT_KEY_HOME: handle = KeyboardButton::home(); break;
  case GLUT_KEY_END: handle = KeyboardButton::end(); break;
  case GLUT_KEY_INSERT: handle = KeyboardButton::insert(); break;
  }

  if (handle != ButtonHandle::none()) {
    _input_devices[0].button_down(handle);
  }
}

void glutGraphicsWindow::flag_redisplay(void) {
  glutPostRedisplay();
}

void glutGraphicsWindow::register_draw_function(GraphicsWindow::vfn f) {
  GraphicsWindow::register_draw_function(f);
  glutDisplayFunc(f);
}

void glutGraphicsWindow::register_idle_function(GraphicsWindow::vfn f) {
  GraphicsWindow::register_idle_function(f);
  glutIdleFunc(f);
}

void glutGraphicsWindow::register_resize_function(GraphicsWindow::vfnii f) {
  GraphicsWindow::register_resize_function(f);
  glutReshapeFunc(f);
}

void glutGraphicsWindow::main_loop(void) {
  glutMainLoop();
}

void glutGraphicsWindow::make_current(void) {
  if (_global_window != this) {
    PStatTimer timer(_make_current_pcollector);
    glutSetWindow(_handle);
    _global_window = this;
  }
}

void glutGraphicsWindow::unmake_current(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: glutGraphicsWindow::get_gsg_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of GSG preferred
//               by this kind of window.
////////////////////////////////////////////////////////////////////
TypeHandle glutGraphicsWindow::
get_gsg_type() const {
  return GLGraphicsStateGuardian::get_class_type();
}

GraphicsWindow *glutGraphicsWindow::
make_GlutGraphicsWindow(const FactoryParams &params) {
  GraphicsWindow::WindowPipe *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    glutdisplay_cat.error()
      << "No pipe specified for window creation!" << endl;
    return NULL;
  }

  GraphicsPipe *pipe = pipe_param->get_pipe();
  
  GraphicsWindow::WindowProps *props_param;
  if (!get_param_into(props_param, params)) {
    return new glutGraphicsWindow(pipe);
  } else {
    return new glutGraphicsWindow(pipe, props_param->get_properties());
  }
}

TypeHandle glutGraphicsWindow::get_class_type(void) {
  return _type_handle;
}

void glutGraphicsWindow::init_type(void) {
  GraphicsWindow::init_type();
  register_type(_type_handle, "glutGraphicsWindow",
		GraphicsWindow::get_class_type());
}

TypeHandle glutGraphicsWindow::get_type(void) const {
  return get_class_type();
}
