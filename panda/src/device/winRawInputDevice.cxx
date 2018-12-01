/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winRawInputDevice.cxx
 * @author rdb
 * @date 2018-01-19
 */

#include "winRawInputDevice.h"
#include "gamepadButton.h"
#include "mouseButton.h"

#if defined(_WIN32) && !defined(CPPPARSER)

#include <CfgMgr32.h>
#include <devpkey.h>

// Copy definitions from hidusage.h, until we can drop support for the 7.1 SDK
typedef USHORT USAGE, *PUSAGE;

#define HID_USAGE_PAGE_UNDEFINED       ((USAGE) 0x00)
#define HID_USAGE_PAGE_GENERIC         ((USAGE) 0x01)
#define HID_USAGE_PAGE_SIMULATION      ((USAGE) 0x02)
#define HID_USAGE_PAGE_VR              ((USAGE) 0x03)
#define HID_USAGE_PAGE_SPORT           ((USAGE) 0x04)
#define HID_USAGE_PAGE_GAME            ((USAGE) 0x05)
#define HID_USAGE_PAGE_KEYBOARD        ((USAGE) 0x07)
#define HID_USAGE_PAGE_LED             ((USAGE) 0x08)
#define HID_USAGE_PAGE_BUTTON          ((USAGE) 0x09)

#define HID_USAGE_GENERIC_POINTER      ((USAGE) 0x01)
#define HID_USAGE_GENERIC_MOUSE        ((USAGE) 0x02)
#define HID_USAGE_GENERIC_JOYSTICK     ((USAGE) 0x04)
#define HID_USAGE_GENERIC_GAMEPAD      ((USAGE) 0x05)
#define HID_USAGE_GENERIC_KEYBOARD     ((USAGE) 0x06)
#define HID_USAGE_GENERIC_KEYPAD       ((USAGE) 0x07)
#define HID_USAGE_GENERIC_SYSTEM_CTL   ((USAGE) 0x80)

#define HID_USAGE_GENERIC_X            ((USAGE) 0x30)
#define HID_USAGE_GENERIC_Y            ((USAGE) 0x31)
#define HID_USAGE_GENERIC_Z            ((USAGE) 0x32)
#define HID_USAGE_GENERIC_RX           ((USAGE) 0x33)
#define HID_USAGE_GENERIC_RY           ((USAGE) 0x34)
#define HID_USAGE_GENERIC_RZ           ((USAGE) 0x35)
#define HID_USAGE_GENERIC_SLIDER       ((USAGE) 0x36)
#define HID_USAGE_GENERIC_DIAL         ((USAGE) 0x37)
#define HID_USAGE_GENERIC_WHEEL        ((USAGE) 0x38)
#define HID_USAGE_GENERIC_HATSWITCH    ((USAGE) 0x39)

// Copy definitions from hidpi.h, until we can drop support for the 7.1 SDK
#define HIDP_STATUS_SUCCESS ((NTSTATUS)(0x11 << 16))

typedef enum _HIDP_REPORT_TYPE {
  HidP_Input,
  HidP_Output,
  HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _HIDP_BUTTON_CAPS {
  USAGE UsagePage;
  UCHAR ReportID;
  BOOLEAN IsAlias;
  USHORT BitField;
  USHORT LinkCollection;
  USAGE LinkUsage;
  USAGE LinkUsagePage;
  BOOLEAN IsRange;
  BOOLEAN IsStringRange;
  BOOLEAN IsDesignatorRange;
  BOOLEAN IsAbsolute;
  ULONG Reserved[10];
  union {
    struct {
      USAGE UsageMin, UsageMax;
      USHORT StringMin, StringMax;
      USHORT DesignatorMin, DesignatorMax;
      USHORT DataIndexMin, DataIndexMax;
    } Range;
    struct  {
      USAGE Usage, Reserved1;
      USHORT StringIndex, Reserved2;
      USHORT DesignatorIndex, Reserved3;
      USHORT DataIndex, Reserved4;
    } NotRange;
  };
} HIDP_BUTTON_CAPS, *PHIDP_BUTTON_CAPS;

typedef struct _HIDP_VALUE_CAPS {
  USAGE UsagePage;
  UCHAR ReportID;
  BOOLEAN IsAlias;
  USHORT BitField;
  USHORT LinkCollection;
  USAGE LinkUsage;
  USAGE LinkUsagePage;
  BOOLEAN IsRange;
  BOOLEAN IsStringRange;
  BOOLEAN IsDesignatorRange;
  BOOLEAN IsAbsolute;
  BOOLEAN HasNull;
  UCHAR Reserved;
  USHORT BitSize;
  USHORT ReportCount;
  USHORT Reserved2[5];
  ULONG UnitsExp;
  ULONG Units;
  LONG LogicalMin, LogicalMax;
  LONG PhysicalMin, PhysicalMax;
  union {
    struct {
      USAGE UsageMin, UsageMax;
      USHORT StringMin, StringMax;
      USHORT DesignatorMin, DesignatorMax;
      USHORT DataIndexMin, DataIndexMax;
    } Range;
    struct {
      USAGE Usage, Reserved1;
      USHORT StringIndex, Reserved2;
      USHORT DesignatorIndex, Reserved3;
      USHORT DataIndex, Reserved4;
    } NotRange;
  };
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;

typedef PUCHAR PHIDP_REPORT_DESCRIPTOR;
typedef struct _HIDP_PREPARSED_DATA *PHIDP_PREPARSED_DATA;

typedef struct _HIDP_CAPS {
  USAGE Usage;
  USAGE UsagePage;
  USHORT InputReportByteLength;
  USHORT OutputReportByteLength;
  USHORT FeatureReportByteLength;
  USHORT Reserved[17];
  USHORT NumberLinkCollectionNodes;
  USHORT NumberInputButtonCaps;
  USHORT NumberInputValueCaps;
  USHORT NumberInputDataIndices;
  USHORT NumberOutputButtonCaps;
  USHORT NumberOutputValueCaps;
  USHORT NumberOutputDataIndices;
  USHORT NumberFeatureButtonCaps;
  USHORT NumberFeatureValueCaps;
  USHORT NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_DATA {
  USHORT DataIndex;
  USHORT Reserved;
  union {
    ULONG RawValue;
    BOOLEAN On;
  };
} HIDP_DATA, *PHIDP_DATA;

typedef LONG NTSTATUS;
typedef NTSTATUS (*pHidP_GetCaps)(PHIDP_PREPARSED_DATA, PHIDP_CAPS);
typedef NTSTATUS (*pHidP_GetButtonCaps)(HIDP_REPORT_TYPE, PHIDP_BUTTON_CAPS, PUSHORT, PHIDP_PREPARSED_DATA);
typedef NTSTATUS (*pHidP_GetValueCaps)(HIDP_REPORT_TYPE, PHIDP_VALUE_CAPS, PUSHORT, PHIDP_PREPARSED_DATA);
typedef NTSTATUS (*pHidP_GetData)(HIDP_REPORT_TYPE, PHIDP_DATA, PULONG, PHIDP_PREPARSED_DATA, PCHAR, ULONG);
typedef ULONG (*pHidP_MaxDataListLength)(HIDP_REPORT_TYPE, PHIDP_PREPARSED_DATA);

static pHidP_GetCaps _HidP_GetCaps = nullptr;
static pHidP_GetButtonCaps _HidP_GetButtonCaps = nullptr;
static pHidP_GetValueCaps _HidP_GetValueCaps = nullptr;
static pHidP_GetData _HidP_GetData = nullptr;
static pHidP_MaxDataListLength _HidP_MaxDataListLength = nullptr;

/**
 * Static method to initialize the HID parser library.  We load it dynamically
 * because the Windows 7.1 SDK doesn't ship hid.lib.
 */
static bool init_hidp() {
  HMODULE module = LoadLibraryA("hid.dll");
  if (module) {
    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Successfully loaded hid.dll\n";
    }

    _HidP_GetCaps = (pHidP_GetCaps)GetProcAddress(module, "HidP_GetCaps");
    _HidP_GetButtonCaps = (pHidP_GetButtonCaps)GetProcAddress(module, "HidP_GetButtonCaps");
    _HidP_GetValueCaps = (pHidP_GetValueCaps)GetProcAddress(module, "HidP_GetValueCaps");
    _HidP_GetData = (pHidP_GetData)GetProcAddress(module, "HidP_GetData");
    _HidP_MaxDataListLength = (pHidP_MaxDataListLength)GetProcAddress(module, "HidP_MaxDataListLength");

    if (_HidP_GetCaps == nullptr || _HidP_GetButtonCaps == nullptr ||
        _HidP_GetValueCaps == nullptr || _HidP_GetData == nullptr ||
        _HidP_MaxDataListLength == nullptr) {
      device_cat.error()
        << "Failed to locate function pointers in hid.dll\n";
      return false;
    }

    return true;
  }

  device_cat.error()
    << "Failed to load hid.dll.\n";
  return false;
}

/**
 * Protected constructor.  Given a raw device handle.
 */
WinRawInputDevice::
WinRawInputDevice(WinInputDeviceManager *manager, const char *path) :
  _manager(manager),
  _path(path),
  _max_data_count(0),
  _preparsed(nullptr) {
}

/**
 *
 */
WinRawInputDevice::
~WinRawInputDevice() {
  // Unregister the device from the manager.
  LightMutexHolder holder(_lock);
  if (_manager != nullptr) {
    _manager->device_destroyed(this);
  }
  if (_preparsed != nullptr) {
    free(_preparsed);
    _preparsed = nullptr;
  }
}

/**
 * Called by InputDeviceManager when this device is connected.  Returns true
 * if the device was connected successfully.
 */
bool WinRawInputDevice::
on_arrival(HANDLE handle, const RID_DEVICE_INFO &info, std::string name) {
  using std::hex;
  using std::dec;
  using std::swap;

  LightMutexHolder holder(_lock);

  _name = std::move(name);

  switch (info.dwType) {
  case RIM_TYPEMOUSE:
    _device_class = DeviceClass::mouse;
    break;

  case RIM_TYPEKEYBOARD:
    _device_class = DeviceClass::keyboard;
    break;

  case RIM_TYPEHID:
    _vendor_id = info.hid.dwVendorId;
    _product_id = info.hid.dwProductId;

    // Gamepads
    if (info.hid.usUsagePage == HID_USAGE_PAGE_GENERIC &&
        info.hid.usUsage == HID_USAGE_GENERIC_GAMEPAD) {
      _device_class = DeviceClass::gamepad;

    // Flight sticks
    } else if (info.hid.usUsagePage == HID_USAGE_PAGE_GENERIC &&
               info.hid.usUsage == HID_USAGE_GENERIC_JOYSTICK) {
      _device_class = DeviceClass::flight_stick;

      if (_name == "usb gamepad") {
        // Well, it claims to be a gamepad...
        _device_class = DeviceClass::gamepad;
      }

    // Mice
    } else if (info.hid.usUsagePage == HID_USAGE_PAGE_GENERIC &&
               info.hid.usUsage == HID_USAGE_GENERIC_MOUSE) {
      _device_class = DeviceClass::mouse;

    // Keyboards
    } else if (info.hid.usUsagePage == HID_USAGE_PAGE_GENERIC &&
               info.hid.usUsage == HID_USAGE_GENERIC_KEYBOARD) {
      _device_class = DeviceClass::keyboard;

    // 3Dconnexion SpaceNavigator and friends.
    } else if (_vendor_id == 0x046d &&
        (_product_id == 0xc623 ||
         _product_id == 0xc625 ||
         _product_id == 0xc626 ||
         _product_id == 0xc627 ||
         _product_id == 0xc628 ||
         _product_id == 0xc629 ||
         _product_id == 0xc62b)) {
      _device_class = DeviceClass::spatial_mouse;
    }
    break;

  default:
    return false;
  }

  // Initialize hid.dll, which provides the HID parser functions.
  static bool hid_initialized = false;
  if (!hid_initialized) {
    if (!init_hidp()) {
      return false;
    }
    hid_initialized = true;
  }

  // Get the "preparsed data", which we can parse with the HID parser API.
  UINT size = 0;
  if (GetRawInputDeviceInfo(handle, RIDI_PREPARSEDDATA, nullptr, &size) < 0) {
    return false;
  }

  PHIDP_PREPARSED_DATA buffer = (PHIDP_PREPARSED_DATA)malloc(size);
  if (GetRawInputDeviceInfo(handle, RIDI_PREPARSEDDATA, buffer, &size) <= 0) {
    return false;
  }
  _preparsed = buffer;

  HIDP_CAPS caps;
  if (_HidP_GetCaps(buffer, &caps) != HIDP_STATUS_SUCCESS) {
    device_cat.warning()
      << "Failed to get capabilities from HID preparsed data.\n";
    return false;
  }

  // Do we have a button mapping?
  static const ButtonHandle gamepad_buttons_common[] = {
    ButtonHandle::none(),
    GamepadButton::face_a(),
    GamepadButton::face_b(),
    GamepadButton::face_x(),
    GamepadButton::face_y(),
    GamepadButton::lshoulder(),
    GamepadButton::rshoulder(),
    GamepadButton::start(),
    GamepadButton::back(),
    GamepadButton::lstick(),
    GamepadButton::rstick(),
  };
  const ButtonHandle *gamepad_buttons = gamepad_buttons_common;
  if (_vendor_id == 0x0810 && _product_id == 0xe501) {
    // SNES-style USB gamepad
    static const ButtonHandle gamepad_buttons_snes[] = {
      ButtonHandle::none(),
      GamepadButton::face_x(),
      GamepadButton::face_a(),
      GamepadButton::face_b(),
      GamepadButton::face_y(),
      GamepadButton::lshoulder(),
      GamepadButton::rshoulder(),
      ButtonHandle::none(),
      ButtonHandle::none(),
      GamepadButton::back(),
      GamepadButton::start(),
    };
    gamepad_buttons = gamepad_buttons_snes;
  }

  // Prepare a mapping of data indices to button/axis indices.
  _indices.resize(caps.NumberInputDataIndices);

  _buttons.clear();
  _axes.clear();

  USHORT num_button_caps = caps.NumberInputButtonCaps;
  PHIDP_BUTTON_CAPS button_caps = (PHIDP_BUTTON_CAPS)alloca(num_button_caps * sizeof(HIDP_BUTTON_CAPS));
  _HidP_GetButtonCaps(HidP_Input, button_caps, &num_button_caps, buffer);

  for (USHORT i = 0; i < num_button_caps; ++i) {
    HIDP_BUTTON_CAPS &cap = button_caps[i];
    int upper = 0;
    if (cap.IsRange) {
      upper = (cap.Range.UsageMax - cap.Range.UsageMin);

      if (device_cat.is_debug()) {
        device_cat.debug()
          << "Found button range: DataIndex=" << dec
          << cap.Range.DataIndexMin << ".." << cap.Range.DataIndexMax
          << ", ReportID=" << (int)cap.ReportID
          << ", UsagePage=0x" << hex << cap.UsagePage
          << ", Usage=0x" << cap.Range.UsageMin << "..0x" << cap.Range.UsageMax
          << dec << "\n";
      }
    } else {
      if (device_cat.is_debug()) {
        device_cat.debug()
          << "Found button: DataIndex=" << dec << cap.NotRange.DataIndex
          << ", ReportID=" << dec << (int)cap.ReportID
          << ", UsagePage=0x" << cap.UsagePage
          << ", Usage=0x" << cap.NotRange.Usage
          << dec << "\n";
      }
    }

    // Windows will only tell us which buttons in a report are "on", so we
    // need to keep track of which buttons exist in which report so that we
    // can figure out which ones are off.
    if (cap.ReportID >= _report_buttons.size()) {
      _report_buttons.resize(cap.ReportID + 1);
    }
    for (int j = 0; j <= upper; ++j) {
      USAGE usage = j + cap.Range.UsageMin;
      USHORT data_index = j + cap.Range.DataIndexMin;
      ButtonHandle handle = ButtonHandle::none();
      switch (cap.UsagePage) {
      case HID_USAGE_PAGE_BUTTON:
        if (_device_class == DeviceClass::gamepad) {
          if (usage < sizeof(gamepad_buttons_common) / sizeof(ButtonHandle)) {
            handle = gamepad_buttons[usage];
          }
        } else if (_device_class == DeviceClass::flight_stick) {
          if (usage > 0) {
            handle = GamepadButton::joystick(usage - 1);
          }
        } else if (_device_class == DeviceClass::mouse) {
          // In Panda, wheel and right button are flipped around...
          int button = (usage == 2 || usage == 3) ? (4 - usage) : (usage - 1);
          handle = MouseButton::button(button);
        }
        break;
      }

      int button_index = _buttons.size();
      _report_buttons[cap.ReportID].set_bit(button_index);
      _indices[data_index] = Index::button(button_index);
      _buttons.push_back(ButtonState(handle));
    }
  }

  USHORT num_value_caps = caps.NumberInputValueCaps;
  PHIDP_VALUE_CAPS value_caps = (PHIDP_VALUE_CAPS)alloca(num_value_caps * sizeof(HIDP_VALUE_CAPS));
  _HidP_GetValueCaps(HidP_Input, value_caps, &num_value_caps, buffer);

  _hat_data_index = -1;

  for (USHORT i = 0; i < num_value_caps; ++i) {
    HIDP_VALUE_CAPS &cap = value_caps[i];
    int upper = 0;
    if (cap.IsRange) {
      upper = (cap.Range.UsageMax - cap.Range.UsageMin);

      if (device_cat.is_debug()) {
        device_cat.debug()
          << "Found value range: DataIndex=" << dec
          << cap.Range.DataIndexMin << ".." << cap.Range.DataIndexMax
          << ", ReportID=" << (int)cap.ReportID
          << ", UsagePage=0x" << hex << cap.UsagePage
          << ", Usage=0x" << cap.Range.UsageMin << "..0x" << cap.Range.UsageMax
          << dec << ", LogicalMin=" << cap.LogicalMin
          << ", LogicalMax=" << cap.LogicalMax << "\n";
      }
    } else {
      if (device_cat.is_debug()) {
        device_cat.debug()
          << "Found value: DataIndex=" << dec << cap.NotRange.DataIndex
          << ", ReportID=" << dec << (int)cap.ReportID
          << ", UsagePage=0x" << hex << cap.UsagePage
          << ", Usage=0x" << cap.NotRange.Usage
          << dec << ", LogicalMin=" << cap.LogicalMin
          << ", LogicalMax=" << cap.LogicalMax << "\n";
      }
    }

    for (int j = 0; j <= upper; ++j) {
      USAGE usage = j + cap.Range.UsageMin;
      USHORT data_index = j + cap.Range.DataIndexMin;
      bool is_signed = true;

      // My gamepads give this odd invalid range.
      if (cap.LogicalMin == 0 && cap.LogicalMax == -1) {
        cap.LogicalMax = 65535;
        is_signed = false;
      }

      Axis axis = Axis::none;
      switch (cap.UsagePage) {
      case HID_USAGE_PAGE_GENERIC:
        switch (usage) {
          case HID_USAGE_GENERIC_X:
          if (_device_class == DeviceClass::gamepad) {
            axis = Axis::left_x;
          } else if (_device_class == DeviceClass::flight_stick) {
            axis = Axis::roll;
          } else {
            axis = Axis::x;
          }
          break;
        case HID_USAGE_GENERIC_Y:
          if (_device_class == DeviceClass::gamepad) {
            axis = Axis::left_y;
            swap(cap.LogicalMin, cap.LogicalMax);
          } else if (_device_class == DeviceClass::flight_stick) {
            axis = Axis::pitch;
          } else {
            axis = Axis::y;
            swap(cap.LogicalMin, cap.LogicalMax);
          }
          break;
        case HID_USAGE_GENERIC_Z:
          if (_device_class == DeviceClass::gamepad) {
            axis = Axis::left_trigger;
          } else if (_device_class == DeviceClass::flight_stick) {
            axis = Axis::throttle;
          } else {
            axis = Axis::z;
            swap(cap.LogicalMin, cap.LogicalMax);
          }
          break;
        case HID_USAGE_GENERIC_RX:
          if (_device_class == DeviceClass::gamepad) {
            axis = Axis::right_x;
          } else {
            axis = Axis::pitch;
          }
          break;
        case HID_USAGE_GENERIC_RY:
          if (_device_class == DeviceClass::gamepad) {
            axis = Axis::right_y;
          } else {
            axis = Axis::roll;
          }
          swap(cap.LogicalMin, cap.LogicalMax);
          break;
        case HID_USAGE_GENERIC_RZ:
          if (_device_class == DeviceClass::gamepad) {
            axis = Axis::right_trigger;
          } else {
            // Flip to match Panda's convention for heading.
            axis = Axis::yaw;
            swap(cap.LogicalMin, cap.LogicalMax);
          }
          break;
        case HID_USAGE_GENERIC_SLIDER:
          // Flip to match Panda's convention for heading.
          axis = Axis::rudder;
          swap(cap.LogicalMin, cap.LogicalMax);
          break;
        case HID_USAGE_GENERIC_WHEEL:
          axis = Axis::wheel;
          break;
        case HID_USAGE_GENERIC_HATSWITCH:
          // This is handled specially.
          _hat_data_index = data_index;
          _hat_data_minimum = cap.LogicalMin;
          continue;
        }
        break;
      }

      int axis_index;
      if (_vendor_id == 0x044f && _product_id == 0xb108 && axis == Axis::throttle) {
        // T.Flight Hotas X throttle is reversed and can go backwards.
        axis_index = add_axis(axis, cap.LogicalMax, cap.LogicalMin, true);
      } else if (!is_signed) {
        // All axes on the weird XInput-style mappings go from -1 to 1
        axis_index = add_axis(axis, cap.LogicalMin, cap.LogicalMax, true);
      } else {
        axis_index = add_axis(axis, cap.LogicalMin, cap.LogicalMax);
      }
      _indices[data_index] = Index::axis(axis_index, is_signed);
    }
  }

  // Do we need to emulate a hat switch or directional pad?
  if (_hat_data_index != -1) {
    _hat_left_button = (int)_buttons.size();
    if (_device_class == DeviceClass::gamepad) {
      _buttons.push_back(ButtonState(GamepadButton::dpad_left()));
      _buttons.push_back(ButtonState(GamepadButton::dpad_right()));
      _buttons.push_back(ButtonState(GamepadButton::dpad_down()));
      _buttons.push_back(ButtonState(GamepadButton::dpad_up()));
    } else {
      _buttons.push_back(ButtonState(GamepadButton::hat_left()));
      _buttons.push_back(ButtonState(GamepadButton::hat_right()));
      _buttons.push_back(ButtonState(GamepadButton::hat_down()));
      _buttons.push_back(ButtonState(GamepadButton::hat_up()));
    }
  }

  _max_data_count = _HidP_MaxDataListLength(HidP_Input, buffer);

  _handle = handle;
  _is_connected = true;
  return true;
}

/**
 * Called by InputDeviceManager when this device is removed.
 */
void WinRawInputDevice::
on_removal() {
  LightMutexHolder holder(_lock);
  _is_connected = false;
  _handle = nullptr;
  if (_preparsed != nullptr) {
    delete _preparsed;
    _preparsed = nullptr;
  }
  _indices.clear();
  _report_buttons.clear();
}

void WinRawInputDevice::
on_input(PRAWINPUT input) {
  nassertv(input != nullptr);
  nassertv(_preparsed != nullptr);

  BYTE *ptr = input->data.hid.bRawData;
  if (input->data.hid.dwSizeHid == 0) {
    return;
  }

  PHIDP_DATA data = (PHIDP_DATA)alloca(sizeof(HIDP_DATA) * _max_data_count);
  nassertv(data != nullptr);
  ULONG count;

  LightMutexHolder holder(_lock);

  for (DWORD i = 0; i < input->data.hid.dwCount; ++i) {
    // The first byte is the report identifier.  We need it to figure out
    // which buttons are off, since each report only contains the buttons that
    // are "on".
    UCHAR report_id = ptr[0];
    BitArray unset_buttons = _report_buttons[report_id];

    count = _max_data_count;
    NTSTATUS status = _HidP_GetData(HidP_Input, data, &count, (PHIDP_PREPARSED_DATA)_preparsed, (PCHAR)ptr, input->data.hid.dwSizeHid);
    if (status == HIDP_STATUS_SUCCESS) {
      for (ULONG di = 0; di < count; ++di) {
        if (data[di].DataIndex != _hat_data_index) {
          const Index &idx = _indices[data[di].DataIndex];
          if (idx._axis >= 0) {
            if (idx._signed) {
              axis_changed(idx._axis, (SHORT)data[di].RawValue);
            } else {
              axis_changed(idx._axis, data[di].RawValue);
            }
          }
          if (idx._button >= 0) {
            unset_buttons.clear_bit(idx._button);
            button_changed(idx._button, (data[di].On != FALSE));
          }
        } else {
          int value = (int)data[di].RawValue - _hat_data_minimum;
          button_changed(_hat_left_button + 0, value >= 5 && value <= 7); // left
          button_changed(_hat_left_button + 1, value >= 1 && value <= 3); // right
          button_changed(_hat_left_button + 2, value >= 3 && value <= 5); // down
          button_changed(_hat_left_button + 3, value == 7 || value == 0 || value == 1); // up
        }
      }

      // Now unset the buttons in this report that aren't pressed.
      int button_index = unset_buttons.get_lowest_on_bit();
      while (button_index >= 0) {
        button_changed(button_index, false);
        unset_buttons.clear_bit(button_index);
        button_index = unset_buttons.get_lowest_on_bit();
      }
    } else if (device_cat.is_spam()) {
      device_cat.spam()
        << "Failed to get data from raw device " << _path
        << " (error 0x" << std::hex << (status & 0xffffffffu) << std::dec << ")\n";
    }

    ptr += input->data.hid.dwSizeHid;
  }
}

/**
 * Polls the input device for new activity, to ensure it contains the latest
 * events.  This will only have any effect for some types of input devices;
 * others may be updated automatically, and this method will be a no-op.
 */
void WinRawInputDevice::
do_poll() {
}

#endif  // _WIN32
