// Filename: glxGraphicsPipe.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "glxGraphicsPipe.h"
#include "config_glxdisplay.h"

#include <GL/glx.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle glxGraphicsPipe::_type_handle;

glxGraphicsPipe::glxGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec)
{
  // _display = XOpenDisplay(get_name().c_str());
  _display = XOpenDisplay((spec.get_X_specifier()).c_str());
  if (!_display) {
    glxdisplay_cat.fatal()
      << "glxGraphicsPipe::construct(): Could not open display: "
      << spec.get_X_specifier() << endl;
    exit(0);
  }
  int errorBase, eventBase;
  if (!glXQueryExtension(_display, &errorBase, &eventBase)) {
    glxdisplay_cat.fatal()
      << "glxGraphicsPipe::construct(): OpenGL GLX extension not "
      << "supported by display: " << spec.get_X_specifier() << endl;
    exit(0);
  }
  _screen = DefaultScreen(_display);
  _root = RootWindow(_display, _screen);
  _width = DisplayWidth(_display, _screen);
  _height = DisplayHeight(_display, _screen);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle glxGraphicsPipe::
get_window_type() const {
  return glxGraphicsWindow::get_class_type();
}

GraphicsPipe *glxGraphicsPipe::
make_glxGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new glxGraphicsPipe(PipeSpecifier());
  } else {
    return new glxGraphicsPipe(pipe_param->get_specifier());
  }
}

TypeHandle glxGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void glxGraphicsPipe::init_type(void) {
  InteractiveGraphicsPipe::init_type();
  register_type(_type_handle, "glxGraphicsPipe",
		InteractiveGraphicsPipe::get_class_type());
}

TypeHandle glxGraphicsPipe::get_type(void) const {
  return get_class_type();
}

glxGraphicsPipe::glxGraphicsPipe(void) {
  glxdisplay_cat.error()
    << "glxGraphicsPipes should not be created with the default constructor"
    << endl;
}

glxGraphicsPipe::glxGraphicsPipe(const glxGraphicsPipe&) {
  glxdisplay_cat.error()
    << "glxGraphicsPipes should not be copied" << endl;
}

glxGraphicsPipe& glxGraphicsPipe::operator=(const glxGraphicsPipe&) {
  glxdisplay_cat.error()
    << "glxGraphicsPipes should not be assigned" << endl;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: find_window
//       Access:
//  Description: Find the window that has the xwindow "win" in the
//		 window list for the pipe (if it exists)
////////////////////////////////////////////////////////////////////
glxGraphicsWindow *glxGraphicsPipe::
find_window(Window win) {
  int num_windows = get_num_windows();
  for (int w = 0; w < num_windows; w++) {
    glxGraphicsWindow *window = DCAST(glxGraphicsWindow, get_window(w));
    if (window->get_xwindow() == win)
      return window;
  }
  return NULL;
}
