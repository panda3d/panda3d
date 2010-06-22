// Filename: customGgraphicswindowProc.h
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

#ifndef CUSTOMGRAPHICSWINDOWPROC_H
#define CUSTOMGRAPHICSWINDOWPROC_H

#include "pandabase.h"
#include "graphicsWindowProc.h"

class CustomGraphicsWindowProc: public GraphicsWindowProc{
public:
  CustomGraphicsWindowProc(PyObject* handler, PyObject* name);
  virtual ~CustomGraphicsWindowProc();

#if defined(__WIN32__) || defined(_WIN32)
  virtual LONG wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif

  PyObject* get_handler();
  PyObject* get_name();

private:
  PyObject* _handler;
  PyObject* _name;
};

#endif //CUSTOMGRAPHICSWINDOWPROC_H
