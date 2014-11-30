#ifndef VRPN_MAGELLAN_H
#define VRPN_MAGELLAN_H

#include "vrpn_Analog.h"
#include "vrpn_Button.h"

class VRPN_API vrpn_Magellan: public vrpn_Serial_Analog
			,public vrpn_Button
{
public:
	vrpn_Magellan (const char * name, vrpn_Connection * c,
			const char * port, int baud, bool altreset = false);

	~vrpn_Magellan () {};

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();

	virtual int reset(void);	//< Set device back to starting config

  protected:
	bool _altreset;			//< Send alternative reset string?
	int _status;			//< Used by mainloop() and get_report()
	int _numbuttons;		//< How many buttons to open
	int _numchannels;		//< How many analog channels to open

	unsigned _expected_chars;		//< How many characters to expect in the report
	unsigned char _buffer[512];	//< Buffer of characters in report
	unsigned _bufcount;			//< How many characters we have so far

	int _null_radius;		//< The range over which no motion should be reported

	struct timeval timestamp;	//< Time of the last report from the device

	virtual	void clear_values(void);//< Set all buttons, analogs and encoders back to 0

	/// Try to read a report from the device.  Returns 1 if complete report received,
	/// 0 otherwise.  Sets _status to match current status.
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
