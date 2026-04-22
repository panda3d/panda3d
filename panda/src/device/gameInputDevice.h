/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gameInputDevice.h
 * @author thetestgame
 * @date 2026-03-16
 */

#ifndef GAMEINPUTDEVICE_H
#define GAMEINPUTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

#if defined(_WIN32) && defined(HAVE_GAMEINPUT) && !defined(CPPPARSER)

#include <GameInput.h>

class GameInputManager;

/**
 * This implementation of InputDevice uses Microsoft's GameInput API, which
 * provides a unified interface for gamepads, flight sticks, racing wheels,
 * and other game controllers on Windows.
 *
 * GameInput supersedes XInput, DirectInput, and Raw Input for game controller
 * support, offering lower latency and better device identification.
 */
class EXPCL_PANDA_DEVICE GameInputDevice final : public InputDevice {
public:
  GameInputDevice(GameInputManager *manager, IGameInputDevice *gi_device);
  ~GameInputDevice();

  void on_reading(IGameInputReading *reading);
  IGameInputDevice *get_gi_device() const { return _gi_device; }

private:
  void init_from_info(const GameInputDeviceInfo *info);
  void process_gamepad_reading(IGameInputReading *reading);
  void process_flight_stick_reading(IGameInputReading *reading);
  void process_racing_wheel_reading(IGameInputReading *reading);

  virtual void do_set_vibration(double strong, double weak);
  virtual void do_poll();

private:
  IGameInputDevice *_gi_device;
  GameInputManager *_manager;

  // Track the kind of reading to process.
  GameInputKind _supported_input;

  friend class GameInputManager;
};

#endif  // _WIN32

#endif
