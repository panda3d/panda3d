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

#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)data, size);
#else
  return PyString_FromStringAndSize((char *)data, size);
#endif
}

/**
 * Returns the datagram's data as a bytes object.
 */
PyObject *Extension<Datagram>::
__bytes__() const {
  return get_message();
}
