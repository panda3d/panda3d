// Filename: linuxJoystickDevice.h
// Created by:  rdb (21Aug15)
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

#ifndef LINUXJOYSTICKDEVICE_H
#define LINUXJOYSTICKDEVICE_H

#include "inputDevice.h"

#ifdef PHAVE_LINUX_INPUT_H

////////////////////////////////////////////////////////////////////
//       Class : LinuxJoystickDevice
// Description : This is a type of device that uses the Linux
//               /dev/input/js# API to read data from a game controller.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE LinuxJoystickDevice : public InputDevice {
PUBLISHED:
  LinuxJoystickDevice(const string &device);
  virtual ~LinuxJoystickDevice();

private:
  virtual void do_poll();

  bool open_device();
  bool process_events();

private:
  int _fd;
  string _device;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    InputDevice::init_type();
    register_type(_type_handle, "LinuxJoystickDevice",
                  InputDevice::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "linuxJoystickDevice.I"

#endif  // PHAVE_LINUX_INPUT_H

#endif  // LINUXJOYSTICKDEVICE_H
