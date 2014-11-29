#ifndef VRPN_WANDA
#define VRPN_WANDA
#include "vrpn_Analog.h"
#include "vrpn_Button.h"

// This is a driver for the Wanda device, which is an analog and
// button device.  You can find out more at http://home.att.net/~glenmurray/
// This driver was written at Brown University.

class VRPN_API vrpn_Wanda :public vrpn_Serial_Analog, public vrpn_Button {
public:
  vrpn_Wanda(char * name, vrpn_Connection * c, char * portname,int
	     baud, double);

  void mainloop(void);

protected:
  void report_new_button_info();
  void report_new_valuator_info();

private:
  double resetval[vrpn_CHANNEL_MAX];
  long MAX_TIME_INTERVAL;
  int  bytesread;
};


#endif
