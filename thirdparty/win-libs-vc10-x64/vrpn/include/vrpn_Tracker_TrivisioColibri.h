///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        vrpn_Tracker_TrivisioColibri.h
//
// Author:      David Borland
//              Institut d'Investigacions Biomèdiques August Pi i Sunyer (IDIBAPS)
//              Virtual Embodiment and Robotic Re-Embodiment (VERE) Project – 257695
//
// Description: VRPN tracker class for Trivisio Colibri device
//
/////////////////////////////////////////////////////////////////////////////////////////////// 

#ifndef VRPN_TRACKER_TRIVISIOCOLIBRI
#define VRPN_TRACKER_TRIVISIOCOLIBRI

#include "vrpn_Configure.h"
#ifdef VRPN_USE_TRIVISIOCOLIBRI

#include "vrpn_Tracker.h" 

class vrpn_Tracker_TrivisioColibri : public vrpn_Tracker {
public:
    // Constructor
    //
    // name:        VRPN tracker name
    //
    // c:           VRPN connection to use
    //
    // numSensors:  The number of devices to connect to
    //
    // Hz:          Update rate in Hertz
    //
    // bufLen:      The buffer length for reading data. 
    //
    //              From the reference manual:
    //
    // An short buffer (0) ensures minimal delay until the sensor measurement is available at the risk
    // of lost measurements. A long buffer guarantees that no data is dropped, at
    // the same time if data is not read fast enough there is a potential risk of a
    // bufLenfrequency before the measurement becomes available.
    //
    vrpn_Tracker_TrivisioColibri(const char* name, vrpn_Connection* c,
                                 int numSensors = 1, int Hz = 60, int bufLen = 0);
    ~vrpn_Tracker_TrivisioColibri();

	/// This function should be called each time through the main loop
	/// of the server code. It checks for a report from the tracker and
	/// sends it if there is one.
	virtual void mainloop();

protected:
    virtual void get_report();
    virtual void send_report();

    // Array of pointers to devices handles
    void** imu;
};


#endif
#endif