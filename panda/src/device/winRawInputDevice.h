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
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_DEVICE WinRawInputDevice final : public InputDevice {
public:
  WinRawInputDevice(WinInputDeviceManager *manager, const char *path);
  ~WinRawInputDevice();

  bool on_arrival(HANDLE handle, const RID_DEVICE_INFO &info, std::string name);
  void on_removal();
  void on_input(PRAWINPUT input);
  void process_report(PCHAR ptr, size_t size);

private:
  virtual void do_poll();

private:
  const std::string _path;
  HANDLE _handle;
  void *_preparsed;
  ULONG _max_data_count;

  // Indexed by report ID
  pvector<BitArray> _report_buttons;

  // Either a button index or a axis index.
  struct Index {
    Index() : _button(-1), _axis(-1) {}

    static Index button(int index) {
      Index idx;
      idx._button = index;
      return idx;
    }
    static Index axis(int index, bool is_signed=true) {
      Index idx;
      idx._axis = index;
      idx._signed = is_signed;
      return idx;
    }

    int _button;
    int _axis;
    bool _signed;
  };

  // Maps a "data index" to either button index or axis index.
  pvector<Index> _indices;
  int _hat_data_index;
  int _hat_data_minimum;
  int _hat_left_button;

  WinInputDeviceManager *_manager;
  friend class WinInputDeviceManager;
};

#endif  // _WIN32

#endif
