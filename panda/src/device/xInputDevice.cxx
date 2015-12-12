// Filename: xInputDevice.cxx
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

#include "xInputDevice.h"

#ifdef _WIN32
#include "gamepadButton.h"

#include <XInput.h>
#include <CfgMgr32.h>

#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif

#ifndef XINPUT_CAPS_FFB_SUPPORTED
#define XINPUT_CAPS_FFB_SUPPORTED 0x0001
#endif

#ifndef BATTERY_DEVTYPE_GAMEPAD
#define BATTERY_DEVTYPE_GAMEPAD 0x00
#endif

#ifndef BATTERY_TYPE_DISCONNECTED
#define BATTERY_TYPE_DISCONNECTED 0x00
#endif

#ifndef BATTERY_TYPE_WIRED
#define BATTERY_TYPE_WIRED 0x01
#endif

#ifndef BATTERY_LEVEL_FULL
#define BATTERY_LEVEL_FULL 0x03
#endif

typedef struct _XINPUT_BATTERY_INFORMATION {
  BYTE BatteryType;
  BYTE BatteryLevel;
} XINPUT_BATTERY_INFORMATION;

// Undocumented, I figured out how this looks by trial and error.
struct XINPUT_BUSINFO {
  WORD VendorID;
  WORD ProductID;
  WORD RevisionID;
  WORD Unknown1; // Unknown - padding?
  DWORD InstanceID;
  DWORD Unknown2;
  //WORD Unknown3;
};

typedef DWORD (*pXInputGetState)(DWORD, XINPUT_STATE *);
typedef DWORD (*pXInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES *);
typedef DWORD (*pXInputGetBatteryInformation)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION *);
typedef DWORD (*pXInputGetBaseBusInformation)(DWORD, XINPUT_BUSINFO *);

static pXInputGetState get_state = NULL;
static pXInputGetCapabilities get_capabilities = NULL;
static pXInputGetBatteryInformation get_battery_information = NULL;
static pXInputGetBaseBusInformation get_base_bus_information = NULL;

bool XInputDevice::_initialized = false;

