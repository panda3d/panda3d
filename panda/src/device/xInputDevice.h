/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xInputDevice.h
 * @author rdb
 * @date 2015-07-15
 */

#ifndef XINPUTDEVICE_H
#define XINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

#if defined(_WIN32) && !defined(CPPPARSER)

#include <CfgMgr32.h>

class InputDeviceManager;

typedef struct _XINPUT_CAPABILITIES_EX XINPUT_CAPABILITIES_EX;
typedef struct _XINPUT_STATE XINPUT_STATE;

typedef struct tagRID_DEVICE_INFO RID_DEVICE_INFO;

/**
 * This implementation of InputDevice uses Microsoft's XInput library to
 * interface with an Xbox 360 game controller.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_DEVICE XInputDevice final : public InputDevice {
public:
  XInputDevice(DWORD user_index);
  ~XInputDevice();

  bool check_arrival(const RID_DEVICE_INFO &info, DEVINST inst,
                     const std::string &name, const std::string &manufacturer);
  void detect(InputDeviceManager *mgr);
  static bool init_xinput();

private:
  void init_device(const XINPUT_CAPABILITIES_EX &caps, const XINPUT_STATE &state);
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
