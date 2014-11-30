
#ifndef VRPN_VPJOYSTICK
#define VRPN_VPJOYSTICK 1

#include "vrpn_Button.h"


#define vrpn_VPJOY_MESSAGE_LENGTH (4)
#define vrpn_VPJOY_NUM_BUTTONS	(8)

class VRPN_API vrpn_VPJoystick : public vrpn_Button {

 public:
  vrpn_VPJoystick(char* name, vrpn_Connection *c,
             const char *port="/dev/ttyS0", long baud=9600);

  ~vrpn_VPJoystick();

  void mainloop();
   
 private:
  int serial_fd;
  unsigned char buffer[VRPN_BUTTON_BUF_SIZE];

  unsigned char message_buffer[ vrpn_VPJOY_MESSAGE_LENGTH ];
  unsigned int bytes_read;
  unsigned int button_masks[ vrpn_VPJOY_NUM_BUTTONS ]; 
  
  unsigned int state;
};

#endif // #ifndef VRPN_VPJOYSTICK
