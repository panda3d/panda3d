// Filename: glxDisplay.h
// Created by:  drose (30Oct00)
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

#ifndef GLXDISPLAY_H
#define GLXDISPLAY_H

#include <pandabase.h>
#include <typedObject.h>

#include <X11/Xlib.h>

class GraphicsPipe;
class glxGraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : glxDisplay
// Description : This class is a base class of glxGraphicsPipe, and
//               also of SgiGlxGraphicsPipe and potentially other
//               glx-based pipes.  It simply records some information
//               about the display that's useful to the
//               glxGraphicsWindow.
////////////////////////////////////////////////////////////////////
class glxDisplay : public TypedObject {
public:
  glxDisplay(GraphicsPipe *pipe, const string &x_specifier);

  INLINE Display *get_display() const;
  INLINE int get_screen() const;
  INLINE Window get_root() const;
  INLINE int get_display_width() const;
  INLINE int get_display_height() const;

  glxGraphicsWindow *find_window(Window win) const;

private:
  GraphicsPipe *_pipe;
  Display *_display;
  int _screen;
  Window _root;
  int _width;
  int _height;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "glxDisplay",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxDisplay.I"

#endif
