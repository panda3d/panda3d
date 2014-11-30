// vrpn_Tracker_Isotrak.h
//  This file contains the code to operate a Polhemus Isotrack Tracker.
//  This file is based on the vrpn_Tracker_Fastrack.C file, with modifications made
//  to allow it to operate a Isotrack instead. The modifications are based
//  on the old version of the Isotrack driver.
//  This version was written in the Spring 2006 by Bruno Herbelin.


#ifndef VRPN_TRACKER_ISOTRAK_H
#define VRPN_TRACKER_ISOTRAK_H

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
#include "quat.h"


const int vrpn_ISOTRAK_MAX_STATIONS = 2;    // How many stations can exist

class VRPN_API vrpn_Tracker_Isotrak: public vrpn_Tracker_Serial {
  
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

  vrpn_Tracker_Isotrak(const char *name, vrpn_Connection *c, 
		      const char *port = "/dev/ttyS1", long baud = 19200,
		      int enable_filtering = 1, int numstations = vrpn_ISOTRAK_MAX_STATIONS,
		      const char *additional_reset_commands = NULL);

  ~vrpn_Tracker_Isotrak();

    /// Add a stylus (with button) to one of the sensors.
    int add_stylus_button(const char *button_device_name, int sensor);

 protected:
  
  virtual int get_report(void);
  virtual void reset();

  struct timeval reset_time;
  int	do_filter;		//< Should we turn on filtering for pos/orient?
  int	num_stations;		//< How many stations maximum on this Isotrak?

  char	add_reset_cmd[2048];	//< Additional reset commands to be sent
  
  int	set_sensor_output_format(int sensor);

  // An Isotrak can have stylus's with buttons on them
  vrpn_Button_Server   *stylus_buttons[vrpn_ISOTRAK_MAX_STATIONS];

private:
    void process_binary();
};

#endif
