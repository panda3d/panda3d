#ifndef __TRACKER_ISENSE_H
#define __TRACKER_ISENSE_H

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"
#include "vrpn_Analog.h"
#ifdef  VRPN_INCLUDE_INTERSENSE
#ifdef __APPLE__
#define MACOSX
#endif
#include "../isense/isense.h"
#endif

#ifdef VRPN_INCLUDE_INTERSENSE
class VRPN_API vrpn_Tracker_InterSense : public vrpn_Tracker {
  
 public:

  vrpn_Tracker_InterSense(const char *name, 
                          vrpn_Connection *c,
                          int commPort, const char *additional_reset_commands = NULL,
					      int is900_timestamps = 0, int reset_at_start=0);

  ~vrpn_Tracker_InterSense();

  /// Add an IS900 button device to one of the sensors
  /// This allows configuration of an InterSense IS-900
  int	add_is900_button(const char *button_device_name, int sensor, int numbuttons = 5);

  /// Add the analog part of an IS900 joystick device to one of the sensors
  /// This allows configuration of an InterSense IS-900
  /// The optional parameters specify the clipping and scaling to take the reports
  /// from the two joystick axes into the range [-1..1].  The default is unscaled.
  int	add_is900_analog(const char *analog_device_name, int sensor,
	    double c0Min = -1, double c0Low = 0, double c0Hi = 0, double c0Max = 1,
	    double c1Min = -1, double c1Low = 0, double c1Hi = 0, double c1Max = 1);

  /// This function should be called each time through the main loop
  /// of the server code. It polls for a report from the tracker and
  /// sends it if there is one. It will reset the tracker if there is
  /// no data from it for a few seconds.

  virtual void mainloop();
    
 protected:
  
  virtual void get_report(void);
  virtual void reset();
  virtual void send_report(void);

  char	add_reset_cmd[2048];	//< Additional reset commands to be sent

  int m_CommPort;
  ISD_TRACKER_HANDLE m_Handle;
  ISD_TRACKER_INFO_TYPE m_TrackerInfo;
  ISD_STATION_INFO_TYPE m_StationInfo[ISD_MAX_STATIONS];

  int	do_is900_timestamps;	    //< Request and process IS-900 timestamps?
  int	m_reset_at_start;			//< should the tracker reset at startup

  struct timeval is900_zerotime;    //< When the IS-900 time counter was zeroed
  vrpn_Button_Server		*is900_buttons[ISD_MAX_STATIONS];	//< Pointer to button on each sensor (NULL if none)
  vrpn_Clipping_Analog_Server	*is900_analogs[ISD_MAX_STATIONS];	//< Pointer to analog on each sensor (NULL if none)

  /// Augments the basic Fastrak format to include IS900 features if needed
  int	set_sensor_output_format(int sensor);

  void  getTrackerInfo(char *msg);
};
#endif


#endif
