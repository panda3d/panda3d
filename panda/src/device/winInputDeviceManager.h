/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winInputDeviceManager.h
 * @author rdb
 * @date 2018-01-21
 */

#ifndef WININPUTDEVICEMANAGER_H
#define WININPUTDEVICEMANAGER_H

#include "inputDeviceManager.h"

#if defined(_WIN32) && !defined(CPPPARSER)

#include "xInputDevice.h"

#include <CfgMgr32.h>
#include <devpkey.h>

class WinRawInputDevice;

/**
 * This is the Windows implementation of InputDeviceManager, managing both
 * XInput controllers and raw input devices.
 */
class EXPCL_PANDA_DEVICE WinInputDeviceManager final : public InputDeviceManager {
private:
  WinInputDeviceManager();
  ~WinInputDeviceManager();

public:
  void device_destroyed(WinRawInputDevice *device);

  void on_input(HRAWINPUT handle);
  void on_input_device_arrival(HANDLE handle);
  void on_input_device_removal(HANDLE handle);

  HWND setup_message_loop();
  void destroy_message_loop();

  static void stop_thread();

private:
  // There are always exactly four of these in existence.
  XInputDevice _xinput_device0;
  XInputDevice _xinput_device1;
  XInputDevice _xinput_device2;
  XInputDevice _xinput_device3;

  HWND _message_hwnd;
  pmap<HANDLE, WinRawInputDevice *> _raw_devices;
  pmap<std::string, WinRawInputDevice *> _raw_devices_by_path;

  virtual void update() override;

  static LRESULT WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  typedef CONFIGRET (WINAPI *pCM_Get_DevNode_Property)(DEVINST, const DEVPROPKEY *, DEVPROPTYPE *, PBYTE, PULONG, ULONG);
  pCM_Get_DevNode_Property _CM_Get_DevNode_PropertyW;

  friend class InputDeviceManager;
};

#endif
#endif
