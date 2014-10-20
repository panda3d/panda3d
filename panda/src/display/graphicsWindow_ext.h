// Filename: renderState_ext.h
// Created by:  CFSworks (11Oct14)
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

#ifndef GRAPHICSWINDOW_EXT_H
#define GRAPHICSWINDOW_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "graphicsWindow.h"
#include "pythonGraphicsWindowProc.h"
#include "py_panda.h"

////////////////////////////////////////////////////////////////////
//       Class : Extension<GraphicsWindow>
// Description : This class defines the extension methods for
//               GraphicsWindow, which are called instead of
//               any C++ methods with the same prototype.
////////////////////////////////////////////////////////////////////
template<>
class Extension<GraphicsWindow> : public ExtensionBase<GraphicsWindow> {
public:
  void add_python_event_handler(PyObject* handler, PyObject* name);
  void remove_python_event_handler(PyObject* name);
};

#endif  // HAVE_PYTHON

#endif  // GRAPHICSWINDOW_EXT_H
