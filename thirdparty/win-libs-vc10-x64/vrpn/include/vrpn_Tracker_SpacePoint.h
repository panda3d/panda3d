/*
 * vrpn_Tracker_SpacePoint.h
 *
 *  Created on: Nov 22, 2010
 *      Author: janoc
 */

#ifndef VRPN_TRACKER_SPACEPOINT_H_
#define VRPN_TRACKER_SPACEPOINT_H_

#include "vrpn_Configure.h"
#include "vrpn_HumanInterface.h"
#include "vrpn_Tracker.h"
#include "vrpn_Button.h"

#ifdef VRPN_USE_HID
#include <string>

class VRPN_API vrpn_Tracker_SpacePoint: public vrpn_Tracker, vrpn_Button, vrpn_HidInterface
{
    public:
        vrpn_Tracker_SpacePoint(const char * name, vrpn_Connection * trackercon);

        virtual void mainloop ();

        virtual void on_data_received(size_t bytes, vrpn_uint8 *buffer);

    protected:
        std::string _name;
        vrpn_Connection *_con;

        bool _should_report;
        struct timeval _timestamp;
};

#endif

#endif /* VRPN_TRACKER_SPACEPOINT_H_ */
