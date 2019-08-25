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

#if defined(_WIN32) && !defined(CPPPARSER)

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
#ifndef XINPUT_CAPS_NO_NAVIGATION
#define XINPUT_CAPS_NO_NAVIGATION 0x0010
#endif

#ifndef BATTERY_DEVTYPE_GAMEPAD
#define BATTERY_DEVTYPE_GAMEPAD 0x00
#endif

#ifndef XINPUT_DEVSUBTYPE_WHEEL
#define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_STICK
#define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
#define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#ifndef XINPUT_DEVSUBTYPE_DANCE_PAD
#define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR
#define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#ifndef XINPUT_DEVSUBTYPE_DRUM_KIT
#define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
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
typedef struct _XINPUT_BUSINFO {
  WORD VendorID;
  WORD ProductID;
  WORD RevisionID;
  WORD Unknown1; // Unknown - padding?
  DWORD InstanceID;
  DWORD Unknown2;
  WORD Unknown3;
} XINPUT_BUSINFO;

typedef struct _XINPUT_CAPABILITIES_EX {
  BYTE Type;
  BYTE SubType;
  WORD Flags;
  XINPUT_GAMEPAD Gamepad;
  XINPUT_VIBRATION Vibration;

  // The following fields are undocumented.
  WORD VendorID;
  WORD ProductID;
  WORD RevisionID;
  WORD Unknown1;
  WORD Unknown2;
} XINPUT_CAPABILITIES_EX;

typedef DWORD (WINAPI *pXInputGetState)(DWORD, XINPUT_STATE *);
typedef DWORD (WINAPI *pXInputSetState)(DWORD, XINPUT_VIBRATION *);
typedef DWORD (WINAPI *pXInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES *);
typedef DWORD (WINAPI *pXInputGetCapabilitiesEx)(DWORD, DWORD, DWORD, XINPUT_CAPABILITIES_EX *);
typedef DWORD (WINAPI *pXInputGetBatteryInformation)(DWORD, BYTE, XINPUT_BATTERY_INFORMATION *);
typedef DWORD (WINAPI *pXInputGetBaseBusInformation)(DWORD, XINPUT_BUSINFO *);

static pXInputGetState get_state = nullptr;
static pXInputSetState set_state = nullptr;
static pXInputGetCapabilities get_capabilities = nullptr;
static pXInputGetCapabilitiesEx get_capabilities_ex = nullptr;
static pXInputGetBatteryInformation get_battery_information = nullptr;
static pXInputGetBaseBusInformation get_base_bus_information = nullptr;

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

  _axes.resize(6);
  _buttons.resize(16);
}

/**
 *
 */
XInputDevice::
~XInputDevice() {
  do_set_vibration(0, 0);
}

/**
 * Called when a new input device arrives in the InputDeviceManager.  This
 * method checks whether it matches this XInput device.
 */
bool XInputDevice::
check_arrival(const RID_DEVICE_INFO &info, DEVINST inst,
              const std::string &name, const std::string &manufacturer) {
  LightMutexHolder holder(_lock);
  if (_is_connected) {
    return false;
  }

  if (!_initialized) {
    nassertr_always(init_xinput(), false);
  }

  XINPUT_CAPABILITIES_EX caps = {0};
  XINPUT_STATE state;
  if ((get_capabilities_ex && get_capabilities_ex(1, _index, 0, &caps) != ERROR_SUCCESS) &&
       get_capabilities(_index, 0, (XINPUT_CAPABILITIES *)&caps) != ERROR_SUCCESS) {
    return false;
  }

  if (get_state(_index, &state) != ERROR_SUCCESS) {
    return false;
  }

  // Extra check for VID/PID if we have it, just to be sure.
  if ((caps.VendorID != 0 && caps.VendorID != info.hid.dwVendorId) ||
      (caps.ProductID != 0 && caps.ProductID != info.hid.dwProductId)) {
    return false;
  }

  // Yes, take the name and manufacturer.
  if (!name.empty()) {
    _name = name;
  } else {
    _name = "XInput Device #";
    _name += format_string(_index + 1);
  }
  _manufacturer = manufacturer;

  if (inst && caps.ProductID == 0 && caps.RevisionID != 0) {
    // XInput does not report a product ID for the Xbox 360 wireless adapter.
    // Instead, we check that the RevisionID matches.
    char buffer[4096];
    ULONG buflen = sizeof(buffer);
    if (CM_Get_DevNode_Registry_Property(inst, CM_DRP_HARDWAREID, 0, buffer, &buflen, 0) == CR_SUCCESS) {
      std::string ids(buffer, buflen);
      char revstr[16];
      sprintf(revstr, "REV_%04x", caps.RevisionID);
      if (ids.find(revstr) == std::string::npos) {
        return false;
      }
    }
  }

  _is_connected = true;
  init_device(caps, state);
  _vendor_id = info.hid.dwVendorId;
  _product_id = info.hid.dwProductId;
  return true;
}

