#ifndef VRPN_RADAMEC_SPI_H
#define VRPN_RADAMEC_SPI_H

#include "vrpn_Connection.h"
#include "vrpn_Analog.h"

class VRPN_API vrpn_Radamec_SPI: public vrpn_Serial_Analog
{
public:
	vrpn_Radamec_SPI (const char * name, vrpn_Connection * c,
			const char * port, int baud = 38400);

	~vrpn_Radamec_SPI () {};

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();

  protected:
	int _status;		    //< Reset, Syncing, or Reading
	int _camera_id;		    //< What is our camera ID, queried from device
	int _numchannels;	    //< How many analog channels to open

	unsigned _expected_chars;	    //< How many characters to expect in the report
	unsigned char _buffer[512]; //< Buffer of characters in report
	unsigned _bufcount;		    //< How many characters we have so far

	struct timeval timestamp;   //< Time of the last report from the device

	virtual int reset(void);		//< Set device back to starting config
	virtual	int get_report(void);		//< Try to read a report from the device

	virtual void clear_values(void);	//< Clears all channels to 0

	/// Compute the CRC for the message or report starting at head with length len.
	unsigned char compute_crc(const unsigned char *head, int len);

	/// Convert a 24-bit value from a buffer into an unsigned integer value
	vrpn_uint32 convert_24bit_unsigned(const unsigned char *buf);

	/// Convert a 16-bit unsigned value from a buffer into an integer
	vrpn_int32  convert_16bit_unsigned(const unsigned char *buf);

	double	int_to_pan(vrpn_uint32 val);	//< Returns pan in degrees
	double	int_to_tilt(vrpn_uint32 val)	//< Returns tilt in degrees
		    { return int_to_pan(val); };
	double	int_to_zoom(vrpn_uint32 val);	//< Returns zoom in meters
	double	int_to_focus(vrpn_uint32 val);	//< Returns focal length in meters
	double	int_to_height(vrpn_uint32 val);	//< Returns height in meters
	double	int_to_X(vrpn_uint32 mm, vrpn_uint32 frac);   //< Returns X location in meters
	double	int_to_Y(vrpn_uint32 mm, vrpn_uint32 frac)    //< Returns Y location in meters
		    { return int_to_X(mm, frac); };
	double	int_to_orientation(vrpn_uint32 val); //< Returns orientation in degrees

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
