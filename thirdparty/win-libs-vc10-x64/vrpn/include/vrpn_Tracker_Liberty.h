// vrpn_Tracker_Liberty.h
//	This file contains the class header for a Polhemus Liberty Tracker.
// This file is based on the vrpn_Tracker_Fastrak.h file, with modifications made
// to allow it to operate a Liberty instead.
//	It has been tested on Linux and relies on being able to open the USB
// port as a named port and using serial commands to communicate with it.

#ifndef VRPN_TRACKER_LIBERTY_H
#define VRPN_TRACKER_LIBERTY_H

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

const int vrpn_LIBERTY_MAX_STATIONS = 8;    //< How many stations can exist
const int vrpn_LIBERTY_MAX_WHOAMI_LEN = 1024; //< Maximum whoami response length

class VRPN_API vrpn_Tracker_Liberty: public vrpn_Tracker_Serial {
  
 public:

  /// The constructor is given the name of the tracker (the name of
  /// the sender it should use), the connection on which it is to
  /// send its messages, the name of the serial port it is to open
  /// (default is /dev/ttyS0 (first serial port in Linux)), the baud
  /// rate at which it is to communicate (default 19200), whether
  /// filtering is enabled (default yes), and the number of stations
  /// that are possible on this Liberty (default 4). The station select
  /// switches on the front of the Liberty determine which stations are
  /// active. The final parameter is a string that can contain additional
  /// commands that are set to the tracker as part of its reset routine.
  /// These might be used to set the hemisphere or other things that are
  /// not normally included; see the Liberty manual for a list of these.
  /// There can be multiple lines of them but putting <CR> into the string.

  vrpn_Tracker_Liberty(const char *name, vrpn_Connection *c, 
		      const char *port = "/dev/ttyS0", long baud = 115200,
		      int enable_filtering = 1, int numstations = vrpn_LIBERTY_MAX_STATIONS,
		      const char *additional_reset_commands = NULL, int whoamilen = 195);

  ~vrpn_Tracker_Liberty();
    
  /// Add a stylus (with button) to one of the sensors.
  int add_stylus_button(const char *button_device_name,
				int sensor, int numbuttons = 1);
 protected:
  
  virtual int get_report(void);
  virtual void reset();

  struct timeval reset_time;
  int	do_filter;		//< Should we turn on filtering for pos/orient?
  int	num_stations;		//< How many stations maximum on this Liberty?
  char	add_reset_cmd[2048];	//< Additional reset commands to be sent
  int whoami_len;

  struct timeval liberty_zerotime;    //< When the liberty time counter was zeroed
  struct timeval liberty_timestamp; //< The time returned from the Liberty System
  vrpn_uint32	REPORT_LEN;	    //< The length that the current report should be

  vrpn_Button_Server	*stylus_buttons[vrpn_LIBERTY_MAX_STATIONS];	//< Pointer to button on each sensor (NULL if none)

  /// Augments the basic Liberty format
  int	set_sensor_output_format(int sensor);

  /// Augments the basic Liberty report length
  int	report_length(int sensor);
};

#endif
