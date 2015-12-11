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

////////////////////////////////////////////////////////////////////
//       Class : XInputDevice
// Description : This uses Microsoft's XInput library to interface
//               with an Xbox 360 game controller.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE XInputDevice : public InputDevice {
PUBLISHED:
  XInputDevice(DWORD user_index);

  static bool init_xinput();

public:
  virtual void do_poll();

private:
  DWORD _index;
  DWORD _last_packet;
  WORD _last_buttons;

  static bool _initialized;

  // There are only four of these in existence.
  //static XInputDevice _devices[4];
};

#endif  // _WIN32

#endif
