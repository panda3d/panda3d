// vrpn_Tracker_GPS.h
// This file contains the header for the VRPN GPS server.  This server
// reads NMEA messages from a serial GPS.
// This implementation can convert to UTM meter-based coordinates.

#ifndef VRPN_TRACKER_GPS_H
#define VRPN_TRACKER_GPS_H

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

#include "gpsnmealib/nmeaParser.h"
#include "gpsnmealib/utmCoord.h"
#include "gpsnmealib/latLonCoord.h" //-eb

class vrpn_Tracker_GPS: public vrpn_Tracker_Serial {
  
 public:

  vrpn_Tracker_GPS(const char *name, 
                   vrpn_Connection *c, 
             	   const char *port = "/dev/ttyS1", 
				   long baud = 4800,
				   int utmFlag = 1,  // report in UTM coordinates if possible
				   int testFileFlag = 0,
				   const char* startStr = "RMC"); // the sentence to use as the "start" of a sequence

  ~vrpn_Tracker_GPS();


  /// This function should be called each time through the main loop
  /// of the server code. It polls for a report from the tracker and
  /// sends it if there is one. It will reset the tracker if there is
  /// no data from it for a few seconds.

  //  virtual void mainloop();
    
 protected:
	 // need a bigger buffer
  unsigned char buffer[VRPN_TRACKER_BUF_SIZE*10];// Characters read in from the tracker so far
 
  virtual int get_report(void);
  virtual void reset();

  struct timeval reset_time;

  FILE *testfile;
  char testfilename[256];

  // an nmeaParser object and associated data objects
  NMEAData nmeaData;
  NMEAParser nmeaParser;
  UTMCoord utmCoord;
  int useUTM;
};

#endif
