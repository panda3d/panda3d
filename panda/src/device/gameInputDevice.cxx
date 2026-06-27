/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file gameInputDevice.cxx
 * @author thetestgame
 * @date 2026-03-16
 */

#include "gameInputDevice.h"
#include "gameInputManager.h"

#if defined(_WIN32) && defined(HAVE_GAMEINPUT) && !defined(CPPPARSER)

#include "gamepadButton.h"
#include <cmath>
#include "inputDeviceManager.h"

/**
 * Constructs a GameInputDevice wrapping the given IGameInputDevice.  The
 * device info is queried to set up axes, buttons, and device classification.
 */
GameInputDevice::
GameInputDevice(GameInputManager *manager, IGameInputDevice *gi_device) :
  _manager(manager),
  _gi_device(gi_device),
  _supported_input(GameInputKindUnknown) {

  _gi_device->AddRef();

  const GameInputDeviceInfo *info = _gi_device->GetDeviceInfo();
  if (info != nullptr) {
    init_from_info(info);
  }
}

/**
 *
 */
GameInputDevice::
~GameInputDevice() {
  if (_gi_device != nullptr) {
    _gi_device->Release();
    _gi_device = nullptr;
  }
}

/**
 * Initializes the device's axes, buttons, and metadata from the GameInput
 * device info structure.
 */
void GameInputDevice::
init_from_info(const GameInputDeviceInfo *info) {
  // Set device name.  displayName is a GameInputString with UTF-8 data.
  if (info->displayName != nullptr && info->displayName->sizeInBytes > 0) {
    _name.assign(info->displayName->data, info->displayName->sizeInBytes);
  }

  if (_name.empty()) {
    _name = "GameInput Device";
  }

  _vendor_id = info->vendorId;
  _product_id = info->productId;

  _supported_input = info->supportedInput;

  // Classify the device and set up axes/buttons based on what it supports.
  if (_supported_input & GameInputKindGamepad) {
    _device_class = DeviceClass::GAMEPAD;

    // Standard gamepad: 6 axes (left stick X/Y, right stick X/Y, left/right triggers)
    add_axis(Axis::LEFT_X, -100, 100, true);
    add_axis(Axis::LEFT_Y, -100, 100, true);
    add_axis(Axis::RIGHT_X, -100, 100, true);
    add_axis(Axis::RIGHT_Y, -100, 100, true);
    add_axis(Axis::LEFT_TRIGGER, 0, 100);
    add_axis(Axis::RIGHT_TRIGGER, 0, 100);

    // Standard gamepad: 15 buttons
    add_button(GamepadButton::face_a());
    add_button(GamepadButton::face_b());
    add_button(GamepadButton::face_x());
    add_button(GamepadButton::face_y());
    add_button(GamepadButton::lshoulder());
    add_button(GamepadButton::rshoulder());
    add_button(GamepadButton::back());
    add_button(GamepadButton::start());
    add_button(GamepadButton::guide());
    add_button(GamepadButton::lstick());
    add_button(GamepadButton::rstick());
    add_button(GamepadButton::dpad_up());
    add_button(GamepadButton::dpad_down());
    add_button(GamepadButton::dpad_left());
    add_button(GamepadButton::dpad_right());

    enable_feature(Feature::VIBRATION);

  } else if (_supported_input & GameInputKindFlightStick) {
    _device_class = DeviceClass::FLIGHT_STICK;

    add_axis(Axis::ROLL, -100, 100, true);
    add_axis(Axis::PITCH, -100, 100, true);
    add_axis(Axis::YAW, -100, 100, true);
    add_axis(Axis::THROTTLE, 0, 100);

    // Flight sticks can have a variable number of buttons; we set up a
    // reasonable default and the HID report will tell us the actual count.
    for (int i = 0; i < 12; ++i) {
      add_button(GamepadButton::joystick(i));
    }

    // Hat switch as 4 directional buttons.
    add_button(GamepadButton::hat_up());
    add_button(GamepadButton::hat_down());
    add_button(GamepadButton::hat_left());
    add_button(GamepadButton::hat_right());

  } else if (_supported_input & GameInputKindRacingWheel) {
    _device_class = DeviceClass::STEERING_WHEEL;

    add_axis(Axis::WHEEL, -100, 100, true);
    add_axis(Axis::ACCELERATOR, 0, 100);
    add_axis(Axis::BRAKE, 0, 100);

    // Racing wheels have: Menu, View, PreviousGear, NextGear, DPad.
    add_button(GamepadButton::start());     // 0: Menu
    add_button(GamepadButton::back());      // 1: View
    add_button(GamepadButton::lshoulder()); // 2: PreviousGear
    add_button(GamepadButton::rshoulder()); // 3: NextGear
    add_button(GamepadButton::dpad_up());   // 4: DpadUp
    add_button(GamepadButton::dpad_down()); // 5: DpadDown
    add_button(GamepadButton::dpad_left()); // 6: DpadLeft
    add_button(GamepadButton::dpad_right());// 7: DpadRight

    enable_feature(Feature::VIBRATION);

  } else {
    _device_class = DeviceClass::UNKNOWN;
  }
}

