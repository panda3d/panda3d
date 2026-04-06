/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textEncoder_ext.h
 * @author rdb
 * @date 2018-09-29
 */

#ifndef TEXTENCODER_EXT_H
#define TEXTENCODER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "textEncoder.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for TextEncoder, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<TextEncoder> : public ExtensionBase<TextEncoder> {
public:
  void set_text(PyObject *text);
  void set_text(PyObject *text, TextEncoder::Encoding encoding);

  PyObject *get_text() const;
  PyObject *get_text(TextEncoder::Encoding encoding) const;
  void append_text(PyObject *text);

  static PyObject *encode_wchar(char32_t ch, TextEncoder::Encoding encoding);
  INLINE PyObject *encode_wtext(const std::wstring &wtext) const;
  static PyObject *encode_wtext(const std::wstring &wtext, TextEncoder::Encoding encoding);
  INLINE PyObject *decode_text(PyObject *text) const;
  static PyObject *decode_text(PyObject *text, TextEncoder::Encoding encoding);
};

#include "textEncoder_ext.I"

#endif  // HAVE_PYTHON

#endif  // TEXTENCODER_EXT_H
