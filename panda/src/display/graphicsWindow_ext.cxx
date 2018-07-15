/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsWindow_ext.cxx
 * @author CFSworks
 * @date 2014-10-11
 */

#include "graphicsWindow_ext.h"

#ifdef HAVE_PYTHON

/**
 * Adds a python event handler to be called when a window event occurs.
 */
void Extension<GraphicsWindow>::
add_python_event_handler(PyObject* handler, PyObject* name){
  PythonGraphicsWindowProc* pgwp = new PythonGraphicsWindowProc(handler, name);
  _this->_python_window_proc_classes.insert(pgwp);
  _this->add_window_proc(pgwp);
}

/**
 * Removes the specified python event handler.
 */
void Extension<GraphicsWindow>::
remove_python_event_handler(PyObject* name){
  std::list<PythonGraphicsWindowProc*> toRemove;
  GraphicsWindow::PythonWinProcClasses::iterator iter;
  for (iter = _this->_python_window_proc_classes.begin(); iter != _this->_python_window_proc_classes.end(); ++iter) {
    PythonGraphicsWindowProc* pgwp = (PythonGraphicsWindowProc*)*iter;
    if (PyObject_RichCompareBool(pgwp->get_name(), name, Py_EQ) == 1) {
      toRemove.push_back(pgwp);
    }
#if PY_MAJOR_VERSION < 3
    else if (PyObject_Compare(pgwp->get_name(), name) == 0) {
      toRemove.push_back(pgwp);
    }
#endif
  }
  std::list<PythonGraphicsWindowProc*>::iterator iter2;
  for (iter2 = toRemove.begin(); iter2 != toRemove.end(); ++iter2) {
    PythonGraphicsWindowProc* pgwp = *iter2;
    _this->remove_window_proc(pgwp);
    _this->_python_window_proc_classes.erase(pgwp);
    delete pgwp;
  }
}

#endif  // HAVE_PYTHON
