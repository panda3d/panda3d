#ifndef VRPN_IMMERSIONBOX_H
#define VRPN_IMMERSIONBOX_H
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Dial.h"

// Written by Rob King at Navy Research Labs.  The button code works;
// the others are not fully implemented.

class VRPN_API vrpn_ImmersionBox: public vrpn_Serial_Analog,
			 public vrpn_Button,
			 public vrpn_Dial
{
 public:
    vrpn_ImmersionBox (const char * name, 
		       vrpn_Connection * c,
		       const char * port, 
		       int baud,
		       const int numbuttons, 
		       const int numchannels, 
		       const int numencoders);

    ~vrpn_ImmersionBox ()  {};

    // Called once through each main loop iteration to handle
    // updates.
    virtual void mainloop (void);
    
 protected:
    int _status;
    int _numbuttons;	// How many buttons to open
    int _numchannels;	// How many analog channels to open
    int _numencoders;	// How many encoders to open

    unsigned _expected_chars;	// How many characters to expect in the report
    unsigned char _buffer[512];	// Buffer of characters in report
    unsigned _bufcount;		// How many characters we have so far

    struct timeval timestamp;	// Time of the last report from the device

    virtual void clear_values(void);	// Set all buttons, analogs and encoders back to 0
    virtual int reset(void);		    // Set device back to starting config
    virtual int get_report(void);		// Try to read a report from the device

    // send report iff changed
    virtual void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
    // send report whether or not changed
    virtual void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

    // NOTE:  class_of_service is only applied to vrpn_Analog
    //  values, not vrpn_Button or vrpn_Dial

 private:

#define MAX_IENCODERS 6	
#define MAX_ICHANNELS 8	
#define MAX_IBUTTONS	 7
#define MAX_IBOX_STRING 32

// utility routine to sync up the baudrate
    int syncBaudrate (double seconds);

// utility to read a string from the ibox
    int sendIboxCommand (char cmd, char * returnString, double delay);

// identification strings obtained from the ibox
    char iname  [MAX_IBOX_STRING];
    char comment[MAX_IBOX_STRING];
    char serial [MAX_IBOX_STRING];
    char id     [MAX_IBOX_STRING];
    char model  [MAX_IBOX_STRING];
    char vers   [MAX_IBOX_STRING];
    char parmf  [MAX_IBOX_STRING];
    
    
// stores the byte sent to the ibox 
    unsigned char commandByte;
    unsigned char dataPacketHeader;
    int  dataRecordLength;

// makes a command byte, given the user's choice of time stamping and the number of 
// reports desired from each type of sensor
// also calculates the expected number of bytes in each report that follow the packet header
    inline void setupCommand (int useTimeStamp, 
			      unsigned int numAnalog, 
			      unsigned int numEncoder) {
	commandByte = (unsigned char) (
	    (useTimeStamp ? 0x20 : 0) |
	    (numAnalog  > 4 ? 0x0C : (numAnalog  > 2 ? 0x08 : (numAnalog  ? 0x04 : 0 ) ) ) |
	    (numEncoder > 3 ? 0x03 : (numEncoder > 2 ? 0x02 : (numEncoder ? 0x01 : 0 ) ) ) );
	
	dataPacketHeader = (unsigned char)(commandByte | 0x80);

	// packet header
	// button status  
	// (optionally) 2 bytes of timer data
	// 8,4,2,or 0 analog   @1 byte each + byte to hold extra controller bits 
	// 6,4,2,or 0 encoders @2 bytes each 
	dataRecordLength =  1 + (useTimeStamp ? 2 : 0)  +
	    (numAnalog > 4 ? 9 :(numAnalog > 2 ? 5: (numAnalog ? 3 : 0 ) ) )  +
	    (numEncoder > 4 ? 12 :(numEncoder > 2 ? 8: (numEncoder ? 4: 0 ) ) );
    };

};

#endif
