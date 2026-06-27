/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gameInputManager.h
 * @author thetestgame
 * @date 2026-03-16
 */

#ifndef GAMEINPUTMANAGER_H
#define GAMEINPUTMANAGER_H

#include "inputDeviceManager.h"

#if defined(_WIN32) && defined(HAVE_GAMEINPUT) && !defined(CPPPARSER)

#include <GameInput.h>
#include <mutex>
#include <vector>

class GameInputDevice;

/**
 * This is the Windows implementation of InputDeviceManager using the
 * Microsoft GameInput API.  GameInput provides a unified, low-latency
 * interface for gamepads, flight sticks, racing wheels, and other game
 * controllers.
 *
 * This replaces the older WinInputDeviceManager which used Raw Input + XInput.
 */
class EXPCL_PANDA_DEVICE GameInputManager final : public InputDeviceManager {
private:
  GameInputManager();
  ~GameInputManager();

public:
  IGameInput *get_game_input() const { return _game_input; }

private:
  void on_device_callback(GameInputCallbackToken token,
                          IGameInputDevice *device,
                          uint64_t timestamp,
                          GameInputDeviceStatus current_status,
                          GameInputDeviceStatus previous_status);

  GameInputDevice *find_device(IGameInputDevice *gi_device);

  virtual void update() override;

  // C-style callback trampoline for GameInput device events.
  static void CALLBACK device_callback_trampoline(
      GameInputCallbackToken token,
      void *context,
      IGameInputDevice *device,
      uint64_t timestamp,
      GameInputDeviceStatus current_status,
      GameInputDeviceStatus previous_status);

  IGameInput *_game_input;
  GameInputCallbackToken _device_callback_token;

  pmap<IGameInputDevice *, GameInputDevice *> _devices;

  // Pending device events queued from the GameInput callback thread.
  // Protected by _cb_mutex (a std::mutex, NOT Panda's LightMutex, because
  // GameInput callbacks fire on threads not registered with Panda).
  struct DeviceEvent {
    IGameInputDevice *device;
    GameInputDeviceStatus current_status;
    GameInputDeviceStatus previous_status;
  };
  std::mutex _cb_mutex;
  std::vector<DeviceEvent> _pending_events;

  friend class InputDeviceManager;
};

#endif  // _WIN32

#endif
