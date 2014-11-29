#ifndef VRPN_GLOBALHAPTICSORB_H
#define VRPN_GLOBALHAPTICSORB_H

#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Dial.h"

// Exports buttons 0-25 as 0-25.
// Exports left pushbutton as 26, right pushbutton as 27.
// Exports rocker up as 28, rocker down as 29.
// Exports Thumbwheel both as clamping analog (-1..1) 0 and as dial 0.
// Exports Trackball both as clamping analogs 1 and 2 and as dials 1 and 2.

class VRPN_API vrpn_GlobalHapticsOrb: public vrpn_Serial_Analog
			,public vrpn_Button
			,public vrpn_Dial
{
public:
	vrpn_GlobalHapticsOrb (const char * name, vrpn_Connection * c,
			const char * port, int baud);
	~vrpn_GlobalHapticsOrb () {};

	// Called once through each main loop iteration to handle
	// updates.
	virtual void mainloop ();

  protected:
	int d_status;

	unsigned d_expected_chars;	//< How many characters to expect in the report
	unsigned char d_buffer[512];	//< Buffer of characters in report
	unsigned d_bufcount;		//< How many characters we have so far

	struct timeval d_timestamp;	//< Time of the last report from the device

	virtual	void clear_values(void);	//< Set all buttons, analogs and encoders back to 0
	virtual int reset(void);		//< Set device back to starting config
	virtual	int get_report(void);		//< Try to read a report from the device

        // NOTE:  class_of_service is only applied to vrpn_Analog
        //  values, not vrpn_Button or vrpn_Dial
	/// Send report iff changed
        virtual void report_changes(vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_RELIABLE);
        /// Send report whether or not changed
        virtual void report(vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_RELIABLE);

	/// Clear all of the values when we get our first client connection request
	static	int VRPN_CALLBACK handle_firstConnection(void * userdata, vrpn_HANDLERPARAM);
};

#endif
