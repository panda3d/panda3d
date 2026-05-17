/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gameInputManager.cxx
 * @author thetestgame
 * @date 2026-03-16
 */

#include "gameInputManager.h"
#include "gameInputDevice.h"
#include "throw_event.h"

#if defined(_WIN32) && defined(HAVE_GAMEINPUT) && !defined(CPPPARSER)

/**
 * Initializes the GameInput API and registers device and reading callbacks.
 */
GameInputManager::
GameInputManager() :
  _game_input(nullptr),
  _device_callback_token(GAMEINPUT_INVALID_CALLBACK_TOKEN_VALUE) {

  HRESULT hr = GameInputCreate(&_game_input);
  if (FAILED(hr) || _game_input == nullptr) {
    device_cat.error()
      << "Failed to create GameInput instance (HRESULT 0x"
      << std::hex << hr << std::dec << ").\n";
    return;
  }

  if (device_cat.is_debug()) {
    device_cat.debug()
      << "Successfully initialized GameInput API.\n";
  }

  // Register a device callback to be notified of device connections and
  // disconnections.  We ask for all supported game device kinds.
  GameInputKind filter = GameInputKindGamepad |
                         GameInputKindFlightStick |
                         GameInputKindRacingWheel;

  hr = _game_input->RegisterDeviceCallback(
      nullptr,                            // device filter (nullptr = all)
      filter,                             // input kinds
      GameInputDeviceConnected,           // status filter
      GameInputBlockingEnumeration,       // enumerate existing devices synchronously
      this,                               // context
      &device_callback_trampoline,
      &_device_callback_token);

  if (FAILED(hr)) {
    device_cat.warning()
      << "Failed to register GameInput device callback (HRESULT 0x"
      << std::hex << hr << std::dec << ").\n";
  }

  // Process any devices that were queued during blocking enumeration.
  update();
}

/**
 * Tears down the GameInput API, unregistering callbacks and releasing
 * resources.
 */
GameInputManager::
~GameInputManager() {
  if (_game_input != nullptr) {
    if (_device_callback_token != GAMEINPUT_INVALID_CALLBACK_TOKEN_VALUE) {
      _game_input->UnregisterCallback(_device_callback_token, 5000);
      _device_callback_token = GAMEINPUT_INVALID_CALLBACK_TOKEN_VALUE;
    }

    // Release all tracked devices.
    {
      LightMutexHolder holder(_lock);
      _devices.clear();
    }

    _game_input->Release();
    _game_input = nullptr;
  }
}

/**
 * Finds the GameInputDevice wrapper for the given IGameInputDevice.
 * Assumes the lock is held.
 */
GameInputDevice *GameInputManager::
find_device(IGameInputDevice *gi_device) {
  auto it = _devices.find(gi_device);
  if (it != _devices.end()) {
    return it->second;
  }
  return nullptr;
}

/**
 * Called when a device is connected or disconnected.  This fires on a
 * GameInput thread, so we cannot use any Panda APIs here.  Instead we
 * queue the event to be processed later on the main thread in update().
 */
void GameInputManager::
on_device_callback(GameInputCallbackToken token,
                   IGameInputDevice *device,
                   uint64_t timestamp,
                   GameInputDeviceStatus current_status,
                   GameInputDeviceStatus previous_status) {

  // AddRef the device so it stays alive until we process the event.
  device->AddRef();

  std::lock_guard<std::mutex> guard(_cb_mutex);
  DeviceEvent ev;
  ev.device = device;
  ev.current_status = current_status;
  ev.previous_status = previous_status;
  _pending_events.push_back(ev);
}

/**
 * Called once per frame on the main thread.  Drains the pending device event
 * queue and processes connections / disconnections using Panda APIs.
 */
void GameInputManager::
update() {
  // Swap the pending events out under the std::mutex so we hold it briefly.
  std::vector<DeviceEvent> events;
  {
    std::lock_guard<std::mutex> guard(_cb_mutex);
    events.swap(_pending_events);
  }

  for (const DeviceEvent &ev : events) {
    bool is_connected = (ev.current_status & GameInputDeviceConnected) != 0;
    bool was_connected = (ev.previous_status & GameInputDeviceConnected) != 0;

    if (is_connected && !was_connected) {
      // Device arrival.
      PT(GameInputDevice) dev;
      {
        LightMutexHolder holder(_lock);

        if (find_device(ev.device) != nullptr) {
          ev.device->Release();
          continue;
        }

        dev = new GameInputDevice(this, ev.device);
        dev->set_connected(true);
        _devices[ev.device] = dev;
        _connected_devices.add_device(dev);
      }

      if (device_cat.is_debug()) {
        device_cat.debug()
          << "GameInput: device connected: " << *dev << "\n";
      }
      throw_event("connect-device", dev.p());

    } else if (!is_connected && was_connected) {
      // Device removal.
      PT(GameInputDevice) dev;
      {
        LightMutexHolder holder(_lock);

        auto it = _devices.find(ev.device);
        if (it == _devices.end()) {
          ev.device->Release();
          continue;
        }
        dev = it->second;
        _devices.erase(it);

        dev->set_connected(false);
        _connected_devices.remove_device(dev);
      }

      if (device_cat.is_debug()) {
        device_cat.debug()
          << "GameInput: device disconnected: " << *dev << "\n";
      }
      throw_event("disconnect-device", dev.p());
    }

    ev.device->Release();
  }
}

// Static callback trampoline -------------------------------------------------

void CALLBACK GameInputManager::
device_callback_trampoline(
    GameInputCallbackToken token,
    void *context,
    IGameInputDevice *device,
    uint64_t timestamp,
    GameInputDeviceStatus current_status,
    GameInputDeviceStatus previous_status) {

  GameInputManager *mgr = static_cast<GameInputManager *>(context);
  mgr->on_device_callback(token, device, timestamp,
                           current_status, previous_status);
}

#endif  // _WIN32
