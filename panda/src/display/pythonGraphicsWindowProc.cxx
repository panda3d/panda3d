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

#include "pythonGraphicsWindowProc.h"
#include "graphicsWindowProcCallbackData.h"

#ifdef HAVE_PYTHON

TypeHandle PythonGraphicsWindowProc::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PythonGraphicWindowProc::Constructor
//       Access: Public
//  Description: Initializes this PythonGraphicsWindowProc to use the
//               specified callback handler and name.
////////////////////////////////////////////////////////////////////
PythonGraphicsWindowProc::
PythonGraphicsWindowProc(PyObject* function, PyObject* name) :
  PythonCallbackObject(function)
{
  _name = name;
  Py_INCREF(_name);
}

////////////////////////////////////////////////////////////////////
//     Function: PythonGraphicWindowProc::Constructor
//       Access: Public, Virtual
//  Description: Decrements references to the handler and name objects.
////////////////////////////////////////////////////////////////////
PythonGraphicsWindowProc::
~PythonGraphicsWindowProc(){
  Py_DECREF(_name);
}

#ifdef WIN32

////////////////////////////////////////////////////////////////////
//     Function: PythonGraphicWindowProc::wnd_proc
//       Access: Public, Virtual
//  Description: A WIN32-specific method that is called when a Window
//               proc event occurrs. Calls the python handler.
////////////////////////////////////////////////////////////////////
LONG PythonGraphicsWindowProc::
wnd_proc(GraphicsWindow* graphicsWindow, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
  GraphicsWindowProcCallbackData cdata(graphicsWindow);
  cdata.set_hwnd((int)hwnd);
  cdata.set_msg(msg);
  cdata.set_wparam(wparam);
  cdata.set_lparam(lparam);
  do_callback(&cdata);

  return 0;
}

#endif // WIN32

////////////////////////////////////////////////////////////////////
//     Function: PythonGraphicWindowProc::get_name
//       Access: Public
//  Description: Returns the python name object.
////////////////////////////////////////////////////////////////////
PyObject* PythonGraphicsWindowProc::
get_name(){
  return _name;
}

#endif  // HAVE_PYTHON
