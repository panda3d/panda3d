#ifndef VRPN_5DT_H
#define VRPN_5DT_H

#include "vrpn_Connection.h"
#include "vrpn_Analog.h"

/** @brief Class to support reading data from serial 5DT data gloves.
*/
class VRPN_API vrpn_5dt: public vrpn_Serial_Analog
{
public:
	/** @brief Constructor.
		@param name Name for the device
		@param c Connection to use.
		@param port serial port to connect to
		@param baud Baud rate - 19200 for "wired"-type gloves (send/receive),
			9600 implies a "wireless" (may be wired, but is send-only) glove
		@param mode Set to 1 for the driver to request reports, set to 2
			to stream them. (wireless implies 2, overriding value passed here)
		@param tenbytes Whether reports should be 10 bytes instead of
			the documented 9. (wireless implies true, overriding value passed here)
	*/
	vrpn_5dt (const char * name,
		  vrpn_Connection * c,
		  const char * port,
		  int baud = 19200,
		  int mode = 1,
		  bool tenbytes = false);

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();
	
	void syncing (void);

  protected:
	bool _wireless;			//< Whether this glove is using the wireless protocol
	bool _gotInfo;			//< Whether we've sent a message about this wireless glove
	int _status;		    //< Reset, Syncing, or Reading
	int _numchannels;	    //< How many analog channels to open
	int _mode ;                  //< glove mode for reporting data (see glove manual)
	unsigned _expected_chars;	    //< How many characters to expect in the report
	unsigned char _buffer[512]; //< Buffer of characters in report
	unsigned _bufcount;		    //< How many characters we have so far
	bool  _tenbytes;	    //< Whether there are 10-byte responses (unusual, but seen)

	struct timeval timestamp;   //< Time of the last report from the device

	virtual int reset(void);		//< Set device back to starting config
	virtual	void get_report(void);		//< Try to read a report from the device

	virtual void clear_values(void);	//< Clears all channels to 0

	/// Compute the CRC for the message, append it, and send message.
	/// Returns 0 on success, -1 on failure.
	int send_command(const unsigned char *cmd, int len);

	/// send report iff changed
        virtual void report_changes
                   (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
        /// send report whether or not changed
        virtual void report
                   (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
};

#endif
