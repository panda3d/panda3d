/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textEncoder_ext.cxx
 * @author rdb
 * @date 2018-09-29
 */

#include "textEncoder_ext.h"

#ifdef HAVE_PYTHON

/**
 * Sets the text as a Unicode string.  In Python 2, if a regular str is given,
 * it is assumed to be in the TextEncoder's specified encoding.
 */
void Extension<TextEncoder>::
set_text(PyObject *text) {
  if (PyUnicode_Check(text)) {
    Py_ssize_t len;
    const char *str = PyUnicode_AsUTF8AndSize(text, &len);
    _this->set_text(std::string(str, len), TextEncoder::E_utf8);
  } else {
    Dtool_Raise_TypeError("expected string");
  }
}

/**
 * Sets the text as an encoded byte string of the given encoding.
 */
void Extension<TextEncoder>::
set_text(PyObject *text, TextEncoder::Encoding encoding) {
  char *str;
  Py_ssize_t len;
  if (PyBytes_AsStringAndSize(text, &str, &len) >= 0) {
    _this->set_text(std::string(str, len), encoding);
  }
}

/**
 * Returns the text as a string.  In Python 2, the returned string is in the
 * TextEncoder's specified encoding.  In Python 3, it is returned as unicode.
 */
PyObject *Extension<TextEncoder>::
get_text() const {
  std::wstring text = _this->get_wtext();
  return PyUnicode_FromWideChar(text.data(), (Py_ssize_t)text.size());
}

/**
 * Returns the text as a bytes object in the given encoding.
 */
PyObject *Extension<TextEncoder>::
get_text(TextEncoder::Encoding encoding) const {
  std::string text = _this->get_text(encoding);
  return PyBytes_FromStringAndSize((char *)text.data(), (Py_ssize_t)text.size());
}

/**
 * Appends the text as a string (or Unicode object in Python 2).
 */
void Extension<TextEncoder>::
append_text(PyObject *text) {
  if (PyUnicode_Check(text)) {
    Py_ssize_t len;
    const char *str = PyUnicode_AsUTF8AndSize(text, &len);
    std::string text_str(str, len);
    if (_this->get_encoding() == TextEncoder::E_utf8) {
      _this->append_text(text_str);
    } else {
      _this->append_wtext(TextEncoder::decode_text(text_str, TextEncoder::E_utf8));
    }
  } else {
    Dtool_Raise_TypeError("expected string");
  }
}

/**
 * Encodes the given wide character as byte string in the given encoding.
 */
PyObject *Extension<TextEncoder>::
encode_wchar(char32_t ch, TextEncoder::Encoding encoding) {
  std::string value = TextEncoder::encode_wchar(ch, encoding);
  return PyBytes_FromStringAndSize((char *)value.data(), (Py_ssize_t)value.size());
}

/**
 * Encodes a wide-text string into a single-char string, according to the
 * given encoding.
 */
PyObject *Extension<TextEncoder>::
encode_wtext(const wstring &wtext, TextEncoder::Encoding encoding) {
  std::string value = TextEncoder::encode_wtext(wtext, encoding);
  return PyBytes_FromStringAndSize((char *)value.data(), (Py_ssize_t)value.size());
}

/**
 * Returns the given wstring decoded to a single-byte string, via the given
 * encoding system.
 */
PyObject *Extension<TextEncoder>::
decode_text(PyObject *text, TextEncoder::Encoding encoding) {
  char *str;
  Py_ssize_t len;
  if (PyBytes_AsStringAndSize(text, &str, &len) >= 0) {
    return Dtool_WrapValue(TextEncoder::decode_text(std::string(str, len), encoding));
  } else {
    return nullptr;
  }
}

#endif  // HAVE_PYTHON