////////////////////////////////////////////////////////////////////
//     Function: XInputDevice::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
XInputDevice::
XInputDevice(DWORD user_index) :
  _index(user_index),
  _last_packet(-1),
  _last_buttons(0) {

  nassertv(user_index >= 0 && user_index < XUSER_MAX_COUNT);

  if (!_initialized) {
    nassertv(init_xinput());
  }

  if (get_capabilities != NULL) {
    XINPUT_CAPABILITIES caps;
    if (get_capabilities(_index, 0, &caps) == ERROR_SUCCESS) {
      _is_connected = true;
    } else {
      _is_connected = false;
    }

    if (caps.Type == XINPUT_DEVTYPE_GAMEPAD) {
      _device_class = DC_gamepad;
    } else {
      _device_class = DC_unknown;
    }

    if (caps.Flags & XINPUT_CAPS_FFB_SUPPORTED) {
      _flags |= IDF_has_vibration;
    }
  }

  if (get_battery_information != NULL) {
    XINPUT_BATTERY_INFORMATION batt;
    if (get_battery_information(_index, 0, &batt) == ERROR_SUCCESS) {
      if (batt.BatteryType == BATTERY_TYPE_DISCONNECTED) {
        _is_connected = false;

      } else if (batt.BatteryType != BATTERY_TYPE_WIRED) {
        // This device has a battery.  Report the battery level.
        _is_connected = true;
        _flags |= IDF_has_battery;
        _battery_level = batt.BatteryLevel;
        _max_battery_level = BATTERY_LEVEL_FULL;
      }
    }
  }

  // Get information about the USB device.
  // This is not documented at all.  I'm probably the first to try this.
  XINPUT_BUSINFO businfo;
  if (get_base_bus_information != NULL &&
      get_base_bus_information(0, &businfo) == ERROR_SUCCESS) {
    _vendor_id = businfo.VendorID;
    _product_id = businfo.ProductID;

    {
      // Reformat the serial number into its original hex string form.
      char sn[10];
      sprintf(sn, "%08X", businfo.InstanceID);
      _serial_number.assign(sn, 8);
    }

    // Get information about the device from Windows.  For that, we'll
    // first need to construct the device path.  Fortunately, we now have
    // enough information to do so.
    char path[32];
    sprintf(path, "USB\\VID_%04X&PID_%04X\\%08X", businfo.VendorID, businfo.ProductID, businfo.InstanceID);

    DEVINST inst;
    if (CM_Locate_DevNodeA(&inst, path, 0) != 0) {
      if (device_cat.is_debug()) {
        device_cat.debug()
          << "Could not locate device node " << path << "\n";
      }
    } else {
      // Get the device properties we need.
      char buffer[4096];
      ULONG buflen = 4096;
      if (CM_Get_DevNode_Registry_Property(inst, CM_DRP_DEVICEDESC, 0, buffer, &buflen, 0) == CR_SUCCESS) {
        _name.assign(buffer);
      }
      buflen = 4096;
      if (CM_Get_DevNode_Registry_Property(inst, CM_DRP_MFG, 0, buffer, &buflen, 0) == CR_SUCCESS) {
        _manufacturer.assign(buffer);
      }
    }
  } else {
    // We need something to name it.
    _name = "XInput Device";
  }

  _controls.resize(6);
  _buttons.resize(16);

  // Get the initial state.
  XINPUT_STATE state;
  if (get_state(_index, &state) != ERROR_SUCCESS) {
    // Device was disconnected.
    _is_connected = false;
    return;
  }

  WORD buttons = state.Gamepad.wButtons;

  set_control_map(0, C_left_trigger);
  set_control_state(0, state.Gamepad.bLeftTrigger / 255.0);
  set_control_map(1, C_right_trigger);
  set_control_state(1, state.Gamepad.bRightTrigger / 255.0);
  set_control_map(2, C_left_x);
  set_control_state(2, state.Gamepad.sThumbLX / 32767.0);
  set_control_map(3, C_left_y);
  set_control_state(3, state.Gamepad.sThumbLY / 32767.0);
  set_control_map(4, C_right_x);
  set_control_state(4, state.Gamepad.sThumbRX / 32767.0);
  set_control_map(5, C_right_y);
  set_control_state(5, state.Gamepad.sThumbRY / 32767.0);

  set_button_map(0, GamepadButton::dpad_up());
  set_button_state(0, (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0);
  set_button_map(1, GamepadButton::dpad_down());
  set_button_state(1, (buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
  set_button_map(2, GamepadButton::dpad_left());
  set_button_state(2, (buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
  set_button_map(3, GamepadButton::dpad_right());
  set_button_state(3, (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
  set_button_map(4, GamepadButton::start());
  set_button_state(4, (buttons & XINPUT_GAMEPAD_START) != 0);
  set_button_map(5, GamepadButton::back());
  set_button_state(5, (buttons & XINPUT_GAMEPAD_BACK) != 0);
  set_button_map(6, GamepadButton::lstick());
  set_button_state(6, (buttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
  set_button_map(7, GamepadButton::rstick());
  set_button_state(7, (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);
  set_button_map(8, GamepadButton::lshoulder());
  set_button_state(8, (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
  set_button_map(9, GamepadButton::rshoulder());
  set_button_state(9, (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
  set_button_map(10, GamepadButton::guide());
  set_button_state(10, (buttons & 0x0400) != 0);
  set_button_map(11, GamepadButton::action_a());
  set_button_state(11, (buttons & XINPUT_GAMEPAD_A) != 0);
  set_button_map(12, GamepadButton::action_b());
  set_button_state(12, (buttons & XINPUT_GAMEPAD_B) != 0);
  set_button_map(13, GamepadButton::action_x());
  set_button_state(13, (buttons & XINPUT_GAMEPAD_X) != 0);
  set_button_map(14, GamepadButton::action_y());
  set_button_state(14, (buttons & XINPUT_GAMEPAD_Y) != 0);

  _last_buttons = buttons;
}

////////////////////////////////////////////////////////////////////
//     Function: XInputDevice::init
//       Access: Protected, Static
//  Description: Initializes the XInput library.
////////////////////////////////////////////////////////////////////
bool XInputDevice::
init_xinput() {
  _initialized = true;
  HMODULE module = LoadLibraryA("Xinput1_4.dll");
  if (!module) {
    module = LoadLibraryA("Xinput1_3.dll");
  }
  if (module) {
    // Undocumented version (XInputGetStateEx) that includes a
    // state bit for the guide button.
    get_state = (pXInputGetState)GetProcAddress(module, MAKEINTRESOURCE(100));
    if (get_state == NULL) {
      get_state = (pXInputGetState)GetProcAddress(module, "XInputGetState");
      if (get_state == NULL) {
        device_cat.error()
          << "Failed to find function XInputGetState in XInput DLL.\n";
        return false;
      }
    }

    get_capabilities = (pXInputGetCapabilities)GetProcAddress(module, "XInputGetCapabilities");
    get_battery_information = (pXInputGetBatteryInformation)GetProcAddress(module, "XInputGetBatteryInformation");
    get_base_bus_information = (pXInputGetBaseBusInformation)GetProcAddress(module, MAKEINTRESOURCE(104));
    return true;
  }

  device_cat.error()
    << "Failed to load XInput DLL.\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::do_poll
//       Access: Public, Virtual
//  Description: Polls the input device for new activity, to ensure
//               it contains the latest events.  This will only have
//               any effect for some types of input devices; others
//               may be updated automatically, and this method will
//               be a no-op.
////////////////////////////////////////////////////////////////////
void XInputDevice::
do_poll() {
  XINPUT_STATE state;

  if (get_state(_index, &state) != ERROR_SUCCESS) {
    // Device was disconnected.
    _is_connected = false;
    return;
  } else {
    _is_connected = true;
  }

  if (state.dwPacketNumber == _last_packet) {
    // No change since last time we asked.
    return;
  }

  WORD changed_buttons = _last_buttons ^ state.Gamepad.wButtons;

  WORD mask = 1;
  for (int i = 0; i < 16; ++i) {
    if (changed_buttons & mask) {
      set_button_state(i, (state.Gamepad.wButtons & mask) != 0);
    }
    mask <<= 1;
    if (i == 10) {
      mask <<= 1;
    }
  }

  set_control_state(0, state.Gamepad.bLeftTrigger / 255.0);
  set_control_state(1, state.Gamepad.bRightTrigger / 255.0);
  set_control_state(2, state.Gamepad.sThumbLX / 32767.0);
  set_control_state(3, state.Gamepad.sThumbLY / 32767.0);
  set_control_state(4, state.Gamepad.sThumbRX / 32767.0);
  set_control_state(5, state.Gamepad.sThumbRY / 32767.0);

  _last_buttons = state.Gamepad.wButtons;
  _last_packet = state.dwPacketNumber;
}

#endif  // _WIN32
