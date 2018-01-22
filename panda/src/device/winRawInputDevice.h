/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winRawInputDevice.h
 * @author rdb
 * @date 2018-01-19
 */

#ifndef WINRAWINPUTDEVICE_H
#define WINRAWINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"
#include "bitArray.h"

#if defined(_WIN32) && !defined(CPPPARSER)

class WinInputDeviceManager;

/**
 * This implementation of InputDevice uses the Win32 raw input API and the HID
 * parser library to support a wide range of devices.
 */
class EXPCL_PANDA_DEVICE WinRawInputDevice FINAL : public InputDevice {
public:
  WinRawInputDevice(WinInputDeviceManager *manager, const char *path);
  ~WinRawInputDevice();

  bool on_arrival(HANDLE handle, const RID_DEVICE_INFO &info, string name);
  void on_removal();
  void on_input(PRAWINPUT input);

private:
  virtual void do_poll();

private:
  const string _path;
  HANDLE _handle;
  DWORD _size;
  void *_preparsed;
  ULONG _max_data_count;
  ULONG _max_usage_count;

  // Indexed by report ID
  pvector<BitArray> _report_buttons;

  // Either a button index or a control index.
  struct Index {
    Index() : _button(-1), _control(-1) {}

    static Index button(int index) {
      Index idx;
      idx._button = index;
      return idx;
    }
    static Index control(int index, bool is_signed=true) {
      Index idx;
      idx._control = index;
      idx._signed = is_signed;
      return idx;
    }

    int _button;
    int _control;
    bool _signed;
  };

  // Maps a "data index" to either button index or control index.
  pvector<Index> _indices;
  int _hat_data_index;
  int _hat_data_minimum;
  int _hat_left_button;

  WinInputDeviceManager *_manager;
  friend class WinInputDeviceManager;
};

#endif  // _WIN32

#endif
