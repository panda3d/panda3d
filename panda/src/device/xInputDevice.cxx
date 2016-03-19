/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xInputDevice.cxx
 * @author rdb
 * @date 2015-07-15
 */

#include "xInputDevice.h"

#ifdef _WIN32
#include "gamepadButton.h"
#include "inputDeviceManager.h"
#include "string_utils.h"

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
typedef DWORD (*pXInputSetState)(DWORD, XINPUT_VIBRATION *);
typedef DWORD (*pXInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES *);
typedef DWORD (*pXInputGetBatteryInformation)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION *);
typedef DWORD (*pXInputGetBaseBusInformation)(DWORD, XINPUT_BUSINFO *);

static pXInputGetState get_state = NULL;
static pXInputSetState set_state = NULL;
static pXInputGetCapabilities get_capabilities = NULL;
static pXInputGetBatteryInformation get_battery_information = NULL;
static pXInputGetBaseBusInformation get_base_bus_information = NULL;

bool XInputDevice::_initialized = false;

/**
 * Protected constructor.  user_index is a number 0-3.
 */
XInputDevice::
XInputDevice(DWORD user_index) :
  _index(user_index),
  _last_packet(-1),
  _last_buttons(0) {

  nassertv(user_index >= 0 && user_index < XUSER_MAX_COUNT);

  if (!_initialized) {
    nassertv(init_xinput());
  }

  _name = "XInput Device #";
  _name += format_string(user_index);

  _controls.resize(6);
  _buttons.resize(16);

  set_control_map(0, C_left_trigger);
  set_control_map(1, C_right_trigger);
  set_control_map(2, C_left_x);
  set_control_map(3, C_left_y);
  set_control_map(4, C_right_x);
  set_control_map(5, C_right_y);

  set_button_map(0, GamepadButton::dpad_up());
  set_button_map(1, GamepadButton::dpad_down());
  set_button_map(2, GamepadButton::dpad_left());
  set_button_map(3, GamepadButton::dpad_right());
  set_button_map(4, GamepadButton::start());
  set_button_map(5, GamepadButton::back());
  set_button_map(6, GamepadButton::lstick());
  set_button_map(7, GamepadButton::rstick());
  set_button_map(8, GamepadButton::lshoulder());
  set_button_map(9, GamepadButton::rshoulder());
  set_button_map(10, GamepadButton::guide());
  set_button_map(11, GamepadButton::action_a());
  set_button_map(12, GamepadButton::action_b());
  set_button_map(13, GamepadButton::action_x());
  set_button_map(14, GamepadButton::action_y());

  // Check if the device is connected.  If so, initialize it.
  XINPUT_CAPABILITIES caps;
  XINPUT_STATE state;
  if (get_capabilities(_index, 0, &caps) == ERROR_SUCCESS &&
      get_state(_index, &state) == ERROR_SUCCESS) {
    _is_connected = true;
    init_device(caps, state);
  } else {
    _is_connected = false;
  }
}

/**
 *
 */
XInputDevice::
~XInputDevice() {
  do_set_vibration(0, 0);
}

/**
 * Called periodically by the InputDeviceManager to detect whether the device
 * is currently connected.
 */
void XInputDevice::
detect(InputDeviceManager *mgr) {
  bool connected = false;

  XINPUT_CAPABILITIES caps;
  XINPUT_STATE state;
  if (get_capabilities(_index, 0, &caps) == ERROR_SUCCESS &&
      get_state(_index, &state) == ERROR_SUCCESS) {
    connected = true;
  } else {
    connected = false;
  }

  LightMutexHolder holder(_lock);
  if (connected == _is_connected) {
    // Nothing changed.
    return;
  }
  _is_connected = connected;

  if (connected) {
    init_device(caps, state);
    mgr->add_device(this);
  } else {
    mgr->remove_device(this);
  }
}

/**
 * Static method to initialize the XInput library.
 */
bool XInputDevice::
init_xinput() {
  if (device_cat.is_debug()) {
    device_cat.debug() << "Initializing XInput library.\n";
  }

  _initialized = true;
  const char *dll_name = "Xinput1_4.dll";
  HMODULE module = LoadLibraryA(dll_name);

  // If we didn't find XInput 1.4, fall back to the older 1.3 version.
  if (!module) {
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Xinput1_4.dll not found, falling back to Xinput1_3.dll\n";
    }

    dll_name = "Xinput1_3.dll";
    module = LoadLibraryA(dll_name);
  }

  if (module) {
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Successfully loaded " << dll_name << "\n";
    }

    // Undocumented version (XInputGetStateEx) that includes a
    // state bit for the guide button.
    get_state = (pXInputGetState)GetProcAddress(module, MAKEINTRESOURCE(100));
    if (get_state == NULL) {
      get_state = (pXInputGetState)GetProcAddress(module, "XInputGetState");
      if (get_state == NULL) {
        device_cat.error()
          << "Failed to find function XInputGetState in " << dll_name << ".\n";
        return false;
      }
    }

    set_state = (pXInputSetState)GetProcAddress(module, "XInputSetState");
    if (set_state == NULL) {
      device_cat.error()
        << "Failed to find function XInputSetState in " << dll_name << ".\n";
      return false;
    }

    get_capabilities = (pXInputGetCapabilities)GetProcAddress(module, "XInputGetCapabilities");
    if (get_capabilities == NULL) {
      device_cat.error()
        << "Failed to find function XInputGetCapabilities in " << dll_name << ".\n";
      return false;
    }

    get_battery_information = (pXInputGetBatteryInformation)GetProcAddress(module, "XInputGetBatteryInformation");
    get_base_bus_information = (pXInputGetBaseBusInformation)GetProcAddress(module, MAKEINTRESOURCE(104));
    return true;
  }

  device_cat.error()
    << "Failed to load Xinput1_4.dll or Xinput1_3.dll.\n";
  return false;
}

