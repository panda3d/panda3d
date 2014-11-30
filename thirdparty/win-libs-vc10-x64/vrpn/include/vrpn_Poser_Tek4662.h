#ifndef vrpn_POSER_TEK4662_H
#define vrpn_POSER_TEK4662_H

#ifndef _WIN32_WCE
#include <time.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#ifndef _WIN32_WCE
#include <sys/time.h>
#endif
#endif

#include "vrpn_Poser.h"
#include "vrpn_Tracker.h"
#include "vrpn_Serial.h"

// This code is for a Poser server that uses a Tektronix 4662 plotter in the
// RS-232 mode.  This is a 2D device.
// This class also acts as a vrpn_Tracker so that it can report back the new positions
// to the client.  This lets the system operate in a closed-loop fashion.
//
// It assumes the following settings on the plotter: GIN terminator (none), no
// DEL modification, device A, copy mode off, 1 stop bit, CR translation off,
// low-speed plotting off, RS-232 interface enabled. It assumes that the plotter
// is not in local mode and that the communications is through the Modem port.

// If multiple requests for motion to a new location arrive while an ongoing
// move is being made, all but the last are ignored.

class VRPN_API vrpn_Poser_Tek4662 : public vrpn_Poser, public vrpn_Tracker {
public:
  vrpn_Poser_Tek4662(const char* name, vrpn_Connection* c,
	      const char * port, int baud = 1200, int bits = 8, 
              vrpn_SER_PARITY parity = vrpn_SER_PARITY_NONE);

  virtual ~vrpn_Poser_Tek4662();

  virtual void mainloop();

protected:
  int		d_serial_fd;	    //< File descriptor for the serial port we're using
  unsigned char d_inbuf[1024];	    //< Input buffer for characters from the plotter
  int		d_inbufcounter;	    //< How many characters have been read

  float		d_newx, d_newy;	    //< New location that we're supposed to go to
  bool		d_new_location_requested; //< Has a new location been requested since our last move?
  int		d_outstanding_requests;	  //< How many GIN requests are outstanding?

  void	reset(void);
  void	run();

  static int VRPN_CALLBACK handle_change_message(void *userdata, vrpn_HANDLERPARAM p);
  static int VRPN_CALLBACK handle_vel_change_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif

