/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalClipboard.cxx
 * @author rdb
 * @date 2022-01-18
 */

#include "internalClipboard.h"
#include "lightMutexHolder.h"
#include "string_utils.h"

/**
 * Returns a future that resolves to the contents of the clipboard in text
 * format, or an empty string if it did not contain any text.
 *
 * If the operating system refuses access to the clipboard, the future will be
 * cancelled.
 */
PT(AsyncFuture) InternalClipboard::
request_text() {
  PT(AsyncFuture) fut = new AsyncFuture;
  LightMutexHolder holder(_lock);
  std::string text;
  if (_type == T_text) {
    text = _text_or_mime_type;
  }
  fut->set_result(text);
  return fut;
}

/**
 * Returns a future that resolves to the contents of the clipboard as binary
 * data of the given MIME type, or null if it did not contain any data of this
 * MIME type.
 *
 * If the operating system refuses access to the clipboard, the future will be
 * cancelled.
 */
PT(AsyncFuture) InternalClipboard::
request_data(const std::string &mime_type) {
  PT(AsyncFuture) fut = new AsyncFuture;
  LightMutexHolder holder(_lock);
  if (_type == T_data && cmp_nocase(mime_type, _text_or_mime_type) == 0) {
    fut->set_result(_data);
  } else {
    fut->set_result(nullptr, nullptr);
  }
  return fut;
}

/**
 * Clears any data currently in the clipboard.
 */
void InternalClipboard::
clear() {
  LightMutexHolder holder(_lock);
  _type = T_empty;
  _text_or_mime_type.clear();
  _data.clear();
}

/**
 * Writes a text string into the clipboard.
 */
void InternalClipboard::
set_text(const std::string &text) {
  LightMutexHolder holder(_lock);
  _type = T_text;
  _text_or_mime_type = text;
  _data.clear();
}

/**
 * Writes arbitrary data into the clipboard.
 */
bool InternalClipboard::
set_data(const std::string &mime_type, const vector_uchar &data) {
  LightMutexHolder holder(_lock);
  _type = T_data;
  _text_or_mime_type = mime_type;
  _data = data;
  return true;
}

/**
 *
 */
Clipboard *InternalClipboard::
get_global_ptr() {
  static InternalClipboard clipboard;
  return &clipboard;
}