/**
 * Processes a new reading from the GameInput API.  Called by the manager
 * when a reading callback fires.  Assumes the lock is held.
 */
void GameInputDevice::
on_reading(IGameInputReading *reading) {
  if (reading == nullptr) {
    return;
  }

  nassertv(_lock.debug_is_locked());

  if (_supported_input & GameInputKindGamepad) {
    process_gamepad_reading(reading);
  } else if (_supported_input & GameInputKindFlightStick) {
    process_flight_stick_reading(reading);
  } else if (_supported_input & GameInputKindRacingWheel) {
    process_racing_wheel_reading(reading);
  }
}

/**
 * Processes a gamepad reading - buttons and axes from a standard gamepad.
 */
void GameInputDevice::
process_gamepad_reading(IGameInputReading *reading) {
  GameInputGamepadState state;
  if (!reading->GetGamepadState(&state)) {
    return;
  }

  // Axes - GameInput returns floats in [-1, 1] for sticks and [0, 1] for
  // triggers.  Our axes were set up with ranges of -100..100 and 0..100, so
  // we pass a raw int scaled by 100 to axis_changed which applies
  // scale/bias.
  axis_changed(0, std::lround(state.leftThumbstickX * 100));   // LEFT_X
  axis_changed(1, std::lround(state.leftThumbstickY * 100));   // LEFT_Y
  axis_changed(2, std::lround(state.rightThumbstickX * 100));  // RIGHT_X
  axis_changed(3, std::lround(state.rightThumbstickY * 100));  // RIGHT_Y
  axis_changed(4, std::lround(state.leftTrigger * 100));       // LEFT_TRIGGER
  axis_changed(5, std::lround(state.rightTrigger * 100));      // RIGHT_TRIGGER

  // Buttons - GameInput uses a bitmask.
  GameInputGamepadButtons buttons = state.buttons;
  button_changed(0,  (buttons & GameInputGamepadA) != 0);
  button_changed(1,  (buttons & GameInputGamepadB) != 0);
  button_changed(2,  (buttons & GameInputGamepadX) != 0);
  button_changed(3,  (buttons & GameInputGamepadY) != 0);
  button_changed(4,  (buttons & GameInputGamepadLeftShoulder) != 0);
  button_changed(5,  (buttons & GameInputGamepadRightShoulder) != 0);
  button_changed(6,  (buttons & GameInputGamepadView) != 0);
  button_changed(7,  (buttons & GameInputGamepadMenu) != 0);
  // Guide button not available through standard GameInput gamepad state.
  // We leave button 8 (guide) unchanged.
  button_changed(9,  (buttons & GameInputGamepadLeftThumbstick) != 0);
  button_changed(10, (buttons & GameInputGamepadRightThumbstick) != 0);
  button_changed(11, (buttons & GameInputGamepadDPadUp) != 0);
  button_changed(12, (buttons & GameInputGamepadDPadDown) != 0);
  button_changed(13, (buttons & GameInputGamepadDPadLeft) != 0);
  button_changed(14, (buttons & GameInputGamepadDPadRight) != 0);
}

/**
 * Processes a flight stick reading.
 */