/**
 * Called periodically by the InputDeviceManager to detect whether the device
 * is currently connected.
 * Returns true if the device wasn't connected, but now is.
 */
void XInputDevice::
detect(InputDeviceManager *mgr) {
  if (!_initialized) {
    nassertv_always(init_xinput());
  }

  bool connected = false;

  XINPUT_CAPABILITIES_EX caps = {0};
  XINPUT_STATE state;
  if (((get_capabilities_ex && get_capabilities_ex(1, _index, 0, &caps) == ERROR_SUCCESS) ||
       get_capabilities(_index, 0, (XINPUT_CAPABILITIES *)&caps) == ERROR_SUCCESS) &&
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
    _name = "XInput Device #";
    _name += format_string(_index + 1);
    _vendor_id = caps.VendorID;
    _product_id = caps.ProductID;
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
  if (_initialized) {
    return true;
  }

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
    if (get_state == nullptr) {
      get_state = (pXInputGetState)GetProcAddress(module, "XInputGetState");
      if (get_state == nullptr) {
        device_cat.error()
          << "Failed to find function XInputGetState in " << dll_name << ".\n";
        return false;
      }
    }

    set_state = (pXInputSetState)GetProcAddress(module, "XInputSetState");
    if (set_state == nullptr) {
      device_cat.error()
        << "Failed to find function XInputSetState in " << dll_name << ".\n";
      return false;
    }

    get_capabilities = (pXInputGetCapabilities)GetProcAddress(module, "XInputGetCapabilities");
    if (get_capabilities == nullptr) {
      device_cat.error()
        << "Failed to find function XInputGetCapabilities in " << dll_name << ".\n";
      return false;
    }

    get_battery_information = (pXInputGetBatteryInformation)GetProcAddress(module, "XInputGetBatteryInformation");
    get_base_bus_information = (pXInputGetBaseBusInformation)GetProcAddress(module, MAKEINTRESOURCE(104));
    get_capabilities_ex = (pXInputGetCapabilitiesEx)GetProcAddress(module, MAKEINTRESOURCE(108));
    return true;
  }

  device_cat.error()
    << "Failed to load Xinput1_4.dll or Xinput1_3.dll.\n";
  return false;
}

/**
 * Initializes the device.  Called when the device was just connected.
 * Assumes either the lock is held or this is called from the constructor.
 */
