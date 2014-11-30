#ifndef SPACE_H
#define SPACE_H

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"

class VRPN_API vrpn_Tracker_3Space: public vrpn_Tracker_Serial {
  
 public:
  
  vrpn_Tracker_3Space(char *name, vrpn_Connection *c,
		      const char *port = "/dev/ttyS1", long baud = 19200) :
  vrpn_Tracker_Serial(name,c,port,baud) {};
    
 protected:
  
  /// Returns 0 if didn't get a complete report, 1 if it did.
  virtual int get_report(void);

  virtual void reset();

};

#endif
