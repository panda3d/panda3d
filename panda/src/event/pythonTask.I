/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pythonTask.I
 * @author drose
 * @date 2008-09-16
 */

/**
 * Returns the function that is called when the task runs.
 */
INLINE PyObject *PythonTask::
get_function() {
  Py_INCREF(_function);
  return _function;
}

/**
 * Returns the function that is called when the task finishes.
 */
INLINE PyObject *PythonTask::
get_upon_death() {
  Py_INCREF(_upon_death);
  return _upon_death;
}

/**
 * Returns the "owner" object.  See set_owner().
 */
INLINE PyObject *PythonTask::
get_owner() const {
  Py_INCREF(_owner);
  return _owner;
}

/**
 * Sets the "result" of this task.  This is the value returned from an "await"
 * expression on this task.
 * This can only be called while the task is still alive.
 */
INLINE void PythonTask::
set_result(PyObject *result) {
  // Note that we don't call notify_done() here since the done status will be
  // automatically notified upon the task's completion.
  nassertv(is_alive());
  nassertv(!done());
  nassertv(_exception == nullptr);
  Py_INCREF(result);
  Py_XDECREF(_exc_value);
  _exc_value = result;
}
