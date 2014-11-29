/*
# Linux Joystick. Interface to the Linux Joystick driver by Vojtech Pavlik
# included in several Linux distributions. The server code has been tested 
# with Linux Joystick driver version 1.2.14. Yet, there is no way how to
# map a typical joystick's zillion buttons and axes on few buttons and axes
# really used. Unfortunately, even joysticks of the same kind can have 
# different button mappings from one to another.  Driver written by Harald
# Barth (haba@pdc.kth.se).
*/

#ifndef VRPN_JOYLIN
#define VRPN_JOYLIN
#include "vrpn_Analog.h"
#include "vrpn_Button.h"

#ifdef linux
#include <linux/joystick.h>
#endif

class VRPN_API vrpn_Joylin :public vrpn_Analog, public vrpn_Button {
public:
  vrpn_Joylin(char * name, vrpn_Connection * c, char * portname);

  void mainloop(void);

protected:
  int init();
private:
  int namelen;
  int fd;
  int version;
  char *name;
};


#endif
