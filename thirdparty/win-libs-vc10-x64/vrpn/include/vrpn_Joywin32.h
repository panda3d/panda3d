/*
# Joystick VRPN Driver based on Win32.
# written by Sebastien MARAUX, ONDIM SA (France)
# maraux@ondim.fr
*/

#ifndef VRPN_WIN32JOYSTICK_H
#define VRPN_WIN32JOYSTICK_H

#if defined(_WIN32)

#include "vrpn_Analog.h"
#include "vrpn_Button.h"

#include <basetsd.h>

class VRPN_API vrpn_Joywin32: public vrpn_Analog, public vrpn_Button
{
public:
    vrpn_Joywin32 (const char * name, vrpn_Connection * c, vrpn_uint8 joyNumber = 1, vrpn_float64 readRate = 60, vrpn_uint8 mode = 0, vrpn_int32 deadzone = 0);

    //~vrpn_Joywin32 ();

    // Called once through each main loop iteration to handle
    // updates.
    virtual void mainloop ();

protected:
    int	_status;
    vrpn_uint8	_mode;	// raw data , 0;1 or -1;1 normalized data for axes

    struct timeval _timestamp;	// Time of the last report from the device
    vrpn_float64 _read_rate;		// How many times per second to read the device
    vrpn_float64 _deadzone;		// apply a dead zone to analog inputs

    virtual vrpn_int32 get_report(void);	// Try to read a report from the device
    void	clear_values(void);	// Clear the Analog and Button values

   // send report iff changed
    virtual void report_changes (vrpn_uint32 class_of_service
	      = vrpn_CONNECTION_LOW_LATENCY);
    // send report whether or not changed
    virtual void report (vrpn_uint32 class_of_service
	      = vrpn_CONNECTION_LOW_LATENCY);

    vrpn_uint32	  _numbuttons;	  // How many buttons
    vrpn_uint32	  _numchannels;	  // How many analog channels

    vrpn_uint8	  _joyNumber;
    
    // joystick caps
    JOYCAPS _jc;
};
#endif
#endif

