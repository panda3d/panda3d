#ifndef VRPN_ANDROID_H
#define VRPN_ANDROID_H

#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string>
#include <string.h>

#include "vrpn_Types.h"

#ifndef	VRPN_BUTTON_H
#include "vrpn_Button.h"
#define VRPN_BUTTON_H
#endif

#ifndef VRPN_ANALOG_H
#include "vrpn_Analog.h"
#define VRPN_ANALOG_H
#endif

/*
 * This is an Android server implementation.  It has as members a vrpn_Button_Server
 * and some number of vrpn_Analog_Servers, as determined by the Java side.  This 
 * implementation also has vrpn_Button_Remote and vrpn_Analog_Remote clients for 
 * the sole purpose of testing.  All data from the Android is represented here
 * as either Analogs or Buttons.
 *
 * All interaction between this class and the Java side are handled by the jni layer.
 *
 * It's worth noting that it is possible to pipe stdout to the Android log - directions
 * for doing so are in the manual for this project - and all output is in the form of:
 * fprintf(stderr, "..."), which we've found to be more reliable than other methods,
 * including fprintf(stdout, "...").
 */
 

class vrpn_Android_Server
{
public:

	/// Constructor.  Takes an array of integers representing the number of channels for each analog server, the number of buttons, and the port number.
	vrpn_Android_Server(vrpn_int32 num_analogs, vrpn_int32 * analog_sizes, vrpn_int32 num_buttons, vrpn_int32 port);
	
	/// Destructor.
	~vrpn_Android_Server();
	
	/// Main loop to be called at every time step.  Calls the mainloop() functions of the member servers.
	void mainloop();

	/// Set the value for the given channel of the given vrpn_Analog_Server
	void set_analog(vrpn_int32 analog_id, vrpn_int32 channel, vrpn_float64 val);
	
	/// Set the value for the given button
	void set_button(vrpn_int32 button_id, vrpn_int32 state);
	
	/// Called when changes are made to any of the analog values
	void report_analog_chg(vrpn_int32 analog_id);
	
private:

	/// Sets up the member servers
	void initialize(vrpn_int32 num_analogs, vrpn_int32 * analog_sizes, vrpn_int32 num_buttons);

	// Names of the member servers.  Only used by the local clients
	const char	* ANALOG_SERVER_NAME;		// with no id number; those are formed automatically
	const char	* BUTTON_SERVER_NAME;		// uses id 0, since there is only one button server
	
	vrpn_int32 num_analogs;					// Number of vrpn_Analog_Servers
	vrpn_int32 * analog_sizes;				// Array of numbers of channels in the analog servers

	vrpn_Analog_Server ** analog_server; 	// handles multi-touch x and y as well as any sliders, dials, etc
	vrpn_Button_Server 	* button_server;
	
	vrpn_Analog_Remote ** analog_client;
	vrpn_Button_Remote	* button_client;
	
	vrpn_Connection		* connection;
	
	vrpn_int32 port;
};

#endif
