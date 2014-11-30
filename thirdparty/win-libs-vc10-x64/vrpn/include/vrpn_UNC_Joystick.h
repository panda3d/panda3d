#ifndef VRPN_JOYSTICK
#define VRPN_JOYSTICK
#include "vrpn_Analog.h"
#include "vrpn_Button.h"

// This class runs the UNC custom serial joystick.  It includes two
// buttons, a slider, and two 3-axis joysticks.  It is based on a
// single-board computer.  This driver is based on the px_sjoy.c
// code.

class VRPN_API vrpn_Joystick :public vrpn_Serial_Analog, public vrpn_Button {
public:
  vrpn_Joystick(char * name, vrpn_Connection * c, char * portname,int
		baud, double);

  void mainloop(void);

protected:
  int get_report();
  void report(struct timeval current_time);
  void reset();
  void parse(int, int reset_rest_pos = 0);
private:
  unsigned char serialbuf[32];
  double restval[vrpn_CHANNEL_MAX];		// Initial value of each channel
  long MAX_TIME_INTERVAL;
};


#endif