void XInputDevice::
init_device(const XINPUT_CAPABILITIES_EX &caps, const XINPUT_STATE &state) {
  nassertv(_initialized);
  // It seems that the Xbox One controller is reported as having a DevType of
  // zero, at least when I tested in with XInput 1.3 on Windows 7.
  //if (caps.Type == XINPUT_DEVTYPE_GAMEPAD) {

  // For subtypes and mappings, see this page:
  // https://msdn.microsoft.com/en-us/library/windows/desktop/hh405050.aspx
  switch (caps.SubType) {
  default:
  case XINPUT_DEVSUBTYPE_GAMEPAD:
    _device_class = DeviceClass::gamepad;
    _axes[0].axis = Axis::left_trigger;
    _axes[1].axis = Axis::right_trigger;
    _axes[2].axis = Axis::left_x;
    _axes[3].axis = Axis::left_y;
    _axes[4].axis = Axis::right_x;
    _axes[5].axis = Axis::right_y;
    break;

  case XINPUT_DEVSUBTYPE_WHEEL:
    _device_class = DeviceClass::steering_wheel;
    _axes[0].axis = Axis::brake;
    _axes[1].axis = Axis::accelerator;
    _axes[2].axis = Axis::wheel;
    _axes[3].axis = Axis::none;
    _axes[4].axis = Axis::none;
    _axes[5].axis = Axis::none;
    break;

  case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
    _device_class = DeviceClass::flight_stick;
    _axes[0].axis = Axis::yaw;
    _axes[1].axis = Axis::throttle;
    _axes[2].axis = Axis::roll;
    _axes[3].axis = Axis::pitch;
    _axes[4].axis = Axis::none;
    _axes[5].axis = Axis::none;
    break;

  case XINPUT_DEVSUBTYPE_DANCE_PAD:
    _device_class = DeviceClass::dance_pad;
    _axes[0].axis = Axis::none;
    _axes[1].axis = Axis::none;
    _axes[2].axis = Axis::none;
    _axes[3].axis = Axis::none;
    _axes[4].axis = Axis::none;
    _axes[5].axis = Axis::none;
    break;
  }

  _axes[0]._scale = 1.0 / 255.0;
  _axes[1]._scale = 1.0 / 255.0;
  _axes[2]._scale = 1.0 / 32767.5;
  _axes[3]._scale = 1.0 / 32767.5;
  _axes[4]._scale = 1.0 / 32767.5;
  _axes[5]._scale = 1.0 / 32767.5;

  _axes[2]._bias = 0.5 / 32767.5;
  _axes[3]._bias = 0.5 / 32767.5;
  _axes[4]._bias = 0.5 / 32767.5;
  _axes[5]._bias = 0.5 / 32767.5;

  if (caps.Flags & XINPUT_CAPS_NO_NAVIGATION) {
    _buttons[0].handle = ButtonHandle::none();
    _buttons[1].handle = ButtonHandle::none();
    _buttons[2].handle = ButtonHandle::none();
    _buttons[3].handle = ButtonHandle::none();
    _buttons[4].handle = ButtonHandle::none();
    _buttons[5].handle = ButtonHandle::none();
  } else {
    _buttons[0].handle = GamepadButton::dpad_up();
    _buttons[1].handle = GamepadButton::dpad_down();
    _buttons[2].handle = GamepadButton::dpad_left();
    _buttons[3].handle = GamepadButton::dpad_right();
    _buttons[4].handle = GamepadButton::start();
    _buttons[5].handle = GamepadButton::back();
  }
  _buttons[6].handle = GamepadButton::lstick();
  _buttons[7].handle = GamepadButton::rstick();
  _buttons[8].handle = GamepadButton::lshoulder();
  _buttons[9].handle = GamepadButton::rshoulder();
  _buttons[10].handle = GamepadButton::guide();
  _buttons[11].handle = GamepadButton::face_a();
  _buttons[12].handle = GamepadButton::face_b();
  _buttons[13].handle = GamepadButton::face_x();
  _buttons[14].handle = GamepadButton::face_y();

  if (caps.Vibration.wLeftMotorSpeed != 0 ||
      caps.Vibration.wRightMotorSpeed != 0) {
    enable_feature(Feature::vibration);
  }

  if (get_battery_information != nullptr) {
    XINPUT_BATTERY_INFORMATION batt;
    if (get_battery_information(_index, BATTERY_DEVTYPE_GAMEPAD, &batt) == ERROR_SUCCESS) {
      if (batt.BatteryType != BATTERY_TYPE_DISCONNECTED &&
          batt.BatteryType != BATTERY_TYPE_WIRED) {
        // This device has a battery.  Report the battery level.
        enable_feature(Feature::battery);
        _battery_data.level = batt.BatteryLevel;
        _battery_data.max_level = BATTERY_LEVEL_FULL;
      }
    }
  }

  WORD buttons = state.Gamepad.wButtons;
  WORD mask = 1;
  for (int i = 0; i < 16; ++i) {
    // Set the state without triggering a button event.
    _buttons[i]._state = (buttons & mask) ? S_down : S_up;
    mask <<= 1;
    if (i == 10) {
      // XInput skips 0x0800.
      mask <<= 1;
    }
  }

  axis_changed(0, state.Gamepad.bLeftTrigger);
  axis_changed(1, state.Gamepad.bRightTrigger);
  axis_changed(2, state.Gamepad.sThumbLX);
  axis_changed(3, state.Gamepad.sThumbLY);
  axis_changed(4, state.Gamepad.sThumbRX);
  axis_changed(5, state.Gamepad.sThumbRY);

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
  nassertv_always(_is_connected);

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
  // Not sure why someone would call this on a disconnected device.
  if (!_is_connected) {
    return;
  }

  XINPUT_STATE state;

  if (get_state(_index, &state) != ERROR_SUCCESS) {
    // Device was disconnected.
    if (_is_connected) {
      _is_connected = false;
      InputDeviceManager *mgr = InputDeviceManager::get_global_ptr();
      mgr->remove_device(this);
    }
    return;
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
      button_changed(i, (state.Gamepad.wButtons & mask) != 0);
    }
    mask <<= 1;
    if (i == 10) {
      // XInput skips 0x0800.
      mask <<= 1;
    }
  }

  axis_changed(0, state.Gamepad.bLeftTrigger);
  axis_changed(1, state.Gamepad.bRightTrigger);
  axis_changed(2, state.Gamepad.sThumbLX);
  axis_changed(3, state.Gamepad.sThumbLY);
  axis_changed(4, state.Gamepad.sThumbRX);
  axis_changed(5, state.Gamepad.sThumbRY);

  _last_buttons = state.Gamepad.wButtons;
  _last_packet = state.dwPacketNumber;
}

#endif  // _WIN32
