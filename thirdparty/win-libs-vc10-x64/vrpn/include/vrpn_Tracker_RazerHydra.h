/** @file
	@brief Header

	@date 2011

	@author
	Ryan Pavlik
	<rpavlik@iastate.edu> and <abiryan@ryand.net>
	http://academic.cleardefinition.com/
	Iowa State University Virtual Reality Applications Center
	Human-Computer Interaction Graduate Program
*/

//          Copyright Iowa State University 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef INCLUDED_vrpn_Tracker_RazerHydra_h_GUID_8c30e762_d7e7_40c5_9308_b9bc118959fd
#define INCLUDED_vrpn_Tracker_RazerHydra_h_GUID_8c30e762_d7e7_40c5_9308_b9bc118959fd

// Internal Includes
#include "vrpn_Configure.h"
#include "vrpn_HumanInterface.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Tracker.h"

// Library/third-party includes
// - none

// Standard includes
// - none

#ifdef VRPN_USE_HID

/** @brief Device supporting the Razer Hydra game controller as a tracker,
	analog device, and button device, using the USB HID protocol directly

	The left wand (the one with LB and LT on its "end" buttons) is sensor 0.
	Be sure to have the wands resting in their positions on the base when
	starting - haven't figured out how to do the calibration yet.

	The base coordinate system is right-handed with the axes:
	* X - out the right of the base
	* Y - out the front of the base
	* Z - down

	The wand coordinates are also right-handed, with the tracked point somewhere near
	the cable entry to the controller. When held with the joystick vertical, the axes
	are:
	* X - to the right
	* Y - out the front of the controller (trigger buttons)
	* Z - Up, along the joystick

	Buttons are as follows, with the right controller's button channels starting
	at 8 instead of 0:
	* 0 - "middle" button below joystick
	* 1-4 - numbered buttons
	* 5 - "bumper" button (above trigger)
	* 6 - joystick button (if you push straight down on the joystick)

	Analog channels are as follows, with the right controller starting at 3
	instead of 0:
	* 0 - joystick left/right: centered at 0, right is positive, in [-1, 1]
	* 1 - joystick up/down: centered at 0, up is positive, in [-1, 1]
	* 2 - analog trigger, in range 0 (not pressed) to 1 (fully pressed).
*/
class VRPN_API vrpn_Tracker_RazerHydra: public vrpn_Analog, public vrpn_Button_Filter, public vrpn_Tracker, vrpn_HidInterface {
	public:
		vrpn_Tracker_RazerHydra(const char * name, vrpn_Connection * trackercon);
		~vrpn_Tracker_RazerHydra();

		virtual void mainloop();
		virtual void reconnect();

		virtual void on_data_received(size_t bytes, vrpn_uint8 *buffer);

	protected:
		enum HydraStatus {
			HYDRA_WAITING_FOR_CONNECT,
			HYDRA_LISTENING_AFTER_CONNECT,
			HYDRA_LISTENING_AFTER_SET_FEATURE,
			HYDRA_REPORTING
		};

		void _waiting_for_connect();
		void _listening_after_connect();
		void _listening_after_set_feature();

		void _enter_motion_controller_mode();

		void _report_for_sensor(int sensorNum, vrpn_uint8 * data);

		HydraStatus status;
		bool _wasInGamepadMode;
		int _attempt;
		struct timeval _timestamp;
		struct timeval _connected;
		struct timeval _set_feature;
};

#endif

#endif // INCLUDED_vrpn_Tracker_RazerHydra_h_GUID_8c30e762_d7e7_40c5_9308_b9bc118959fd
