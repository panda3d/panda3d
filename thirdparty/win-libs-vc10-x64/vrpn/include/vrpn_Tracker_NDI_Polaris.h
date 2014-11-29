
//TODO

//---Do I really need a reset? If we add one, we'll need to either store 
//the filenames of each .rom file, or store the contents of
//the files themselves. The NDI Polaris needs theses files to be uploaded
//to the tracker, over the serial port, each time we connect to it.

//---do update_rate affect only the rate at which tracker reports are 
//generated, or also the rate at which they are sent?

//---read frame # from tracker and avoid sending duplicate reports
//(if this runs at faster than 60Hz)

//---test on other platforms (only tested on WinXP so far)

//---add support for 



// This server works with the NDI Polaris Spectra and Polaris Vectra
// trackers.  It doesn't work with the Optitrak or Aurora.  It only
// handles rigid bodies made from passive sphere markers, and does
// not yet support active led markers nor single "stray" passive 
// spheres.

// Each vrpn "sensor" is a single rigid body (or "tool" in NDI
// terminology), which consists of 3 or more passive spheres in a 
// particular geometric arrangement. In order to define a custom
// rigid body (one that didn't come from NDI), you must use the NDI
// Architect software. That software produces .rom files for each 
// rigid body. This vrpn tracker class will load those files during 
// initialization.

// Before configuring the vrpn server here, you should first run
// the NDI software to track the rigid bodies using the NDI's 
// interactive GUI. Only after you have configured and tested the
// tracker and rigid bodies to your satisfaction, should you try
// this vrpn server. 

// This currently only handles an IR strobe rate of 60Hz, not 
// 20Hz or 30Hz.


// The NDI Polaris Vectra and Spectra communicates with the host over
// the serial port, but this VRPN Tracker isn't a sub-class of 
// vrpn_Tracker_Serial.  Perhaps a future version of this class should 
// be.  Here are some potential differences between this Tracker class
// and the other serial port trackers:
//
// 1) The host must connect at a fixed 9600 baud rate at first, but
// then switches to a faster rate. This should happen each time the
// tracker server initializes, and after each reset. And the com port
// isn't really a com port - it's a virtual com port that runs over 
// NDI's own USB device.  When we tell the host to connect at 19200 baud 
// (on Windows), it really runs at 1.2Mbps.
//
// 2) While other trackers consciously send tracker report to the
// host, the NDI Polaris trackers do not.  Instead, they wait for
// a command from the host, then immediately (< 17 ms later) send a 
// single reply back.  The host asks for a single trackerport at a
// time (using the "TX" command).  Because of the 1.2Mbps connection
// speed, it seems reasonable (to me) to simply block and wait for the
// response, rather than return control back to the caller and then 
// processing the response later.

#ifndef VRPN_TRACKER_NDI_POLARIS_H
#define VRPN_TRACKER_NDI_POLARIS_H

#include "vrpn_Shared.h"
#include "vrpn_Tracker.h"

class VRPN_API vrpn_Tracker_NDI_Polaris : public vrpn_Tracker {

public:
  /// The constructor is given the name of the tracker (the name of
  /// the sender it should use), the connection on which it is to
  /// send its messages, the name of the serial port it is to open.
  /// The final two parameters are the number of rigid bodies to track,
  /// and a pointer to an array of strings containing the filenames of the
  /// the .rom file for each rigid body.

  vrpn_Tracker_NDI_Polaris(const char *name, 
                          vrpn_Connection *c,
						  const char *port,
                          int numOfRigidBodies,
                          const char** rigidBodyNDIRomFileNames);

  ~vrpn_Tracker_NDI_Polaris();

  virtual void mainloop();

protected:

	// constants, that are enums instead of const ints, to allow VC6 compatibility
	// Other vrpn objects appear to put the globals in the global scope?
	enum { NDI_ROMFILE_CHUNK_SIZE=64};
	enum { MAX_NDI_ROM_FILE_SIZE_IN_BYTES=1024};
    enum { MAX_NDI_RESPONSE_LENGTH=300} ; //FIXME look up what the longest response the NDI tracker will send back
	enum { VRPN_MSGBUFSIZE=1024};
	

int serialFd; //the fid for the serial port
int numOfRigidBodies;
unsigned char* latestResponseStr;


protected:
  // FIXME - do I need a reset() method?
  virtual int get_report(void);
  virtual void send_report(void);

// Send a command to the NDI tracker over the serial port.
// This assumes the serial port has already been opened.
// Some NDI commands require a white-space char at the end of the command, the
// call must be sure to add it.
// NDI commands end with a Carriage-return (\r) char. This function automatically adds it, and
// flushes the output buffer.
// commandString MUST be terminated with \0, since we don't know the string
// length otherwise.
// The NDI trackers have two different command syntaxes - this only supports the syntax
// WITHOUT CRC checksums

	void sendCommand(const char* commandString );

// Read a fully formed responses from the NDI tracker, (including the 4-byte CRC at the end)
// and copies it to latestResponseString.
// Returns the number of characters in the response (not including the CR),
// or -1 in case of an error
// NDI responses are all terminated with a CR, which this function replace with a end-of-string char.
//
// This function blocks until the CR has been received.
// FIXME: add a timeout parameter, and timeout if it's been too long
	int readResponse();

// Given a filename of a binary .rom file, this reads the file and returns
// a string with the contents of the file as ascii encoded hex: For each byte of
// the file, this returns two ascii characters, each of which are ascii representation
// of a HEX digit (0-9 & a-f). Hex letters are returned as lower case.
// The string is padded with zeros to make it's length a multiple of 128
// characters( which is 64 bytes of the original binary file).
// asciiEncodedHexStr must be allocated before calling, be
// MAX_NDI_FROM_FILE_SIZE_IN_BYTES * 2 characters long.
//
// RETURNS the number of bytes represented in the string (which is half the number of ASCII characters)
// , or -1 on failure
	int convertBinaryFileToAsciiEncodedHex(const char* filename, char *asciiEncodedHexStr);

// NDI response strings often encode ints as a two ascii's WITHOUT any separator behind it
// this returns the value as an int.
// The caller passes in the string, and a pointer to an (int) index into that string (which will be advanced
// to the end of the value we just parsed.
// The caller must make sure the string is at least two characters long
	unsigned int parse2CharIntFromNDIResponse(unsigned char* str, int* strIndexPtr=NULL);

// NDI TX response strings often encode floats as a size ascii's WITHOUT any separator behind it
// this returns the value as an float. The last 4 digits are implicitly to the right of the decimal point
// (the decimal point itself is not in the string)
// The caller passes in the string, and a pointer to an (int) index into that string (which will be advanced
// to the end of the value we just parsed.
// The caller must make sure the string is at least six characters long
	float parse6CharFloatFromNDIResponse(unsigned char* str,  int* strIndexPtr);

// NDI TX response strings often encode floats as a size ascii's WITHOUT any separator behind it
// this returns the value as an float. The last 2 digits are implicitly to the right of the decimal point
// (the decimal point itself is not in the string)
// The caller passes in the string, and a pointer to an (int) index into that string (which will be advanced
// to the end of the value we just parsed.
// The caller must make sure the string is at least seven characters long
	float parse7CharFloatFromNDIResponse(unsigned char* str,  int* strIndexPtr);

	int setupOneTool(const char* NDIToolRomFilename);

	void switchToHigherBaudRate(const char* port);

};

#endif
