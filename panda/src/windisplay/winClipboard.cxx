/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winClipboard.cxx
 * @author rdb
 * @date 2022-01-18
 */

#include "winClipboard.h"
#include "datagram.h"
#include "genericThread.h"
#include "mutexHolder.h"
#include "string_utils.h"
#include "textEncoder.h"

/**
 *
 */
WinClipboard::
WinClipboard(HWND hwnd) :
  _hwnd(hwnd),
  _lock("clipboard"),
  _cvar(_lock) {
}

/**
 * Shuts down the clipboard thread.
 */
WinClipboard::
~WinClipboard() {
  {
    MutexHolder holder(_lock);
    if (_write_thread == nullptr) {
      return;
    }

    if (windisplay_cat.is_debug()) {
      windisplay_cat.debug()
        << "Shutting down clipboard thread.\n";
    }

    _thread_state = TS_shutdown;
  }

  // Wake up the thread to let it process the TS_shutdown message.
  // Wait for it to finish, otherwise it might access deleted structures.
  _cvar.notify();
  _write_thread->join();
  _write_thread.clear();
}

/**
 * Returns a future that resolves to the contents of the clipboard in text
 * format, or an empty string if it did not contain any text.
 *
 * If the operating system refuses access to the clipboard, the future will be
 * cancelled.
 */
PT(AsyncFuture) WinClipboard::
request_text() {
  MutexHolder holder(_lock);
  return do_request(CF_UNICODETEXT);
}

/**
 * Returns a future that resolves to the contents of the clipboard as binary
 * data of the given MIME type, or null if it did not contain any data of this
 * MIME type.
 *
 * If the operating system refuses access to the clipboard, the future will be
 * cancelled.
 */
PT(AsyncFuture) WinClipboard::
request_data(const std::string &mime_type) {
  MutexHolder holder(_lock);

  UINT format = 0;
  if (cmp_nocase(mime_type, "image/bmp") == 0) {
    format = CF_DIBV5;
  }
  else if (cmp_nocase(mime_type, "image/tiff") == 0) {
    format = CF_TIFF;
  }
  else if (cmp_nocase(mime_type, "audio/wav") == 0) {
    format = CF_WAVE;
  }
  else if (cmp_nocase(mime_type, "image/png") == 0) {
    format = RegisterClipboardFormatA("PNG");
  }
  else if (cmp_nocase(mime_type, "image/gif") == 0) {
    format = RegisterClipboardFormatA("GIF");
  }
  else if (cmp_nocase(mime_type, "image/jpeg") == 0) {
    format = RegisterClipboardFormatA("JFIF");
  }
  else if (cmp_nocase(mime_type, "text/html") == 0) {
    format = RegisterClipboardFormatA("HTML Format");
  }
  else if (cmp_nocase(mime_type, "text/rtf") == 0) {
    format = RegisterClipboardFormatA("Rich Text Format");
  }

  return do_request(format);
}

/**
 * Clears any data currently in the clipboard.
 */
void WinClipboard::
clear() {
  write(0, 0);
}

/**
 * Writes a text string into the clipboard.
 */
void WinClipboard::
set_text(const std::string &text) {
  TextEncoder encoder;
  encoder.set_text(text);
  const std::wstring &wtext = encoder.get_wtext();

  HGLOBAL data = GlobalAlloc(GMEM_MOVEABLE, (wtext.size() + 1) * sizeof(wchar_t));
  nassertv_always(data != nullptr);

  {
    wchar_t *buf = (wchar_t *)GlobalLock(data);
    nassertv_always(buf != nullptr);
    memcpy(buf, wtext.data(), wtext.size() * sizeof(wchar_t));
    buf[wtext.size()] = 0;
    GlobalUnlock(data);
  }

  write(CF_UNICODETEXT, data);
}

/**
 * Writes arbitrary data into the clipboard.
 */
bool WinClipboard::
set_data(const std::string &mime_type, const vector_uchar &data) {
  UINT format = 0;
  const unsigned char *ptr = data.data();
  size_t size = data.size();

  if (cmp_nocase(mime_type, "image/bmp") == 0) {
    // Is it a v5 structure?
    if (data.size() < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
      windisplay_cat.error()
        << "Refusing to put invalid image/bmp data on clipboard.\n";
      return nullptr;
    }

    BITMAPINFOHEADER *info_header = (BITMAPINFOHEADER *)(data.data() + sizeof(BITMAPFILEHEADER));
    switch (info_header->biSize) {
    case sizeof(BITMAPINFOHEADER):
      format = CF_DIB;
      break;

    case sizeof(BITMAPV5HEADER):
      format = CF_DIBV5;
      break;

    default:
      windisplay_cat.error()
        << "Refusing to put image/bmp data with unrecognized header on clipboard.\n";
      return false;
    }

    // Remove the header.
    ptr += 14;
    size -= 14;
  }
  else if (cmp_nocase(mime_type, "image/tiff") == 0) {
    format = CF_TIFF;
  }
  else if (cmp_nocase(mime_type, "audio/wav") == 0) {
    format = CF_WAVE;
  }
  else if (cmp_nocase(mime_type, "image/png") == 0) {
    format = RegisterClipboardFormatA("PNG");
  }
  else if (cmp_nocase(mime_type, "image/gif") == 0) {
    format = RegisterClipboardFormatA("GIF");
  }
  else if (cmp_nocase(mime_type, "image/jpeg") == 0) {
    format = RegisterClipboardFormatA("JFIF");
  }
  else if (cmp_nocase(mime_type, "text/html") == 0) {
    format = RegisterClipboardFormatA("HTML Format");
  }
  else if (cmp_nocase(mime_type, "text/rtf") == 0) {
    format = RegisterClipboardFormatA("Rich Text Format");
  }

  if (format == 0) {
    // Unsupported format.
    return false;
  }

  HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE, size);
  nassertr_always(handle != nullptr, false);

  {
    wchar_t *buf = (wchar_t *)GlobalLock(handle);
    nassertr_always(buf != nullptr, false);
    memcpy(buf, ptr, size);
    GlobalUnlock(handle);
  }

  write(format, handle);
  return true;
}

