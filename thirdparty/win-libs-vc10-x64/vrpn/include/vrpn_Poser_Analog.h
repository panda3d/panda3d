#ifndef vrpn_POSER_ANALOG_H
#define vrpn_POSER_ANALOG_H

#ifndef _WIN32_WCE
#include <time.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#ifndef _WIN32_WCE
#include <sys/time.h>
#endif
#endif

#include "vrpn_Poser.h"
#include "vrpn_Analog_Output.h"
#include "vrpn_Tracker.h"


// This code is for a Poser server that uses a vrpn_Analog_Output to drive a device.
// It is basically the inverse of a vrpn_Tracker_AnalogFly.
// We are assuming that one Analog device will be used to drive all axes.  This could be
// Changed by storing an Analog device per axis.
//
// This class can also act as a vrpn_Tracker so that it can report back the new positions
// and velocities to the client when the requests are within the bounds.  This is not
// what you want it to do if you've got an independent tracker watching your output,
// and in any case is open-loop, so use with caution.


// Class for holding data used in transforming pose data to analog values for each axis
class VRPN_API vrpn_PA_axis {
    public:
        vrpn_PA_axis(char *name = NULL, int c = -1, double offset = 0.0, double s = 1.0) : ana_name(name), channel(c), offset(offset), scale(s) {}  

	char *ana_name;	    // Name of the Analog Output device to drive with this axis
        int channel;        // Which channel to use from the Analog device.  Default value of -1 means 
                            // no motion for this axis
        double offset;      // Offset to apply to pose values for this channel to reach 0
        double scale;       // Scale applied to pose values to get the correct analog value
};

// Class for passing in config data.  Usually read from the vrpn.cfg file in the server code.
class VRPN_API vrpn_Poser_AnalogParam {
    public:
        vrpn_Poser_AnalogParam();

        // Translation for the three axes
        vrpn_PA_axis x, y, z;

        // Rotation for the three axes
        vrpn_PA_axis rx, ry, rz;

        // Workspace max and min info
        vrpn_float64    pos_min[3], pos_max[3], pos_rot_min[3], pos_rot_max[3],
                        vel_min[3], vel_max[3], vel_rot_min[3], vel_rot_max[3];
};

class VRPN_API vrpn_Poser_Analog;	// Forward reference

class VRPN_API vrpn_PA_fullaxis {
public:
        vrpn_PA_fullaxis (void) { ana = NULL; value = 0.0; pa = NULL; };

	vrpn_PA_axis axis;
        vrpn_Analog_Output_Remote * ana;
	vrpn_Poser_Analog * pa;
        double value;
};

class VRPN_API vrpn_Poser_Analog : public vrpn_Poser, public vrpn_Tracker {
    public:
        vrpn_Poser_Analog(const char* name, vrpn_Connection* c, vrpn_Poser_AnalogParam* p, bool act_as_tracker = false);

        virtual ~vrpn_Poser_Analog();

        virtual void mainloop();

    protected:
        // Axes for translation and rotation
        vrpn_PA_fullaxis x, y, z, rx, ry, rz;

        // Should we act like a tracker, and report back values?
        bool  d_act_as_tracker;

        static int VRPN_CALLBACK handle_change_message(void *userdata, vrpn_HANDLERPARAM p);
        static int VRPN_CALLBACK handle_vel_change_message(void *userdata, vrpn_HANDLERPARAM p);

        // Routine to update the analog values from the current pose
        bool update_Analog_values();
	bool setup_channel(vrpn_PA_fullaxis *full);
};

#endif

