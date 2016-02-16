// Filename: evdevInputDevice.h
// Created by:  rdb (24Aug15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef EVDEVINPUTDEVICE_H
#define EVDEVINPUTDEVICE_H

#include "inputDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

////////////////////////////////////////////////////////////////////
//       Class : EvdevInputDevice
// Description : This is a type of device that uses the Linux
//               /dev/input/event# API to read data from a raw mouse.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE EvdevInputDevice : public InputDevice {
public:
  EvdevInputDevice(int index);
  virtual ~EvdevInputDevice();

private:
  virtual void do_set_vibration(double strong, double weak);
  virtual void do_poll();

  bool init_device();
  bool process_events();

private:
  int _index;
  int _fd;
  bool _can_write;
  int _ff_id;
  bool _ff_playing;
  int _ff_strong;
  int _ff_weak;

  struct AxisRange {
    double _scale;
    double _bias;
  };
  pvector<AxisRange> _axis_ranges;

public:
  static ButtonHandle map_button(int code);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    InputDevice::init_type();
    register_type(_type_handle, "EvdevInputDevice",
                  InputDevice::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "evdevInputDevice.I"

#endif  // PHAVE_LINUX_INPUT_H

#endif  // EVDEVINPUTDEVICE_H
