#ifndef VRPN_NIKON_CONTROLS_H
#define VRPN_NIKON_CONTROLS_H

#include  "vrpn_Analog.h"
#include  "vrpn_Analog_Output.h"

class VRPN_API vrpn_Nikon_Controls : public vrpn_Serial_Analog, public vrpn_Analog_Output {
public:
  vrpn_Nikon_Controls(const char *device_name, vrpn_Connection *con = NULL, const char *port_name = "COM1");
  ~vrpn_Nikon_Controls(void) {};

  virtual void mainloop ();

protected:
  int _status;

  unsigned char _buffer[512]; //< Buffer of characters in report
  unsigned _bufcount;	      //< How many characters we have so far

  double  _requested_focus;   //< Where we asked the focus to be set to

  struct timeval timestamp;   //< Time of the last report from the device

  virtual int reset(void);		//< Set device back to starting config
  virtual int get_report(void);		//< Try to read a report from the device
  virtual int set_channel(unsigned chan_num, vrpn_float64 value); //< Try to set the focus to this

  /// Send changes since the last time
  virtual void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

  /// Send values whether or not they have changed.
  virtual void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

  /// Responds to a connection request with a report of the values
  static int VRPN_CALLBACK handle_connect_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Responds to a request to change one of the values by
  /// setting the channel to that value.
  static int VRPN_CALLBACK handle_request_message(void *userdata, vrpn_HANDLERPARAM p);

  /// Responds to a request to change multiple channels at once.
  static int VRPN_CALLBACK handle_request_channels_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif
