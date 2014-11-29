#ifndef VRPN_TNG3B_H
#include "vrpn_Analog.h"
#include "vrpn_Button.h"

// TNG stands for "Totally Neat Gadget."
// The TNG3 is an interface device from Mindtel <www.mindtel.com>.
// It is powered by the serial control lines and offers 8 digital 
// and 8 analog inputs.  It costs about $150.
// Written by Rob King at Navy Research Labs.

class VRPN_API vrpn_Tng3: public vrpn_Serial_Analog,
			 public vrpn_Button
{
 public:
    vrpn_Tng3 (const char * name, 
		       vrpn_Connection * c,
		       const char * port, 
		       int baud = 19200,         // only speed
		       const int numbuttons = 8, 
		       const int numchannels = 8);

    ~vrpn_Tng3 ()  {};

    // Called once through each main loop iteration to handle
    // updates.
    virtual void mainloop (void);
    
 protected:
    int _status;
    int _numbuttons;	// How many buttons to open
    int _numchannels;	// How many analog channels to open

    unsigned _expected_chars;	// How many characters to expect in the report
    unsigned char _buffer[512];	// Buffer of characters in report
    unsigned _bufcount;		// How many characters we have so far

    struct timeval _timestamp;	// Time of the last report from the device

    virtual void clear_values(void);	// Set all buttons, analogs back to 0
    virtual int reset(void);		// Set device back to starting config
    virtual int get_report(void);	// Try to read a report from the device

    // send report iff changed
    virtual void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
    // send report whether or not changed
    virtual void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

    // NOTE:  class_of_service is only applied to vrpn_Analog
    //  values, not vrpn_Button or vrpn_Dial

 private:

// utility routine to sync up the data stream
    int syncDatastream (double seconds);

    unsigned char bDataPacketStart;
};

#define VRPN_TNG3B_H
#endif
