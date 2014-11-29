/// This object has been superceded by the vrpn_Tracker_AnalogFly object.

#ifndef INCLUDED_JOYFLY
#define INCLUDED_JOYFLY

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"

#include <quat.h>

class VRPN_API vrpn_Tracker_JoyFly : public vrpn_Tracker {

  private:
    double chanAccel [7];
    int chanPower [7];
    struct timeval prevtime;

    vrpn_Analog_Remote * joy_remote;
    q_matrix_type initMatrix, currentMatrix;

  public:
    vrpn_Tracker_JoyFly (const char * name, vrpn_Connection * c, 
		         const char * source, const char * config_file_name,
                         vrpn_Connection * sourceConnection = NULL);
    virtual ~vrpn_Tracker_JoyFly (void);

    virtual void mainloop (void);
    virtual void reset (void);

    void update (q_matrix_type &);

    static void VRPN_CALLBACK handle_joystick (void *, const vrpn_ANALOGCB);
    static int VRPN_CALLBACK handle_newConnection (void *, vrpn_HANDLERPARAM);
};

#endif















