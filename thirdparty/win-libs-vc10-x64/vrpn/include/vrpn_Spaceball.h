#ifndef VRPN_SPACEBALL_H
#define VRPN_SPACEBALL_H

#include "vrpn_Analog.h"
#include "vrpn_Button.h"

class VRPN_API vrpn_Spaceball: public vrpn_Serial_Analog
			,public vrpn_Button
{
  public:
	vrpn_Spaceball (const char * name, vrpn_Connection * c,
			const char * port, int baud);

	~vrpn_Spaceball () {};

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();

	virtual int reset(void);  ///< Set device back to starting config

  protected:
	int _numbuttons;          ///< How many buttons to open
	int _numchannels;         ///< How many analog channels to open
	unsigned char buf[512];	  ///< Buffer of characters in report,
	int bufpos;               ///< Current char pos in buffer 
        int packtype;             ///< What kind of packet we are decoding
        int packlen;              ///< Expected packet length
        int escapedchar;          ///< We're processing an escaped char
        int erroroccured;         ///< A device error has occured
        int resetoccured;         ///< A reset event has occured
        int spaceball4000;        ///< We found a Spaceball 4000
        int leftymode4000;        ///< Spaceball 4000 is in lefty mode
	int null_radius;          ///< range where no motion should be reported
	struct timeval timestamp; ///< Time of the last report from the device

	virtual	void clear_values(void); ///< Set all buttons, analogs and encoders back to 0

	/// Try to read reports from the device.  Returns 1 if a complete 
        /// report received, 0 otherwise.  Sets status to current mode.
	virtual	int get_report(void);

	/// send report iff changed
        virtual void report_changes
                   (vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_LOW_LATENCY);

        /// send report whether or not changed
        virtual void report
                   (vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_LOW_LATENCY);

        // NOTE:  class_of_service is only applied to vrpn_Analog
        //  values, not vrpn_Button, which are always vrpn_RELIABLE
};

#endif
