#ifndef VRPN_CEREALBOX_H
#define VRPN_CEREALBOX_H

#include "vrpn_Connection.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Dial.h"

class VRPN_API vrpn_CerealBox: public vrpn_Serial_Analog
			,public vrpn_Button
			,public vrpn_Dial
{
public:
	vrpn_CerealBox (const char * name, vrpn_Connection * c,
			const char * port, int baud,
			const int numbuttons, const int numchannels, const int numencoders);

	~vrpn_CerealBox () {};

	// Called once through each main loop iteration to handle
	// updates.
	virtual void mainloop ();

  protected:
	int _status;
	int _numbuttons;	// How many buttons to open
	int _numchannels;	// How many analog channels to open
	int _numencoders;	// How many encoders to open

	unsigned _expected_chars;	// How many characters to expect in the report
	unsigned char _buffer[512];	// Buffer of characters in report
	unsigned _bufcount;		// How many characters we have so far

	struct timeval timestamp;	// Time of the last report from the device

	virtual	void clear_values(void);	// Set all buttons, analogs and encoders back to 0
	virtual int reset(void);		// Set device back to starting config
	virtual	int get_report(void);		// Try to read a report from the device

	// send report iff changed
        virtual void report_changes
                   (vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_LOW_LATENCY);
        // send report whether or not changed
        virtual void report
                   (vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_LOW_LATENCY);

          // NOTE:  class_of_service is only applied to vrpn_Analog
          //  values, not vrpn_Button or vrpn_Dial
};

#endif
