#ifndef VRPN_SGIBOX
#define VRPN_SGIBOX

#ifdef sgi

#include "vrpn_Analog.h"
#include "vrpn_Button.h"
// Do we really need this here as everything is already
// surrounded by #ifdef sgi...???
#ifndef _WIN32 
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif
#ifndef MICROSCAPE_H
#include <gl/gl.h>
#include <gl/device.h>
#endif

/* Number of buttons and number of dials on sgi button/dial boxes */
#define NUM_BUTTONS (32)
#define NUM_DIALS   (8)
#define NUMDEVS (NUM_BUTTONS+NUM_DIALS)

class VRPN_API vrpn_SGIBox :public vrpn_Analog, public vrpn_Button_Filter {
public:
  vrpn_SGIBox(char * name, vrpn_Connection * c);
  void mainloop();
  void reset();


protected:
  void get_report();

  
private:
  double resetval[vrpn_CHANNEL_MAX];
  long MAX_TIME_INTERVAL;

  unsigned long btstat;           /* status of of on/off buttons */
  unsigned long bs1, bs2;         /* status of all buttons */
  unsigned long *bpA, *bpB, *bpT; /* ptrs to above */
  Device  devs[NUMDEVS];          /* device array */
  short   vals1[NUMDEVS],
    vals2[NUMDEVS];         /* two values arrays */
  int	dial_changed[NUM_DIALS];
  int  mid_values[NUM_DIALS];
  int winid;
  int sid;// server id;
};

#endif  // sgi

#endif  // VRPN_SGIBOX

