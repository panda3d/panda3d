/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonGraphicsWindowProc.h
 * @author Walt Destler
 * @date 2010-05
 */

#ifndef PYTHONGRAPHICSWINDOWPROC_H
#define PYTHONGRAPHICSWINDOWPROC_H

#include "pandabase.h"
#include "graphicsWindowProc.h"
#include "pythonCallbackObject.h"

#ifdef HAVE_PYTHON

/**
 * Extends GraphicsWindowProc to provde callback functionality to a python
 * program.
 */
class PythonGraphicsWindowProc: public GraphicsWindowProc,
                                public PythonCallbackObject {
public:
  PythonGraphicsWindowProc(PyObject *function, PyObject* name);
  virtual ~PythonGraphicsWindowProc();
  ALLOC_DELETED_CHAIN(PythonGraphicsWindowProc);

#ifdef WIN32
  virtual LONG wnd_proc(GraphicsWindow* graphicsWindow, HWND hwnd,
                        UINT msg, WPARAM wparam, LPARAM lparam);
#endif

  PyObject *get_name();

private:
  PyObject *_name;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "PythonGraphicsWindowProc",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_PYTHON

#endif  // PYTHONGRAPHICSWINDOWPROC_H
