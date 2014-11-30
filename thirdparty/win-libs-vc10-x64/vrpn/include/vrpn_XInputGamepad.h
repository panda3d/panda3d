#ifndef VRPN_XINPUTGAMEPAD_H
#define VRPN_XINPUTGAMEPAD_H

#include "vrpn_Configure.h"

#if defined(_WIN32) && defined(VRPN_USE_DIRECTINPUT) && defined(VRPN_USE_WINDOWS_XINPUT)

#include "vrpn_Connection.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Analog_Output.h"

// This implements an XInput gamepad (read: Xbox 360 controller), which has
// analog outputs and button outputs but also enables the user to set a
// rumble magnitude using an Analog_Output (channel 0 controls the left motor,
// channel 1 controls the right motor).

class VRPN_API vrpn_XInputGamepad: public vrpn_Analog, public vrpn_Button, public vrpn_Analog_Output {
public:
	vrpn_XInputGamepad(const char *name, vrpn_Connection *c = NULL, unsigned int controllerIndex = 0);
	~vrpn_XInputGamepad();

	virtual void mainloop();

protected:
	// Handle requests to change rumble magnitude
	static int VRPN_CALLBACK handle_request_message(void *selfPtr, vrpn_HANDLERPARAM data);
	static int VRPN_CALLBACK handle_request_channels_message(void *selfPtr, vrpn_HANDLERPARAM data);
	static int VRPN_CALLBACK handle_last_connection_dropped(void *selfPtr, vrpn_HANDLERPARAM data);

	// send report iff changed
    void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
    // send report whether or not changed
    void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
    // NOTE:  class_of_service is only applied to vrpn_Analog
    //  values, not vrpn_Button

	void update_vibration();

	// These functions may be overridden to disable the default filtering
	virtual vrpn_float64 normalize_axis(SHORT axis, SHORT deadzone) const;
	virtual vrpn_float64 normalize_trigger(BYTE trigger) const;
	virtual vrpn_float64 normalize_dpad(WORD buttons) const;

private:
	unsigned int _controllerIndex;
	timeval _timestamp;

	WORD _motorSpeed[2];
};

#endif // _WIN32 && VRPN_USE_DIRECTINPUT
#endif // VRPN_XINPUTGAMEPAD_H

