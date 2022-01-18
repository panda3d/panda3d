/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clipboard.h
 * @author rdb
 * @date 2022-01-17
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "pandabase.h"
#include "asyncFuture.h"
#include "vector_uchar.h"

/**
 * Abstract base class for clipboard implementations.
 */
class EXPCL_PANDA_DISPLAY Clipboard {
PUBLISHED:
  /**
   * Returns a future that resolves to the contents of the clipboard in text
   * format, or an empty string if it did not contain any text.
   *
   * If the operating system refuses access to the clipboard, the future will
   * be cancelled.
   */
  virtual PT(AsyncFuture) request_text()=0;

  /**
   * Returns a future that resolves to the contents of the clipboard as binary
   * data of the given MIME type, or null if it did not contain any data of
   * this MIME type.
   *
   * If the operating system refuses access to the clipboard, the future may
   * be cancelled.
   */
  virtual PT(AsyncFuture) request_data(const std::string &mime_type)=0;

  /**
   * Empties the contents of the clipboard.
   */
  virtual void clear()=0;

  /**
   * Replaces the contents of the clipboard with the given text string.
   *
   * It is not guaranteed that the change is applied \em immediately, but it
   * \em is guaranteed that successive calls to request_text() return this
   * data, unless the clipboard is overwritten after the call to set_text().
   */
  virtual void set_text(const std::string &text)=0;

  /**
   * Writes arbitrary data to the clipboard, the format indicated by a MIME
   * type.  Returns false if the OS does not support data of the given type.
   *
   * Which types are supported is determined by what the operating system and
   * what other programs support.  For example, "image/bmp" generally works
   * well on Windows, "image/tiff" is accepted but is not recognized by most
   * applications, and "image/webp" is rejected entirely.
   *
   * It is not guaranteed that the change is applied \em immediately, but it
   * \em is guaranteed that successive calls to request_text() return this
   * data, unless the clipboard is overwritten by someone else.
   */
  virtual bool set_data(const std::string &mime_type, const vector_uchar &data)=0;
};

#endif