/**
 * Initializes the device.  Called when the device was just connected.
 */
void XInputDevice::
init_device(const XINPUT_CAPABILITIES &caps, const XINPUT_STATE &state) {
  if (caps.Type == XINPUT_DEVTYPE_GAMEPAD) {
    _device_class = DC_gamepad;
  } else {
    _device_class = DC_unknown;
  }

  if (caps.Flags & XINPUT_CAPS_FFB_SUPPORTED) {
    _flags |= IDF_has_vibration;
  }

  if (get_battery_information != NULL) {
    XINPUT_BATTERY_INFORMATION batt;
    if (get_battery_information(_index, 0, &batt) == ERROR_SUCCESS) {
      if (batt.BatteryType != BATTERY_TYPE_DISCONNECTED &&
          batt.BatteryType != BATTERY_TYPE_WIRED) {
        // This device has a battery.  Report the battery level.
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
  }

  WORD buttons = state.Gamepad.wButtons;
  WORD mask = 1;
  for (int i = 0; i < 16; ++i) {
    if (buttons & mask) {
      // Set the state without triggering a button event.
      _buttons[i]._state = (buttons & mask) ? S_down : S_up;
    }
    mask <<= 1;
    if (i == 10) {
      // XInput skips 0x0800.
      mask <<= 1;
    }
  }

  set_control_state(0, state.Gamepad.bLeftTrigger / 255.0);
  set_control_state(1, state.Gamepad.bRightTrigger / 255.0);
  set_control_state(2, state.Gamepad.sThumbLX / 32767.0);
  set_control_state(3, state.Gamepad.sThumbLY / 32767.0);
  set_control_state(4, state.Gamepad.sThumbRX / 32767.0);
  set_control_state(5, state.Gamepad.sThumbRY / 32767.0);

  _last_buttons = buttons;
  _last_packet = state.dwPacketNumber;
}

/**
 * Sets the vibration strength.  The first argument controls a low-frequency
 * motor, if present, and the latter controls a high-frequency motor.
 * The values are within the 0-1 range.
 */
void XInputDevice::
do_set_vibration(double strong, double weak) {
  XINPUT_VIBRATION vibration;
  vibration.wLeftMotorSpeed = strong * 0xffff;
  vibration.wRightMotorSpeed = weak * 0xffff;
  set_state(_index, &vibration);
}

/**
 * Polls the input device for new activity, to ensure it contains the latest
 * events.  This will only have any effect for some types of input devices;
 * others may be updated automatically, and this method will be a no-op.
 */
void XInputDevice::
do_poll() {
  XINPUT_STATE state;

  if (get_state(_index, &state) != ERROR_SUCCESS) {
    // Device was disconnected.
    if (_is_connected) {
      _is_connected = false;
      InputDeviceManager *mgr = InputDeviceManager::get_global_ptr();
      mgr->remove_device(this);
    }
    return;

  } else if (!_is_connected) {
    // Device was (re)connected.  It's a bit strange to call poll() on a
    // disconnected device, but there's nothing stopping the user from
    // doing so.
    XINPUT_CAPABILITIES caps;
    if (get_capabilities(_index, 0, &caps) == ERROR_SUCCESS) {
      _is_connected = true;
      init_device(caps, state);
      InputDeviceManager *mgr = InputDeviceManager::get_global_ptr();
      mgr->add_device(this);
    }
  }

  if (state.dwPacketNumber == _last_packet) {
    // No change since last time we asked.
    return;
  }

  // Did any buttons change state?
  WORD changed_buttons = _last_buttons ^ state.Gamepad.wButtons;

  WORD mask = 1;
  for (int i = 0; i < 16; ++i) {
    if (changed_buttons & mask) {
      set_button_state(i, (state.Gamepad.wButtons & mask) != 0);
    }
    mask <<= 1;
    if (i == 10) {
      // XInput skips 0x0800.
      mask <<= 1;
    }
  }

  set_control_state(0, state.Gamepad.bLeftTrigger / 255.0);
  set_control_state(1, state.Gamepad.bRightTrigger / 255.0);
  set_control_state(2, (state.Gamepad.sThumbLX + 0.5) / 32767.5);
  set_control_state(3, (state.Gamepad.sThumbLY + 0.5) / 32767.5);
  set_control_state(4, (state.Gamepad.sThumbRX + 0.5) / 32767.5);
  set_control_state(5, (state.Gamepad.sThumbRY + 0.5) / 32767.5);

  _last_buttons = state.Gamepad.wButtons;
  _last_packet = state.dwPacketNumber;
}

#endif  // _WIN32
