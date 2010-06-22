// Filename: customGraphicsWindowProc.cxx
// Created by:  Walt Destler (May 2010)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "customGraphicsWindowProc.h"

CustomGraphicsWindowProc::CustomGraphicsWindowProc(PyObject* handler, PyObject* name){
  _handler = handler;
  _name = name;
  Py_INCREF(_handler);
  Py_INCREF(_name);
}

CustomGraphicsWindowProc::~CustomGraphicsWindowProc(){
  Py_DECREF(_name);
  Py_DECREF(_handler);
}

#ifdef WIN32
LONG CustomGraphicsWindowProc::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
  PyObject* ret = PyObject_CallFunction(_handler, "IIII", hwnd, msg, wparam, lparam);
  Py_XDECREF(ret);

  return 0;
}
#endif

PyObject* CustomGraphicsWindowProc::get_handler(){
  return _handler;
}

PyObject* CustomGraphicsWindowProc::get_name(){
  return _name;
}
