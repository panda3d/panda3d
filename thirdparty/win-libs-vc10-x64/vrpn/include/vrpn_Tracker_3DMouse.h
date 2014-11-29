#ifndef __TRACKER_3DMOUSE_H
#define __TRACKER_3DMOUSE_H

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"

class VRPN_API vrpn_Tracker_3DMouse : public vrpn_Tracker_Serial, public vrpn_Button {
  
 public:

  vrpn_Tracker_3DMouse(const char *name, 
                          vrpn_Connection *c,
                          const char *port = "/dev/ttyS1",
						  long baud = 19200,
						  int filtering_count = 1);

  ~vrpn_Tracker_3DMouse();

  /// Called once through each main loop iteration to handle updates.
  virtual void mainloop();

    
 protected:

  virtual void reset();
  virtual int get_report(void);
  bool set_filtering_count(int count);
  virtual void clear_values(void);

  unsigned char		_buffer[2048];
  int				_filtering_count;
  int				_numbuttons;
};

#endif
