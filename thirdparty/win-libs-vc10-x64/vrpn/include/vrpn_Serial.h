#ifndef	VRPN_SERIAL_H
#define VRPN_SERIAL_H
#ifndef _WIN32_WCE
#include <time.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#ifndef _WIN32_WCE
#include <io.h>
#endif
#else
#include <sys/time.h>
#endif

// vrpn_Serial
//
// Russ Taylor, 1998

// Pulls all the serial port routines into one file to make porting to
// new operating systems easier.

typedef enum {vrpn_SER_PARITY_NONE, vrpn_SER_PARITY_ODD, vrpn_SER_PARITY_EVEN,
			vrpn_SER_PARITY_MARK, vrpn_SER_PARITY_SPACE} vrpn_SER_PARITY;

// flush discards characters in buffer
// drain blocks until they are written

// Open a serial port, given its name and baud rate. Settings are 8 bits,
// no parity, 1 start and stop bits.  Also, set the port so that it will
// return immediately if there are no characters or less than the number
// of characters requested.  Returns the file descriptor on success,
// -1 on failure.
extern VRPN_API int vrpn_open_commport(const char *portname, long baud, int charsize = 8, vrpn_SER_PARITY parity = vrpn_SER_PARITY_NONE);

// Set and clear functions for the RTS ("ready to send") hardware flow-
// control bit.  These are used on a port that is already open.  Some
// devices (like the Ascension Flock of Birds) use this to reset the
// device.  Return 0 on success, nonzero on error.
extern VRPN_API int vrpn_set_rts(int comm);
extern VRPN_API int vrpn_clear_rts(int comm);

extern VRPN_API int vrpn_close_commport(int comm);
// Throw out any characters within the input buffer.
//  Return 0 on success, -1 on error.
extern VRPN_API int vrpn_flush_input_buffer( int comm );

// Throw out any characters (do not send) within the output buffer
// Return 0 on success, tc err codes (whatever those are) on error.
extern VRPN_API int vrpn_flush_output_buffer( int comm );

// Wait until all of the characters in the output buffer are sent, then
// return.  Return 0 on success, -1 on error.
extern VRPN_API int vrpn_drain_output_buffer( int comm );

// Read up the the requested count of characters from the input buffer,
// return with less if less (or none) are there.  Return the number of
// characters read, or -1 if there is an error.  The second of these
// will keep looking until the timeout period expires before returning
// (NULL pointer will cause it to block indefinitely).

extern VRPN_API int vrpn_read_available_characters(int comm, unsigned char *buffer,
		int count);
extern VRPN_API int vrpn_read_available_characters(int comm, unsigned char *buffer,
		int count, struct timeval *timeout);

extern VRPN_API int vrpn_write_characters(int comm, const unsigned char *buffer, int bytes); 
#endif
