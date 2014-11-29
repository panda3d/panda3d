#ifndef INCLUDED_DYNA
#define INCLUDED_DYNA

#include "vrpn_Tracker.h"
#include "vrpn_Serial.h"

// only 13 receivers allowed in normal addressing mode
#define MAX_SENSORS 13

// This is a class which provides a server for an ascension 
// DynaSight.  The server will send out messages
// The timestamp is the time when the first character was read
// from the serial driver with "read".  No adjustment is currently
// made to this time stamp.

// If this is running on a non-linux system, then the serial port driver
// is probably adding more latency -- see the vrpn README for more info.

class VRPN_API vrpn_Tracker_Dyna: public vrpn_Tracker_Serial {
private:
  unsigned reportLength;
  unsigned totalReportLength;

 public:
  
  vrpn_Tracker_Dyna(char *name, vrpn_Connection *c, int cSensors=1,
		      const char *port = "/dev/ttyd3", long baud = 38400);
    
  virtual ~vrpn_Tracker_Dyna();
    
private:
  void my_flush() {
    // clear the input data buffer 
    unsigned char foo[128];
    while (vrpn_read_available_characters(serial_fd, foo, 1) > 0) ;
  }
  int valid_report();
  int decode_record();
  int get_status();
 protected:

  virtual int get_report(void);
  virtual void reset();
  void printError(unsigned char uchErrCode, unsigned char uchExpandedErrCode);
  int checkError();
  int cResets;
  int cSensors;
};


#endif
