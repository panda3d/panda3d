#ifndef vrpn_5dt16_H
#define vrpn_5dt16_H

#include "vrpn_Connection.h"
#include "vrpn_Analog.h"

// This class will read the finger-pad values of the 5DT glove as analogs
// and send them.  Use the vrpn_Button_5DT_Server class below if you want
// to turn them into buttons by thresholding them.

class VRPN_API vrpn_5dt16: public vrpn_Serial_Analog
{
public:
	vrpn_5dt16 (const char * name,
		  vrpn_Connection * c,
		  const char * port,
		  int baud = 19200);

	~vrpn_5dt16 () {};

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();
	
  protected:
	int _status;		    //< Reset, Syncing, or Reading
	int _numchannels;	    //< How many analog channels to open
	int _mode ;                 //< glove mode for reporting data (see glove manual)
	unsigned _expected_chars;	    //< How many characters to expect in the report
	unsigned char _buffer[512]; //< Buffer of characters in report
	unsigned _bufcount;		    //< How many characters we have so far
	bool  _tenbytes;	    //< Whether there are 10-byte responses (unusual, but seen)

	struct timeval timestamp;   //< Time of the last report from the device

	virtual int reset(void);	//< Set device back to starting config
	virtual	void get_report(void);	//< Try to read a report from the device

	virtual void clear_values(void);	//< Clears all channels to 0

	/// send report iff changed
        virtual void report_changes
                   (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
        /// send report whether or not changed
        virtual void report
                   (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
};

#include "vrpn_Button.h"

// 5dt16 button server code.   This device will listen to a 5dt16 analog server
// and report button press and release event when the analog pass a threshold
// value.
// This class is derived from the vrpn_Button_Filter class, so that it
// can be made to toggle its buttons using messages from the client.
class VRPN_API vrpn_Button_5DT_Server : public vrpn_Button_Filter
{
public:
	// Buttons are considered pressed when their analog value exceeds the
	// threshold value.
        vrpn_Button_5DT_Server(const char *name, const char *deviceName, vrpn_Connection *c,
				double threshold[16]);
        ~vrpn_Button_5DT_Server();

        virtual void mainloop();

protected:
        static void     VRPN_CALLBACK handle_analog_update (void * userdata, const vrpn_ANALOGCB info);
        vrpn_Analog_Remote *d_5dt_button;
        double m_threshold[16];
};

#endif

