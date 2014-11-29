#ifndef VRPN_ZABER_H
#define VRPN_ZABER_H

#include "vrpn_Analog.h"
#include "vrpn_Analog_Output.h"

class VRPN_API vrpn_Zaber: public vrpn_Serial_Analog, public vrpn_Analog_Output
{
public:
	vrpn_Zaber (const char * name, vrpn_Connection * c,
			const char * port);
	~vrpn_Zaber () {};

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();

  protected:
	unsigned d_expected_chars;	      //< How many characters to expect in the report
	unsigned char d_buffer[512];  //< Buffer of characters in report
	unsigned d_bufcount;		      //< How many characters we have so far

	struct timeval timestamp;   //< Time of the last report from the device

	virtual int reset(void);		//< Set device back to starting config
	virtual	int get_report(void);		//< Try to read a report from the device

	bool  send_command(unsigned char devicenum, unsigned char cmd, vrpn_int32 data);
	bool  send_command(unsigned char devnum, unsigned char cmd, unsigned char d0,
	  unsigned char d1, unsigned char d2, unsigned char d3);
	vrpn_int32  convert_bytes_to_reading(const unsigned char *buf);

	/// send report iff changed
        virtual void report_changes
                   (vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE);
        /// send report whether or not changed
        virtual void report
                   (vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE);

      /// Responds to a request to change one of the values by
      /// setting the channel to that value.
      static int VRPN_CALLBACK handle_request_message(void *userdata, vrpn_HANDLERPARAM p);

      /// Responds to a request to change multiple channels at once.
      static int VRPN_CALLBACK handle_request_channels_message(void *userdata, vrpn_HANDLERPARAM p);

      /// Responds to a connection request with a report of the values
      static int VRPN_CALLBACK handle_connect_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif
