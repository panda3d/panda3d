/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winInputDeviceManager.cxx
 * @author rdb
 * @date 2018-01-21
 */

#include "winInputDeviceManager.h"
#include "winRawInputDevice.h"
#include "throw_event.h"

#if defined(_WIN32) && !defined(CPPPARSER)

#ifdef HAVE_THREADS
/**
 *
 */
class InputThread : public Thread {
public:
  InputThread(WinInputDeviceManager *manager) :
    Thread("input", "input"), _manager(manager) {}

private:
  virtual void thread_main();

  WinInputDeviceManager *_manager;
};
#endif

/**
 * Initializes the input device manager by scanning which devices are currently
 * connected and setting up any platform-dependent structures necessary for
 * listening for future device connect events.
 */
WinInputDeviceManager::
WinInputDeviceManager() :
  _xinput_device0(0),
  _xinput_device1(1),
  _xinput_device2(2),
  _xinput_device3(3),
  _message_hwnd(nullptr) {

  // XInput provides four device slots, so we simply create four XInputDevice
  // objects that are bound to the lifetime of the input manager.
  _xinput_device0.local_object();
  _xinput_device1.local_object();
  _xinput_device2.local_object();
  _xinput_device3.local_object();

  // This function is only available in Vista and later, so we use a wrapper.
  HMODULE module = LoadLibraryA("cfgmgr32.dll");
  if (module) {
    _CM_Get_DevNode_PropertyW = (pCM_Get_DevNode_Property)GetProcAddress(module, "CM_Get_DevNode_PropertyW");
  } else {
    _CM_Get_DevNode_PropertyW = nullptr;
  }

  // If we have threading enabled, start a thread with a message-only window
  // loop to listen for input events.  We can't actually just let this be
  // handled by the main window loop, because the main window might actually
  // have been created in a different thread.
#ifdef HAVE_THREADS
  if (Thread::is_threading_supported()) {
    PT(Thread) thread = new InputThread(this);
    thread->start(TP_normal, false);
  } else
#endif
  {
    setup_message_loop();
  }
}

/**
 * Closes any resources that the device manager was using to listen for events.
 */
WinInputDeviceManager::
~WinInputDeviceManager() {
  if (_message_hwnd != nullptr) {
#ifdef HAVE_THREADS
    if (Thread::is_threading_supported()) {
      HWND hwnd = _message_hwnd;
      if (hwnd) {
        SendMessage(hwnd, WM_QUIT, 0, 0);
      }
    } else
#endif
    {
      destroy_message_loop();
    }
  }
}

/**
 * Called by the raw input device destructor.
 */
void WinInputDeviceManager::
device_destroyed(WinRawInputDevice *device) {
  LightMutexHolder holder(_lock);
  // It shouldn't be in here, but let's check to be sure.
  if (device->_handle != nullptr) {
    _raw_devices.erase(device->_handle);
  }

  _raw_devices_by_path.erase(device->_path);
}

/**
 * Called upon receiving a WM_INPUT message.
 */
void WinInputDeviceManager::
on_input(HRAWINPUT handle) {
  UINT size;
  if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) < 0) {
    return;
  }

  PRAWINPUT data = (PRAWINPUT)alloca(size);
  if (GetRawInputData(handle, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER)) <= 0) {
    return;
  }

  // Look up the device in the map.
  PT(WinRawInputDevice) device;
  {
    LightMutexHolder holder(_lock);
    auto it = _raw_devices.find(data->header.hDevice);
    if (it != _raw_devices.end()) {
      device = it->second;
    }
  }
  if (device != nullptr) {
    device->on_input(data);
  }
}

/**
 * Called upon receiving WM_INPUT_DEVICE_CHANGE with wparam GIDC_ARRIVAL.
 */
void WinInputDeviceManager::
on_input_device_arrival(HANDLE handle) {
  // Get the device path.
  UINT size;
  if (GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, nullptr, &size) != 0) {
    return;
  }

  char *path = (char *)alloca(size);
  if (path == nullptr ||
      GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, (void *)path, &size) < 0) {
    return;
  }

  if (device_cat.is_debug()) {
    device_cat.debug()
      << "GIDC_ARRIVAL: " << path << "\n";
  }

  // Get the device info.
  RID_DEVICE_INFO info;
  info.cbSize = sizeof(RID_DEVICE_INFO);
  size = sizeof(RID_DEVICE_INFO);
  if (GetRawInputDeviceInfoA(handle, RIDI_DEVICEINFO, &info, &size) <= 0) {
    return;
  }

  // Strip the \\?\ prefix from the path.
  while (path[0] == '\\' || path[0] == '?' || path[0] == '.') {
    ++path;
  }

  // Now, replace # with \\ in the path, but only up to three components.
  char *p = path;
  int i = 0;
  while (*p != '\0') {
    if (*p == '#') {
      if (i++ < 2) {
        *p = '\\';
      } else {
        *p = '\0';
        break;
      }
    }
    ++p;
  }

  // Find the device node, which will be something like "HID\VID_0123..."
  // Then we walk the device tree upward to get the USB node, which which will
  // be something like a "USB\VID..." node, from which we can fetch the real
  // USB device information (such as the product name).
  std::string name, manufacturer;
  DEVINST inst;
  CONFIGRET ret = CM_Locate_DevNodeA(&inst, (DEVINSTID_A)path, CM_LOCATE_DEVNODE_PHANTOM);
  if (ret == CR_SUCCESS) {
    char buffer[4096];
    ULONG buflen = 4096;
    if (CM_Get_DevNode_Registry_Property(inst, CM_DRP_DEVICEDESC, 0, buffer, &buflen, 0) == CR_SUCCESS) {
      name.assign(buffer);
    }
    buflen = 4096;
    if (CM_Get_DevNode_Registry_Property(inst, CM_DRP_MFG, 0, buffer, &buflen, 0) == CR_SUCCESS) {
      if (strcmp(buffer, "(Standard system devices)") != 0) {
        manufacturer.assign(buffer);
      }
    }

    // Now walk the device tree upwards so fetch the bus-reported name of the
    // parent USB device, which we prefer over the regular device description
    // that is probably boring like "HID-compliant game controller".
    DEVINST cur = inst;
    DEVINST parent;
    while (CM_Get_Parent(&parent, cur, 0) == CR_SUCCESS) {
      buflen = 4096;
      std::string dev_class;
      if (CM_Get_DevNode_Registry_Property(parent, CM_DRP_CLASS, 0, buffer, &buflen, 0) == CR_SUCCESS) {
        if (strcmp(buffer, "USB") == 0) {
          // This is some generic USB device, like a hub.  We've gone too far.
          break;
        }
        dev_class.assign(buffer);
      }
      cur = parent;

      // While we're at it, maybe this one defines a manufacturer?
      buflen = 4096;
      if (manufacturer.empty() &&
          CM_Get_DevNode_Registry_Property(cur, CM_DRP_MFG, 0, buffer, &buflen, 0) == CR_SUCCESS) {
        if (strcmp(buffer, "(Standard system devices)") != 0) {
          manufacturer.assign(buffer);
        }
      }

      // If it's a generic HID device, take the name from the USB bus.
      // See devpkey.h for the available property keys.
      static const DEVPROPKEY bus_reported_device_desc = {
        {0x540b947e, 0x8b40, 0x45bc, {0xa8, 0xa2, 0x6a, 0x0b, 0x89, 0x4c, 0xbd, 0xa2}},
        4,
      };
      DEVPROPTYPE type;
      buflen = 4096;
      if (dev_class == "HIDClass" && _CM_Get_DevNode_PropertyW != nullptr &&
          _CM_Get_DevNode_PropertyW(cur, &bus_reported_device_desc, &type, (PBYTE)buffer, &buflen, 0) == CR_SUCCESS &&
          type == DEVPROP_TYPE_STRING) {

        // Some devices insert quite some trailing space here.
        wchar_t *wbuffer = (wchar_t *)buffer;
        size_t wlen = wcsnlen_s(wbuffer, sizeof(buffer) / sizeof(wchar_t));
        while (wlen > 0 && iswspace(wbuffer[wlen - 1])) {
          wbuffer[--wlen] = 0;
        }
        TextEncoder encoder;
        name.assign(encoder.encode_wtext(std::wstring(wbuffer, wlen)));
        break;
      } else {
        buflen = 4096;
        if (CM_Get_DevNode_Registry_Property(cur, CM_DRP_DEVICEDESC, 0, buffer, &buflen, 0) == CR_SUCCESS) {
          // We'll pass if it has this awfully boring name.  Is there a
          // language-independent way to check this?
          if (strcmp(buffer, "USB Input Device") != 0) {
            name.assign(buffer);
          }
        }
      }
    }
  } else if (device_cat.is_debug()) {
    // No big deal, we just won't be able to get the name.
    device_cat.debug()
      << "Could not locate device node " << path << " (" << ret << ")\n";
  }

  // Is this an XInput device?  If so, handle it via XInput, which allows us
  // to handle independent left/right triggers as well as vibration output.
  if (info.dwType == RIM_TYPEHID && strstr(path, "&IG_") != nullptr &&
      XInputDevice::init_xinput()) {
    // This is a device we should handle via the XInput API.  Check which of
    // the four players was the lucky one.
    if (_xinput_device0.check_arrival(info, inst, name, manufacturer)) {
      add_device(&_xinput_device0);
    }
    if (_xinput_device1.check_arrival(info, inst, name, manufacturer)) {
      add_device(&_xinput_device1);
    }
    if (_xinput_device2.check_arrival(info, inst, name, manufacturer)) {
      add_device(&_xinput_device2);
    }
    if (_xinput_device3.check_arrival(info, inst, name, manufacturer)) {
      add_device(&_xinput_device3);
    }
    return;
  }

  LightMutexHolder holder(_lock);

  // Do we have a device by this path already?  This can happen if the
  // user keeps around a pointer to a disconnected device in the hope that
  // it will reconnect later.
  PT(WinRawInputDevice) device;
  auto it = _raw_devices_by_path.find(path);
  if (it != _raw_devices_by_path.end()) {
    device = it->second;
  } else {
    device = new WinRawInputDevice(this, path);
    _raw_devices_by_path[path] = device;
  }

  if (device->on_arrival(handle, info, move(name))) {
    _raw_devices[handle] = device;
    _connected_devices.add_device(device);

    if (device_cat.is_debug()) {
      device_cat.debug()
        << "Discovered input device " << *device << "\n";
    }
    throw_event("connect-device", device.p());
  }
}

/**
 * Called upon receiving WM_INPUT_DEVICE_CHANGE with wparam GIDC_REMOVAL.
 */
void WinInputDeviceManager::
on_input_device_removal(HANDLE handle) {
  // The handle will probably no longer be valid after this, so find the
  // device and remove it from _raw_devices.  However, we keep it in
  // _raw_devices_by_path in case there's still a pointer around to it.

  // We keep the pointer outside the lock because the input device
  // destructor calls back to InputDeviceManager.
  PT(WinRawInputDevice) device;
  {
    LightMutexHolder holder(_lock);
    auto it = _raw_devices.find(handle);
    if (it != _raw_devices.end()) {
      device = std::move(it->second);
      _raw_devices.erase(it);
      device->on_removal();

      if (_connected_devices.remove_device(device)) {
        throw_event("disconnect-device", device.p());
      }
      if (device_cat.is_debug()) {
        device_cat.debug()
          << "Removed input device " << *device << "\n";
      }
    }
  }
}

/**
 * Polls the system to see if there are any new devices.  In some
 * implementations this is a no-op.
 */
void WinInputDeviceManager::
update() {
}

/**
 * Sets up a Windows message loop.  Should be called from the thread that will
 * be handling the messages.
 */
HWND WinInputDeviceManager::
setup_message_loop() {
  _message_hwnd = 0;

  // Now create a message-only window for the raw input.
  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = window_proc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = "InputDeviceManager";
  if (!RegisterClassEx(&wc)) {
    device_cat.warning()
      << "Failed to register message-only window class for input device detection.\n";
  } else {
    _message_hwnd = CreateWindowEx(0, wc.lpszClassName, "InputDeviceManager", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, nullptr, nullptr);
    if (!_message_hwnd) {
      device_cat.warning()
        << "Failed to create message-only window for input device detection.\n";
    }
  }

  // Now listen for raw input devices using the created message loop.
  RAWINPUTDEVICE rid[3];
  rid[0].usUsagePage = 1;
  rid[0].usUsage = 4; // Joysticks
  rid[0].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
  rid[0].hwndTarget = _message_hwnd;
  rid[1].usUsagePage = 1;
  rid[1].usUsage = 5; // Gamepads
  rid[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
  rid[1].hwndTarget = _message_hwnd;
  rid[2].usUsagePage = 1;
  rid[2].usUsage = 8; // Multi-axis controllers (including 3D mice)
  rid[2].dwFlags = RIDEV_DEVNOTIFY | RIDEV_INPUTSINK;
  rid[2].hwndTarget = _message_hwnd;
  if (!RegisterRawInputDevices(rid, 3, sizeof(RAWINPUTDEVICE))) {
    device_cat.warning()
      << "Failed to register raw input devices.\n";
  }

  // Do we have any XInput devices plugged in now?
  int num_xinput = 0;
  HANDLE xinput_handle;
  RAWINPUTDEVICELIST devices[64];
  UINT num_devices = 64;
  num_devices = GetRawInputDeviceList(devices, &num_devices, sizeof(RAWINPUTDEVICELIST));
  if (num_devices == (UINT)-1) {
    num_devices = 0;
  }
  for (UINT i = 0; i < num_devices; ++i) {
    if (devices[i].dwType != RIM_TYPEHID) {
      continue;
    }
    HANDLE handle = devices[i].hDevice;
    UINT size;
    if (GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, nullptr, &size) != 0) {
      continue;
    }

    char *path = (char *)alloca(size);
    if (path == nullptr ||
        GetRawInputDeviceInfoA(handle, RIDI_DEVICENAME, (void *)path, &size) < 0) {
      continue;
    }

    if (strstr(path, "&IG_") != nullptr) {
      xinput_handle = handle;
      ++num_xinput;
    }
  }
  if (num_xinput == 1) {
    // There's only one XInput device, so we know which one it is.
    on_input_device_arrival(xinput_handle);
  } else if (num_xinput > 0) {
    // Just poll all the XInput devices.
    _xinput_device0.detect(this);
    _xinput_device1.detect(this);
    _xinput_device2.detect(this);
    _xinput_device3.detect(this);
  }

  return _message_hwnd;
}

/**
 * Tears down the message loop.  Should be called from the thread that called
 * setup_message_loop().
 */
void WinInputDeviceManager::
destroy_message_loop() {
  HWND hwnd = nullptr;
  {
    LightMutexHolder holder(_lock);
    std::swap(_message_hwnd, hwnd);
  }

  if (hwnd) {
    DestroyWindow(hwnd);
  }
}

/**
 * Sends a signal to the thread input thread, asking it to shut itself down.
 */
void WinInputDeviceManager::
stop_thread() {
#ifdef HAVE_THREADS
  WinInputDeviceManager *mgr = (WinInputDeviceManager *)_global_ptr;
  if (mgr != nullptr) {
    LightMutexHolder holder(mgr->_lock);
    HWND hwnd = mgr->_message_hwnd;
    if (hwnd) {
      PostMessage(hwnd, WM_QUIT, 0, 0);
    }
  }
#endif
}

/**
 * Implementation of the message loop.
 */
LRESULT WINAPI WinInputDeviceManager::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinInputDeviceManager *mgr;
  switch (msg) {
  case WM_INPUT:
    mgr = (WinInputDeviceManager *)InputDeviceManager::get_global_ptr();
    if (mgr != nullptr) {
      mgr->on_input((HRAWINPUT)lparam);
    }
    break;

  case WM_INPUT_DEVICE_CHANGE:
    switch (LOWORD(wparam)) {
    case GIDC_ARRIVAL:
      mgr = (WinInputDeviceManager *)InputDeviceManager::get_global_ptr();
      if (mgr != nullptr) {
        mgr->on_input_device_arrival((HANDLE)lparam);
      }
      break;

    case GIDC_REMOVAL:
      mgr = (WinInputDeviceManager *)InputDeviceManager::get_global_ptr();
      if (mgr != nullptr) {
        mgr->on_input_device_removal((HANDLE)lparam);
      }
      break;
    }
    break;

  default:
    break;
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

#ifdef HAVE_THREADS
/**
 * Thread entry point for the input listener thread.
 */
void InputThread::
thread_main() {
  WinInputDeviceManager *manager = _manager;
  HWND hwnd = manager->setup_message_loop();
  if (!hwnd) {
    return;
  }

  if (device_cat.is_debug()) {
    device_cat.debug()
      << "Started input device listener thread.\n";
  }

  MSG msg;
#ifdef SIMPLE_THREADS
  // In the simple threading case, we can't block the thread waiting for a
  // message; we yield control back if there are no more messages.
  while (true) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      Thread::force_yield();
    }
  }
#else
  while (GetMessage(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
#endif

  if (device_cat.is_debug()) {
    device_cat.debug()
      << "Stopping input device listener thread.\n";
  }

  manager->destroy_message_loop();
}
#endif  // HAVE_THREADS

#endif
