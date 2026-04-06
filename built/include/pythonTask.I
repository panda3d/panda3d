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
  return Py_NewRef(_function);
}

/**
 * Returns the function that is called when the task finishes.
 */
INLINE PyObject *PythonTask::
get_upon_death() {
  return Py_NewRef(_upon_death);
}

/**
 * Returns the "owner" object.  See set_owner().
 */
INLINE PyObject *PythonTask::
get_owner() const {
  return Py_NewRef(_owner);
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
  Py_XSETREF(_exc_value, Py_NewRef(result));
}
