/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winClipboard.h
 * @author rdb
 * @date 2022-01-18
 */

#ifndef WINCLIPBOARD_H
#define WINCLIPBOARD_H

#include "config_display.h"
#include "clipboard.h"
#include "pmutex.h"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

/**
 * Clipboard implementation that is used in absence of an OS-provided
 * clipboard.  This one is local to the current process only.
 */
class EXPCL_PANDAWIN WinClipboard final : public Clipboard {
public:
  WinClipboard(HWND hwnd);
  ~WinClipboard();

  virtual PT(AsyncFuture) request_text() override;
  virtual PT(AsyncFuture) request_data(const std::string &mime_type) override;

  virtual void clear() override;
  virtual void set_text(const std::string &text) override;
  virtual bool set_data(const std::string &mime_type, const vector_uchar &data) override;

private:
  enum ThreadState {
    TS_wait,
    TS_shutdown,
    TS_write,
  };

  PT(AsyncFuture) do_request(UINT format);
  static void process_data(AsyncFuture *fut, UINT format, HANDLE data);
  void write(UINT format, HANDLE data);
  void write_thread_main();

  const HWND _hwnd;
  PT(Thread) _write_thread;

  Mutex _lock;
  ConditionVar _cvar;
  ThreadState _thread_state = TS_wait;
  UINT _pending_format = 0;
  HANDLE _pending_data = nullptr;
  unsigned int _pending_requests = 0;
};

#endif
