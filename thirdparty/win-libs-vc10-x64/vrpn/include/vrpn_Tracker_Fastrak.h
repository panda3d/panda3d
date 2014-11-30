// vrpn_Tracker_Fastrak.h
//	This file contains the class header for a Polhemus Fastrak Tracker.
// This file is based on the vrpn_3Space.h file, with modifications made
// to allow it to operate a Fastrak instead. The modifications are based
// on the old version of the Fastrak driver, which had been mainly copied
// from the Trackerlib driver and was very difficult to understand.
//	This version was written in the Summer of 1999 by Russ Taylor.
//	Later, it was updated to allow the extended reset commands needed
// to drive an IS-600 tracker.
//	Later (Summer of 2000), it was augmented to allow the button, analog,
// and timestamp capabilities of the InterSense IS-900 tracker.

#ifndef VRPN_TRACKER_FASTRAK_H
#define VRPN_TRACKER_FASTRAK_H

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

const int vrpn_FASTRAK_MAX_STATIONS = 4;    //< How many stations can exist

class VRPN_API vrpn_Tracker_Fastrak: public vrpn_Tracker_Serial {
  
 public:

  /// The constructor is given the name of the tracker (the name of
  /// the sender it should use), the connection on which it is to
  /// send its messages, the name of the serial port it is to open
  /// (default is /dev/ttyS1 (first serial port in Linux)), the baud
  /// rate at which it is to communicate (default 19200), whether
  /// filtering is enabled (default yes), and the number of stations
  /// that are possible on this Fastrak (default 4). The station select
  /// switches on the front of the Fastrak determine which stations are
  /// active. The final parameter is a string that can contain additional
  /// commands that are set to the tracker as part of its reset routine.
  /// These might be used to set the hemisphere or other things that are
  /// not normally included; see the Fastrak manual for a list of these.
  /// There can be multiple lines of them but putting <CR> into the string.

  vrpn_Tracker_Fastrak(const char *name, vrpn_Connection *c, 
		      const char *port = "/dev/ttyS1", long baud = 19200,
		      int enable_filtering = 1, int numstations = vrpn_FASTRAK_MAX_STATIONS,
		      const char *additional_reset_commands = NULL,
		      int is900_timestamps = 0);

  ~vrpn_Tracker_Fastrak();

  // is the version modified (debug@cs.unc.edu, 5/20/01) to handle Pol.Fastrak stylus buttons
  int add_fastrak_stylus_button(const char *button_device_name,
				int sensor, int numbuttons = 1);

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
    
 protected:
  
  virtual int get_report(void);
  virtual void reset();

  struct timeval reset_time;
  int	do_filter;		//< Should we turn on filtering for pos/orient?
  int	num_stations;		//< How many stations maximum on this Fastrak?
  char	add_reset_cmd[2048];	//< Additional reset commands to be sent

  // another modification by Debug, 5/23/01
  bool really_fastrak;  // to distinguish it from intersense when using buttons!!!

  // The following members provide support for the InterSense IS-900 features
  // that are beyond the standard Fastrak features.

  int	do_is900_timestamps;	    //< Request and process IS-900 timestamps?
  struct timeval is900_zerotime;    //< When the IS-900 time counter was zeroed
  vrpn_Button_Server		*is900_buttons[vrpn_FASTRAK_MAX_STATIONS];	//< Pointer to button on each sensor (NULL if none)
  vrpn_Clipping_Analog_Server	*is900_analogs[vrpn_FASTRAK_MAX_STATIONS];	//< Pointer to analog on each sensor (NULL if none)
  vrpn_uint32	REPORT_LEN;	    //< The length that the current report should be

  /// Augments the basic Fastrak format to include IS900 features if needed
  int	set_sensor_output_format(int sensor);

  /// Augments the basic Fastrak report length to include IS900 features if needed
  int	report_length(int sensor);
};

#endif
