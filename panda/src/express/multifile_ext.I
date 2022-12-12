/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file multifile_ext.I
 * @author Derzsi Daniel
 * @date 2022-07-20
 */

#include <cstring>

/**
 * Specifies the password, either as a Python string or a Python bytes object,
 * that will be used to encrypt subfiles subsequently added to the multifile
 */
INLINE PyObject *Extension<Multifile>::
set_encryption_password(PyObject *encryption_password) const {
  Py_ssize_t pass_len;

  // Have we been passed a string?
  if (PyUnicode_Check(encryption_password)) {
    const char *pass_str = PyUnicode_AsUTF8AndSize(encryption_password, &pass_len);
    _this->set_encryption_password(std::string(pass_str, pass_len));
    return Dtool_Return_None();
  }

  // Have we been passed a bytes object?
  if (PyBytes_Check(encryption_password)) {
    char *pass_str;

    if (PyBytes_AsStringAndSize(encryption_password, &pass_str, &pass_len) < 0) {
      PyErr_SetString(PyExc_TypeError, "A valid bytes object is required.");
      return NULL;
    }

    // It is dangerous to use null bytes inside the encryption password.
    // OpenSSL will cut off the password prematurely at the first null byte
    // encountered.
    if (memchr(pass_str, '\0', pass_len) != NULL) {
      PyErr_SetString(PyExc_ValueError, "The password must not contain null bytes.");
      return NULL;
    }

    _this->set_encryption_password(std::string(pass_str, pass_len));
    return Dtool_Return_None();
  }

  return Dtool_Raise_BadArgumentsError(
    "set_encryption_password(const Multifile self, str encryption_password)\n"
  );
}

/**
 * Returns the password that will be used to encrypt subfiles subsequently
 * added to the multifile, either as a Python string (when possible) or a
 * Python bytes object.
 */
INLINE PyObject *Extension<Multifile>::
get_encryption_password() const {
  std::string password = _this->get_encryption_password();
  const char *pass_str = password.c_str();
  Py_ssize_t pass_len = password.length();

  // First, attempt to decode it as an UTF-8 string...
  PyObject *result = PyUnicode_DecodeUTF8(pass_str, pass_len, NULL);

  if (PyErr_Occurred()) {
    // This password cannot be decoded as an UTF-8 string, so let's
    // return it as a bytes object.
    PyErr_Clear();
    result = PyBytes_FromStringAndSize(pass_str, pass_len);
  }

  return result;
}
