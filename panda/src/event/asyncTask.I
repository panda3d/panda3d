// Filename: asyncTask.I
// Created by:  drose (23Aug06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE AsyncTask::
AsyncTask() : 
  _state(S_inactive),
  _manager(NULL)
{
#ifdef HAVE_PYTHON
  _python_object = NULL;
#endif  // HAVE_PYTHON
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_state
//       Access: Published
//  Description: Returns the current state of the task.
////////////////////////////////////////////////////////////////////
INLINE AsyncTask::State AsyncTask::
get_state() const {
  return _state;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_done_event
//       Access: Published
//  Description: Sets the event name that will be triggered
//               when the task finishes.  This should only be called
//               before the task has been started, or after it has
//               finished and before it is about to be restarted
//               (i.e. when get_state() returns S_inactive).
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
set_done_event(const string &done_event) {
  nassertv(_state == S_inactive);
  _done_event = done_event;
}

////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_done_event
//       Access: Published
//  Description: Returns the event name that will be triggered
//               when the task finishes.  See set_done_event().
////////////////////////////////////////////////////////////////////
INLINE const string &AsyncTask::
get_done_event() const {
  return _done_event;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::set_python_object
//       Access: Published
//  Description: Specifies an arbitrary Python object that will be
//               piggybacked on the task object.
////////////////////////////////////////////////////////////////////
INLINE void AsyncTask::
set_python_object(PyObject *python_object) {
  Py_XINCREF(python_object);
  Py_XDECREF(_python_object);
  _python_object = python_object;
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: AsyncTask::get_python_object
//       Access: Published
//  Description: Returns the Python object that was specified to
//               set_python_object(), if any, or None if no object was
//               specified.
////////////////////////////////////////////////////////////////////
INLINE PyObject *AsyncTask::
get_python_object() const {
  if (_python_object != (PyObject *)NULL) {
    Py_XINCREF(_python_object);
    return _python_object;
  }
  Py_INCREF(Py_None);
  return Py_None;
}
#endif  // HAVE_PYTHON

