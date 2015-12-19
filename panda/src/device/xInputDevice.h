// Filename: xInputDevice.h
// Created by:  rdb (21Jul15)
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

#ifndef XINPUTDEVICE_H
#define XINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

#ifdef _WIN32

class InputDeviceManager;

typedef struct _XINPUT_CAPABILITIES XINPUT_CAPABILITIES;
typedef struct _XINPUT_STATE XINPUT_STATE;

////////////////////////////////////////////////////////////////////
//       Class : XInputDevice
// Description : This uses Microsoft's XInput library to interface
//               with an Xbox 360 game controller.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE XInputDevice FINAL : public InputDevice {
public:
  XInputDevice(DWORD user_index);
  ~XInputDevice();

  void detect(InputDeviceManager *mgr);
  static bool init_xinput();

private:
  void init_device(const XINPUT_CAPABILITIES &caps, const XINPUT_STATE &state);
  virtual void do_set_vibration(double strong, double weak);
  virtual void do_poll();

private:
  const DWORD _index;
  DWORD _last_packet;
  WORD _last_buttons;

  static bool _initialized;
};

#endif  // _WIN32

#endif
