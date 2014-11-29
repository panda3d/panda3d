///////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:        vrpn_Tracker_ViewPoint.h
//
// Author:      David Borland
//
//				EventLab at the University of Barcelona
//
// Description: VRPN server class for Arrington Research ViewPoint EyeTracker.
//
//              The VRPN server connects to the eye tracker using the VPX_InterApp DLL.  
//              Whatever other control software is being used to connect to the eye tracker
//              (e.g. the ViewPoint software that comes with the tracker) to perform 
//              calibration, etc. should link to the same copy of the DLL, so they can share 
//              information.
//
//              -------------------------------------------------------------------------------
//
//              Tracker:
//
//              The tracker has two sensors, as the ViewPoint can optionally have binocular
//              tracking.  In the case of monocular tracking, only sensor 0 (EYE_A) will have 
//              valid information.  Retrieving smoothed or raw tracking data is controlled by 
//              the smoothedData parameter.
//
//              Position: The (x,y) gaze point in gaze space (smoothed or raw).
//              
//              Rotation: The (x,y) gaze angle as a quaternion (smoothed or raw).
//              
//              Velocity: The x- and y- components of the eye movement velocity in gaze space 
//                        (always smoothed).
//
//              -------------------------------------------------------------------------------
//
//              Analog:
//
//              There are a lot of additional data that can be retrieved from the tracker.
//              These values are always calculated from the smoothed gaze point.  Currently, 
//              the following are sent as analog values, but more can be added as needed.
//              Please see the ViewPoint documentation regarding what other data are available.
//
//              Because each channel needs to be duplicated in the case of a binocular tracker, 
//              the first n/2 values are for EYE_A, and the second n/2 values are for EYE_B.  
//              
//              EYE_A:
//  
//              Channel 0: The pupil aspect ratio, from 0.0 to 1.0.  Can be used to detect 
//                         blinks when it falls below a given threshold.
//
//              Channel 1: The total velocity (magnitude of eye movement velocity).  Can be 
//                         used to detect saccades.
//
//              Channel 2: The fixation seconds (length of time below the velocity criterion
//                         used to detect saccades).  0 if saccade is occurring.
//
//              EYE_B:
//
//              Channels 3-5: See EYE_A.
//
/////////////////////////////////////////////////////////////////////////////////////////////// 

#ifndef VRPN_TRACKER_VIEWPOINT
#define VRPN_TRACKER_VIEWPOINT

// Make sure ViewPoint EyeTracker is being used
#include "vrpn_Configure.h"
#ifdef VRPN_USE_VIEWPOINT

#include "vrpn_Tracker.h" 
#include "vrpn_Analog.h"

class vrpn_Tracker_ViewPoint : public vrpn_Tracker, public vrpn_Analog {
public:
    // Constructor
    //
    // name:			VRPN tracker name
    //
    // c:				VRPN connection to use
	//
	// smoothedData:	Get smoothed data or raw data for tracker values.
    //
    vrpn_Tracker_ViewPoint(const char* name, vrpn_Connection* c, bool smoothedData = true);
    ~vrpn_Tracker_ViewPoint();

	/// This function should be called each time through the main loop
	/// of the server code. It checks for a report from the tracker and
	/// sends it if there is one.
	virtual void mainloop();

protected:
    virtual void get_report();

	virtual void get_tracker();
	virtual void get_analog();

    virtual void send_report();

	bool useSmoothedData;
};


#endif
#endif