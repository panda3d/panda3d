// Filename: inputDeviceManager.h
// Created by:  rdb (09Dec15)
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

#ifndef INPUTDEVICEMANAGER_H
#define INPUTDEVICEMANAGER_H

#include "pandabase.h"
#include "lightMutex.h"
#include "inputDevice.h"

////////////////////////////////////////////////////////////////////
//       Class : InputDeviceManager
// Description : This class keeps track of all the devices on a
//               system, and sends out events when a device has been
//               hot-plugged.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE InputDeviceManager {
private:
  InputDeviceManager();
  ~InputDeviceManager();

#ifdef PHAVE_LINUX_INPUT_H
  bool consider_add_linux_device(const string &name);
#endif

public:
  void add_device(InputDevice *device);
  void remove_device(InputDevice *device);

PUBLISHED:
  void poll();

  INLINE static InputDeviceManager *get_global_ptr();

private:
  LightMutex _lock;

#ifdef PHAVE_LINUX_INPUT_H
  int _inotify_fd;

  typedef pmap<string, InputDevice*> DevicesByPath;
  DevicesByPath _devices_by_path;
#endif

  typedef pvector<PT(InputDevice)> InputDevices;
  InputDevices _all_devices;

  static InputDeviceManager *_global_ptr;
};

#include "inputDeviceManager.I"

#endif
