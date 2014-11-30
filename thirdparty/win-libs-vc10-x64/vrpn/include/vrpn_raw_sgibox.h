/*					vrpn_raw_sgibox.h
 *	
 *	This file describes the interface to an SGI dial & button box that
 * is connected through a serial interface.  This allows the control of
 * the boxes without going through the SGI GL library, rather using the
 * serial interface to connect with the device.
 */

#ifndef VRPN_RAW_SGIBOX
#define VRPN_RAW_SGIBOX

#include "vrpn_Analog.h"
#include "vrpn_Dial.h"
#include "vrpn_Button.h"
#ifndef _WIN32 
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

/* Number of buttons and number of dials on sgi button/dial boxes */
#define vrpn_SGI_NUM_BUTTONS (32)
#define vrpn_SGI_NUM_DIALS   (8)
#define vrpn_SGI_NUMDEVS (vrpn_SGI_NUM_BUTTONS+vrpn_SGI_NUM_DIALS)

class VRPN_API vrpn_raw_SGIBox :public vrpn_Analog, public vrpn_Dial, public vrpn_Button_Filter {
public:
  vrpn_raw_SGIBox(char * name, vrpn_Connection * c, char *serialDevName);
  void mainloop();
  int reset();
  int send_light_command();

protected:
  void get_report();
  void check_press_bank(int base_button, unsigned char base_command,
	  unsigned char command);
  void check_release_bank(int base_button, unsigned char base_command,
	  unsigned char command);
  
private:
  int	serialfd;		// Serial port that has been opened
  unsigned long btstat;           /* status of of on/off buttons */
  unsigned long bs1, bs2;         /* status of all buttons */
  short   vals1[vrpn_SGI_NUMDEVS];	// Value array?
  int	dial_changed[vrpn_SGI_NUM_DIALS];
  int	mid_values[vrpn_SGI_NUM_DIALS];	  //< Used to perform clamping
  int	last_values[vrpn_SGI_NUM_DIALS];	  //< Used by dial reporting code
};

#endif  // VRPN_RAW_SGIBOX
