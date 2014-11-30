#ifndef FLOCK_H
#define FLOCK_H

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"

// only 13 receivers allowed in normal addressing mode
#define MAX_SENSORS 13

// This is a class which provides a server for an Ascension 
// Flock of Birds tracker.  The server will send out messages
// at whatever rate the flock of bird's stream mode feeds them.
// Default filtering is active.
// The timestamp is the time when the first character was read
// from the serial driver with "read".  No adjustment is currently
// made to this time stamp.

// Adelstein, et al, "Dynamic response of electromagnetic spatial
// displacement trackers", Presence 5(3) found that if all of the filters
// are turned off, and the time required to transport the data to the
// server is ignored, then the latency of the flock is about 2 ms for ori,
// 7.5 ms for pos for the 0-4hz range (ie, when the data is ready to leave
// the flock in stream mode, the pos is 7.5 ms old, and the ori is 2 ms
// old).  Later this data will be combined with the baud rate to provide a
// more accurate time stamp for certain flock modes.

// If this is running on a non-linux system, then the serial port driver
// is probably adding more latency -- see the vrpn README for more info.

// The FOB now doesn't need to be used in a chain starting with an Exteded
// range controller.  You may use the optionnal useERT to set whether this
// is the case or not.  Added by David Nahon, for Virtools VR Pack,
// david@z-a.net, support@virtools.com

class VRPN_API vrpn_Tracker_Flock: public vrpn_Tracker_Serial {
  
 public:
  vrpn_Tracker_Flock(char *name, vrpn_Connection *c, int cSensors=1,
		     const char *port = "/dev/ttyd3", long baud = 38400,
		     int fStreamMode = 1, int useERT=1, bool invertQuaternion = false, int active_hemisphere=HEMI_PLUSZ);
  virtual ~vrpn_Tracker_Flock();
    
  enum {HEMI_PLUSX, HEMI_MINUSX, HEMI_PLUSY, HEMI_MINUSY, HEMI_PLUSZ, HEMI_MINUSZ};

 protected:

  int activeHemisphere;

  virtual int get_report(void);
  virtual void send_report(void);
  virtual void reset();
  void printError(unsigned char uchErrCode, unsigned char uchExpandedErrCode);
  int checkError();
  int cSensors;
  int fStream;  // stream or polled mode
  int fGroupMode; // for get_report -- group report mode or individual

  int d_useERT;	// do we have an extended range transmitter, this was the default
  bool d_invertQuaternion;	// Do we invert the Quaternion before returning it?

  // class members used to help with error recovery
  unsigned cResets;
  unsigned cSyncs;

  // class members used for statistics only
  int fFirstStatusReport;
  struct timeval tvLastStatusReport;
  int cReports;
  int cStatusInterval;
  double getMeasurementRate();
};

#endif
