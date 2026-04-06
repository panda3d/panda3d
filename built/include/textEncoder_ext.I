/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textEncoder_ext.I
 * @author rdb
 * @date 2018-10-08
 */

/**
 * Encodes a wide-text string into a single-char string, according to the
 * current encoding.
 */
INLINE PyObject *Extension<TextEncoder>::
encode_wtext(const std::wstring &wtext) const {
  return encode_wtext(wtext, _this->get_encoding());
}

/**
 * Returns the given wstring decoded to a single-byte string, via the current
 * encoding system.
 */
INLINE PyObject *Extension<TextEncoder>::
decode_text(PyObject *text) const {
  return decode_text(text, _this->get_encoding());
}
