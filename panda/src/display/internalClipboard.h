/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalClipboard.h
 * @author rdb
 * @date 2022-01-18
 */

#ifndef INTERNALCLIPBOARD_H
#define INTERNALCLIPBOARD_H

#include "clipboard.h"

/**
 * A simple clipboard implementation that is used in absence of an OS-provided
 * clipboard implementation.  This one is internal to the current process only.
 */
class EXPCL_PANDA_DISPLAY InternalClipboard final : public Clipboard {
public:
  virtual PT(AsyncFuture) request_text() override;
  virtual PT(AsyncFuture) request_data(const std::string &mime_type) override;

  virtual void clear() override;
  virtual void set_text(const std::string &text) override;
  virtual bool set_data(const std::string &mime_type, const vector_uchar &data) override;

  static Clipboard *get_global_ptr();

private:
  LightMutex _lock;
  enum Type {
    T_empty,
    T_text,
    T_data,
  };
  Type _type = T_empty;
  std::string _text_or_mime_type;
  vector_uchar _data;
};

#endif
