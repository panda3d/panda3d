/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagram_ext.I
 * @author rdb
 * @date 2018-08-19
 */

/**
 * Returns the datagram's data as a bytes object.
 */
INLINE PyObject *Extension<Datagram>::
get_message() const {
  const char *data = (const char *)_this->get_data();
  size_t size = _this->get_length();
  return PyBytes_FromStringAndSize((char *)data, size);
}

/**
 * Returns the datagram's data as a bytes object.
 */
PyObject *Extension<Datagram>::
__bytes__() const {
  return get_message();
}

/**
 * This special Python method is implemented to provide support for the pickle
 * module.
 */
INLINE PyObject *Extension<Datagram>::
__reduce__() const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.
  PyObject *args;
  if (_this->get_length() > 0) {
    args = PyTuple_New(1);
    PyTuple_SET_ITEM(args, 0, get_message());
  } else {
    args = PyTuple_New(0);
  }

  extern struct Dtool_PyTypedObject Dtool_Datagram;
  PyObject *tp = (PyObject *)Dtool_GetPyTypeObject(&Dtool_Datagram);

  PyObject *result = PyTuple_New(2);
  PyTuple_SET_ITEM(result, 0, Py_NewRef(tp));
  PyTuple_SET_ITEM(result, 1, args);
  return result;
}