/**
 * Assumes the lock is held.
 */
PT(AsyncFuture) WinClipboard::
do_request(UINT format) {
  PT(AsyncFuture) fut = new AsyncFuture;

  // If there is a pending write to the clipboard, return that instead,
  // so that we don't get issues with reads/writes being out of order.
  if (_pending_format == format) {
    process_data(fut, format, _pending_data);
    return fut;
  }

  if (format == 0 || _thread_state != TS_wait) {
    process_data(fut, format, nullptr);
    return fut;
  }

  // The write thread should wait for the writes to succeed.
  ++_pending_requests;

  PT(Thread) thread = new GenericThread("clipboard", "clipboard", [this, fut, format] {
    // Unfortunately there is no WaitClipboard call, so we have to do an
    // inefficient retry loop.
    if (!OpenClipboard(_hwnd)) {
      Sleep(0);
      while (!OpenClipboard(_hwnd)) {
        Sleep(1);
      }
    }

    process_data(fut, format, GetClipboardData(format));
    CloseClipboard();

    _lock.lock();
    if (--_pending_requests == 0 && _thread_state != TS_wait) {
      // Wake up the writer thread, which was waiting for us to be done.
      _lock.unlock();
      _cvar.notify();
    } else {
      _lock.unlock();
    }
  });

  thread->start(TP_normal, false);
  return fut;
}

/**
 * Processes the result of reading from the clipboard.
 */
void WinClipboard::
process_data(AsyncFuture *fut, UINT format, HANDLE data) {
  if (format == 0 || data == nullptr) {
    if (format == CF_UNICODETEXT) {
      fut->set_result(std::string());
    } else {
      fut->set_result(nullptr);
    }
    return;
  }

  const char *buf = (const char *)GlobalLock(data);
  size_t size = GlobalSize(data);

  if (format == CF_UNICODETEXT) {
    TextEncoder encoder;
    encoder.set_wtext(std::wstring((const wchar_t *)buf, wcsnlen((const wchar_t *)buf, size)));
    fut->set_result(encoder.get_text());
  }
  else if (format == CF_DIBV5) {
    uint32_t offset = 14;

    BITMAPINFOHEADER *info_header = (BITMAPINFOHEADER *)buf;
    offset += info_header->biSize;
    if (info_header->biBitCount <= 8) {
      offset += (1 << info_header->biBitCount) * 4;
    }

    // Insert a BMP header before the DIB data.
    Datagram dg;
    dg.append_data("BM", 2);
    dg.add_uint32(size + 14);
    dg.add_uint16(0);
    dg.add_uint16(0);
    dg.add_uint32(offset);

    vector_uchar data((unsigned char *)dg.get_data(),
                      (unsigned char *)dg.get_data() + dg.get_length());
    data.insert(data.end(), buf, buf + size);
    fut->set_result(std::move(data));
  }
  else {
    fut->set_result(vector_uchar((const unsigned char *)buf, (const unsigned char *)buf + size));
  }

  GlobalUnlock(data);
}

/**
 * Schedules that the given data should be written to the clipboard.
 */
void WinClipboard::
write(UINT format, HANDLE data) {
  {
    MutexHolder holder(_lock);
    _thread_state = TS_write;

    if (_pending_data != nullptr) {
      GlobalFree(_pending_data);
    }
    _pending_data = data;
    _pending_format = format;

    if (_write_thread == nullptr) {
      _write_thread = new GenericThread("clipboard", "clipboard", [this] { write_thread_main(); });
      _write_thread->start(TP_normal, true);

      // No need to notify(), the thread will start writing as soon as
      // we release the lock (or as soon as it begins to run).
      return;
    }
  }
  _cvar.notify();
}

/**
 *
 */
void WinClipboard::
write_thread_main() {
  MutexHolder holder(_lock);

  while (true) {
    // If there are any clipboard reads pending, we continue to wait (a reader
    // thread will wake us up when it's done), to avoid out-of-order issues.
    while (_thread_state == TS_wait || _pending_requests > 0) {
      _cvar.wait();
    }

    if (_thread_state == TS_shutdown) {
      return;
    }

    // We have a pending change, and there are no read threads active.
    // Try to unlock the clipboard.  There is no risk of any read threads
    // being started as long as _thread_state is not TS_wait.
    while (!OpenClipboard(_hwnd)) {
      _lock.unlock();
      Sleep(1);
      _lock.lock();
    }

    nassertd(_thread_state == TS_write && _pending_requests == 0) continue;

    // We got the system clipboard lock.  We always have to call EmptyClipboard
    // to take ownership.
    EmptyClipboard();

    if (_pending_data != nullptr) {
      if (SetClipboardData(_pending_format, _pending_data) != nullptr) {
        _thread_state = TS_wait;
        GlobalFree(_pending_data);
        _pending_data = nullptr;
        _pending_format = 0;
      }
    }

    CloseClipboard();
  }
}
