///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        vrpn_Tracker_zSight.h
//
// Authors:     David Borland
//              Josep Maria Tomas Sanahuja
//
//				EventLab at the University of Barcelona
//
// Description: VRPN tracker class for Sensics zSight HMD with built-in tracker.  The tracker
//              reports only orientation information, no position.  It is interfaced to as
//              a DirectX joystick, so VRPN_USE_DIRECTINPUT must be defined in 
//              vrpn_Configure.h to use it.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 

#ifndef VRPN_TRACKER_ZSIGHT
#define VRPN_TRACKER_ZSIGHT

// Make sure Direct Input is being used
#include "vrpn_Configure.h"
#if defined(_WIN32) && defined(VRPN_USE_DIRECTINPUT)

#include "vrpn_Tracker.h"

#ifndef DIRECTINPUT_VERSION
#define	DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <windows.h>

class vrpn_Tracker_zSight : public vrpn_Tracker {
public:
    // Constructor
    //
    // name:        VRPN tracker name
    //
    // c:           VRPN connection to use
	//
    vrpn_Tracker_zSight(const char* name, vrpn_Connection* c);
    ~vrpn_Tracker_zSight();

	/// This function should be called each time through the main loop
	/// of the server code. It checks for a report from the tracker and
	/// sends it if there is one.
	virtual void mainloop();

protected:
    // VRPN tracker functions for generating and sending reports
    virtual void get_report();
    virtual void send_report();

    // Initialize the device
    HRESULT InitDevice();

    // Callbacks for Direct Input
	static BOOL CALLBACK EnumSensicsCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* selfPtr);
    static BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* selfPtr);

	// Handle to the console window
	HWND hWnd;

    // The Direct Input and device handles
	LPDIRECTINPUT8 directInput;
	LPDIRECTINPUTDEVICE8 sensics;
};


#endif
#endif