void GameInputDevice::
process_flight_stick_reading(IGameInputReading *reading) {
  GameInputFlightStickState state;
  if (!reading->GetFlightStickState(&state)) {
    return;
  }

  axis_changed(0, std::lround(state.roll * 100));
  axis_changed(1, std::lround(state.pitch * 100));
  axis_changed(2, std::lround(state.yaw * 100));
  axis_changed(3, std::lround(state.throttle * 100));

  // Buttons via the generic controller interface.
  uint32_t button_count = reading->GetControllerButtonCount();
  if (button_count > 0) {
    uint32_t count = std::min(button_count, (uint32_t)12);
    bool pressed[12] = {};
    reading->GetControllerButtonState(count, pressed);
    for (uint32_t i = 0; i < count; ++i) {
      button_changed(i, pressed[i]);
    }
  }

  // Hat switch.
  GameInputSwitchPosition hat = state.hatSwitch;
  button_changed(12, hat == GameInputSwitchUp ||
                     hat == GameInputSwitchUpRight ||
                     hat == GameInputSwitchUpLeft);   // up
  button_changed(13, hat == GameInputSwitchDown ||
                     hat == GameInputSwitchDownRight ||
                     hat == GameInputSwitchDownLeft);  // down
  button_changed(14, hat == GameInputSwitchLeft ||
                     hat == GameInputSwitchUpLeft ||
                     hat == GameInputSwitchDownLeft);  // left
  button_changed(15, hat == GameInputSwitchRight ||
                     hat == GameInputSwitchUpRight ||
                     hat == GameInputSwitchDownRight); // right
}

/**
 * Processes a racing wheel reading.
 */
void GameInputDevice::
process_racing_wheel_reading(IGameInputReading *reading) {
  GameInputRacingWheelState state;
  if (!reading->GetRacingWheelState(&state)) {
    return;
  }

  axis_changed(0, std::lround(state.wheel * 100));
  axis_changed(1, std::lround(state.throttle * 100));
  axis_changed(2, std::lround(state.brake * 100));

  GameInputRacingWheelButtons buttons = state.buttons;
  button_changed(0, (buttons & GameInputRacingWheelMenu) != 0);
  button_changed(1, (buttons & GameInputRacingWheelView) != 0);
  button_changed(2, (buttons & GameInputRacingWheelPreviousGear) != 0);
  button_changed(3, (buttons & GameInputRacingWheelNextGear) != 0);
  button_changed(4, (buttons & GameInputRacingWheelDpadUp) != 0);
  button_changed(5, (buttons & GameInputRacingWheelDpadDown) != 0);
  button_changed(6, (buttons & GameInputRacingWheelDpadLeft) != 0);
  button_changed(7, (buttons & GameInputRacingWheelDpadRight) != 0);
}

/**
 * Sets the vibration strength.  The first argument controls the low-frequency
 * motor, and the second controls the high-frequency motor.
 * Values are within the 0-1 range.
 */
void GameInputDevice::
do_set_vibration(double strong, double weak) {
  if (_gi_device == nullptr || !_is_connected) {
    return;
  }

  GameInputRumbleParams params = {};
  params.lowFrequency = (float)strong;
  params.highFrequency = (float)weak;
  params.leftTrigger = 0.0f;
  params.rightTrigger = 0.0f;

  _gi_device->SetRumbleState(&params);
}

/**
 * Polls the input device for new activity.  With GameInput, readings are
 * delivered via callbacks, so this is typically a no-op.  However, we can
 * also pull the latest reading synchronously as a fallback.
 */
void GameInputDevice::
do_poll() {
  if (_gi_device == nullptr || !_is_connected) {
    return;
  }

  IGameInput *gi = nullptr;
  if (_manager != nullptr) {
    gi = _manager->get_game_input();
  }
  if (gi == nullptr) {
    return;
  }

  IGameInputReading *reading = nullptr;
  HRESULT hr = gi->GetCurrentReading(_supported_input, _gi_device, &reading);
  if (SUCCEEDED(hr) && reading != nullptr) {
    on_reading(reading);
    reading->Release();
  }
}

#endif  // _WIN32
