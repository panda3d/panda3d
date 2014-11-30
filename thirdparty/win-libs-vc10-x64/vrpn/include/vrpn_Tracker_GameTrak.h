/*
 * vrpn_Tracker_GameTrak.h
 *
 *  Created on: Nov 22, 2010
 *      Author: janoc
 */

#ifndef VRPN_TRACKER_GAMETRAK_H_
#define VRPN_TRACKER_GAMETRAK_H_

#include <string>

#include "vrpn_Configure.h"
#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"

class VRPN_API vrpn_Tracker_GameTrak: public vrpn_Tracker
{
    public:
        vrpn_Tracker_GameTrak(const char * name, vrpn_Connection * trackercon, const char *joystick_dev, int *mapping);

        virtual void mainloop ();

        virtual ~vrpn_Tracker_GameTrak();

    protected:
        int _mapping[6];
        std::string _name;
        vrpn_Connection *_con;
        std::string _joydev;

        bool _should_report;
        vrpn_float64 _sensor0[3], _sensor1[3];
        struct timeval _timestamp;

        vrpn_Analog_Remote *_analog;

        static void VRPN_CALLBACK handle_update (void *, const vrpn_ANALOGCB);
};

#endif /* VRPN_TRACKER_GAMETRAK_H_ */
