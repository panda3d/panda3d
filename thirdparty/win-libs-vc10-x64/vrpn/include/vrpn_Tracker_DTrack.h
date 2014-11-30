// vrpn_Tracker_DTrack.h 
// 
// Advanced Realtime Tracking GmbH's (http://www.ar-tracking.de) DTrack/DTrack2 client
//
// developed by David Nahon for Virtools VR Pack (http://www.virtools.com)
// (07/20/2004) improved by Advanced Realtime Tracking GmbH (http://www.ar-tracking.de)
// (07/02/2007, 06/29/2009) upgraded by Advanced Realtime Tracking GmbH to support new devices
// (08/25/2010) a correction added by Advanced Realtime Tracking GmbH
// (12/01/2010) support of 3dof objects added by Advanced Realtime Tracking GmbH

#ifndef VRPN_TRACKER_DTRACK_H
#define VRPN_TRACKER_DTRACK_H

// There is a problem with linking on SGI related to the use of standard
// libraries.
#ifndef sgi

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#ifndef _WIN32
	#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"
#include "vrpn_Analog.h"


// --------------------------------------------------------------------------
// Data types:

// Standard marker data (3DOF):

typedef struct{
	int id;               // id number (starting with 0)
	float loc[3];         // location (in mm)
} vrpn_dtrack_marker_type;

// Standard body data (6DOF):
//  - currently not tracked bodies are getting a quality of -1

typedef struct{
	int id;               // id number (starting with 0)
	float quality;        // quality (0 <= qu <= 1, no tracking if -1)
	
	float loc[3];         // location (in mm)
	float rot[9];         // rotation matrix (column-wise)
} vrpn_dtrack_body_type;

// A.R.T. Flystick data (6DOF + buttons):
//  - currently not tracked bodies are getting a quality of -1
//  - note the maximum number of buttons and joystick values

#define vrpn_DTRACK_FLYSTICK_MAX_BUTTON    16  // maximum number of buttons
#define vrpn_DTRACK_FLYSTICK_MAX_JOYSTICK   8  // maximum number of joystick values

typedef struct{
	int id;               // id number (starting with 0)
	float quality;        // quality (0 <= qu <= 1, no tracking if -1)

	int num_button;       // number of buttons
	int button[vrpn_DTRACK_FLYSTICK_MAX_BUTTON];  // button state (1 pressed, 0 not pressed)
	                                              // (0 front, 1..n-1 right to left)
	int num_joystick;     // number of joystick values
	float joystick[vrpn_DTRACK_FLYSTICK_MAX_JOYSTICK];  // joystick value (-1 <= joystick <= 1) 
	                                                    // (0 horizontal, 1 vertical)

	float loc[3];         // location (in mm)
	float rot[9];         // rotation matrix (column-wise)
} vrpn_dtrack_flystick_type;


// --------------------------------------------------------------------------
// VRPN class:

class VRPN_API vrpn_Tracker_DTrack : public vrpn_Tracker, public vrpn_Button, public vrpn_Analog
{
  
 public:

#ifdef _WIN32
        typedef SOCKET socket_type;
#else
        typedef int socket_type;
#endif

// Constructor:
// name (i): device name
// c (i): vrpn_Connection
// dtrackPort (i): DTrack UDP port
// timeToReachJoy (i): time needed to reach the maximum value of the joystick
// fixNbody, fixNflystick (i): fixed numbers of DTrack bodies and Flysticks (-1 if not wanted)
// fixId (i): renumbering of targets; must have exact (fixNbody + fixNflystick) elements (NULL if not wanted)
// act3DOFout (i): activate 3dof marker output if present
// actTracing (i): activate trace output

	vrpn_Tracker_DTrack(const char *name, vrpn_Connection *c,
	                    int dtrackPort, float timeToReachJoy = 0.f,
	                    int fixNbody = -1, int fixNflystick = -1, int* fixId = NULL,
	                    bool act3DOFout = false, bool actTracing = false);

	~vrpn_Tracker_DTrack();

	/// This function should be called each time through the main loop
	/// of the server code. It checks for a report from the tracker and
	/// sends it if there is one.

	virtual void mainloop();


 private:

	// general:
	
	struct timeval tim_first;      // timestamp of first frame
	struct timeval tim_last;       // timestamp of current frame
	
	bool tracing;                  // activate debug output
	unsigned int tracing_frames;   // frame counter for debug output

	// DTrack data:

	bool use_fix_numbering;        // use fixed numbers of standard bodies and Flysticks

	int fix_nbody;                 // fixed number of standard bodies
	int fix_nflystick;             // fixed number of Flysticks

	std::vector<int> fix_idbody;      // fixed vrpn standard body IDs
	std::vector<int> fix_idflystick;  // fixed vrpn Flystick IDs

	bool warning_nbodycal;         // already warned cause of missing '6dcal' data

	// preparing data for VRPN:
	// these functions convert DTrack data to vrpn data

	std::vector<bool> joy_simulate;  // simulate time varying floating values
	std::vector<float> joy_last;     // current value of 'joystick' channel (hor, ver)
	float joy_incPerSec;             // increase of 'joystick' channel (in 1/sec)

	int dtrack2vrpn_marker(int id, const char* str_dtrack, int id_dtrack,
	                     const float* loc, struct timeval timestamp);
	int dtrack2vrpn_body(int id, const char* str_dtrack, int id_dtrack,
	                     const float* loc, const float* rot, struct timeval timestamp);
	int dtrack2vrpn_flystickbuttons(int id, int id_dtrack,
	                                int num_but, const int* but, struct timeval timestamp);
	int dtrack2vrpn_flystickanalogs(int id, int id_dtrack,
	                                int num_ana, const float* ana, float dt, struct timeval timestamp);

	// communicating with DTrack:
	// these functions receive and parse data packets from DTrack

	socket_type d_udpsock;          // socket number for UDP
	int d_udptimeout_us;            // timeout for receiving UDP data

	int d_udpbufsize;               // size of UDP buffer
	char* d_udpbuf;                 // UDP buffer

	unsigned int act_framecounter;                   // frame counter
	double act_timestamp;                            // time stamp
	
	bool output_3dof_marker;                         // 3dof marker output if available
	int act_num_marker;                              // number of 3dof marker (due to '3d' line)
	std::vector<vrpn_dtrack_marker_type> act_marker; // array containing 3dof marker data

	int act_num_body;                                // number of calibrated standard bodies (due to '6d' line)
	std::vector<vrpn_dtrack_body_type> act_body;     // array containing standard body data
	bool act_has_bodycal_format;                     // DTrack sent '6dcal' format
	int act_num_bodycal;                             // number of calibrated standard bodies (due to '6dcal' line)

	int act_num_flystick;                            // number of calibrated Flysticks
	std::vector<vrpn_dtrack_flystick_type> act_flystick;  // array containing Flystick data
	bool act_has_old_flystick_format;                // DTrack uses old Flystick format

	int d_lasterror;                // last receive error
	
	bool dtrack_init(int udpport);
	bool dtrack_exit(void);
	
	bool dtrack_receive(void);
};

#endif

#endif

