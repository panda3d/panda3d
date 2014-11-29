#ifndef VRPN_ANALOG_H
#define VRPN_ANALOG_H

#include "vrpn_Connection.h"
#include "vrpn_BaseClass.h"

#ifndef	VRPN_CLIENT_ONLY
#include "vrpn_Serial.h"
#endif

#define vrpn_CHANNEL_MAX 128

// analog status flags
const	int vrpn_ANALOG_SYNCING		= (2);
const	int vrpn_ANALOG_REPORT_READY 	= (1);
const	int vrpn_ANALOG_PARTIAL 	= (0);
const	int vrpn_ANALOG_RESETTING	= (-1);
const	int vrpn_ANALOG_FAIL 	 	= (-2);

// Analog time value meaning "go find out what time it is right now"
const struct timeval vrpn_ANALOG_NOW = { 0 , 0 };

class VRPN_API vrpn_Analog : public vrpn_BaseClass {
public:
	vrpn_Analog (const char * name, vrpn_Connection * c = NULL);

	// Print the status of the analog device
	void print(void);
	
	vrpn_int32 getNumChannels(void) const;

  protected:
	vrpn_float64	channel[vrpn_CHANNEL_MAX];
	vrpn_float64	last[vrpn_CHANNEL_MAX];
	vrpn_int32	num_channel;
	struct timeval	timestamp;
	vrpn_int32	channel_m_id;	        //< channel message id (message from server)
	int status; 

	virtual	int register_types(void);

	//------------------------------------------------------------------
	// Routines used to send data from the server
	virtual vrpn_int32 encode_to(char *buf);
	/// Send a report only if something has changed (for servers)
	/// Optionally, tell what time to stamp the value with
	virtual void report_changes (vrpn_uint32 class_of_service
                                   = vrpn_CONNECTION_LOW_LATENCY, const struct timeval time = vrpn_ANALOG_NOW);
	/// Send a report whether something has changed or not (for servers)
	/// Optionally, tell what time to stamp the value with
	virtual void report (vrpn_uint32 class_of_service
                           = vrpn_CONNECTION_LOW_LATENCY, const struct timeval time = vrpn_ANALOG_NOW);
};

#ifndef	VRPN_CLIENT_ONLY
class VRPN_API vrpn_Serial_Analog: public vrpn_Analog {
public:
  vrpn_Serial_Analog(const char * name, vrpn_Connection * connection,
                     const char * port, int baud = 9600, int bits = 8, 
                     vrpn_SER_PARITY parity = vrpn_SER_PARITY_NONE);
  ~vrpn_Serial_Analog();
protected:
  int serial_fd;
  char portname[1024];
  int baudrate;
  unsigned char buffer[1024];
  int bufcounter;

  int read_available_characters(char *buffer, int bytes);
};
#endif

// vrpn_Analog_Server
// Tom Hudson, March 1999
//
// A *Sample* Analog server.  Use this or derive your own from vrpn_Analog with this
// as a guide.
//
// Write whatever values you want into channels(), then call report()
// or report_changes().  (Original spec only called for report_changes(),
// but vrpn_Analog's assumption that "no new data = same data" doesn't
// match the BLT stripchart assumption  of "no intervening data = ramp".
//
// For a sample application, see server_src/sample_analog.C


class VRPN_API vrpn_Analog_Server : public vrpn_Analog {

  public:

    vrpn_Analog_Server (const char * name, vrpn_Connection * c, vrpn_int32 numChannels = vrpn_CHANNEL_MAX );

    /// Makes public the protected base class function
    virtual void report_changes (vrpn_uint32 class_of_service
                                 = vrpn_CONNECTION_LOW_LATENCY,
				 const struct timeval time = vrpn_ANALOG_NOW);

    /// Makes public the protected base class function
    virtual void report (vrpn_uint32 class_of_service
                                 = vrpn_CONNECTION_LOW_LATENCY,
				 const struct timeval time = vrpn_ANALOG_NOW);

    /// For this server, the user must normally call report() or
    /// report_changes() directly.  This mainloop() only takes
    /// care of the things any server object should do.
    virtual void mainloop () { server_mainloop(); };

    /// Exposes an array of values for the user to write into.
    vrpn_float64* channels (void) { return channel; }

    /// Sets the size of the array;  returns the size actually set.
    /// (May be clamped to vrpn_CHANNEL_MAX)   
    /// This should be used before mainloop is ever called.
    vrpn_int32 setNumChannels (vrpn_int32 sizeRequested);
};

/// Analog server that can scale and clip its range to -1..1.
// This is useful for joysticks, to allow them to be centered and
// scaled to cover the whole range.  Rather than writing directly
// into the channels array, call the setChannel() method.

class VRPN_API vrpn_Clipping_Analog_Server : public vrpn_Analog_Server {
  public:
    vrpn_Clipping_Analog_Server(const char *name, vrpn_Connection *c, vrpn_int32 numChannels = vrpn_CHANNEL_MAX );

    /// Set the clipping values for the specified channel.
    /// min maps to -1, values between lowzero and highzero map to 0,
    /// max maps to 1.  Values less than min map to -1, values larger
    /// than max map to 1. Default for each channel is -1,0,0,1
    /// It is possible to compress the range to [0..1] by setting the
    /// minimum equal to the lowzero.
    /// Returns 0 on success, -1 on failure.
    int	setClipValues(int channel, double min, double lowzero, double
	    highzero, double max);

    /// This method should be used to set the value of a channel.
    /// It will be scaled and clipped as described in setClipValues.
    /// It returns 0 on success and -1 on failure.
    int	setChannelValue(int channel, double value);

  protected:
      typedef	struct {
	      double    minimum_val;    // Value mapped to -1
	      double    lower_zero;	    // Minimum value mapped to 0
	      double    upper_zero;	    // Maximum value mapped to 0
	      double    maximum_val;    // Value mapped to 1
      } clipvals_struct;

      clipvals_struct	clipvals[vrpn_CHANNEL_MAX];
};

//----------------------------------------------------------
//************** Users deal with the following *************

// User routine to handle a change in analog values.  This is called when
// the analog callback is called (when a message from its counterpart
// across the connection arrives).


typedef	struct _vrpn_ANALOGCB {
	struct timeval	msg_time;	// Timestamp of analog data
	vrpn_int32	num_channel;    // how many channels
	vrpn_float64	channel[vrpn_CHANNEL_MAX];  // analog values
} vrpn_ANALOGCB;

typedef void (VRPN_CALLBACK *vrpn_ANALOGCHANGEHANDLER) (void * userdata,
					  const vrpn_ANALOGCB info);

// Open an analog device that is on the other end of a connection
// and handle updates from it.  This is the type of analog device
// that user code will deal with.

class VRPN_API vrpn_Analog_Remote: public vrpn_Analog {
    public:
        // The name of the analog device to connect to
        // Optional argument to be used when the Remote should listen on
        // a connection that is already open.
        vrpn_Analog_Remote (const char * name, vrpn_Connection * c = NULL );

        // This routine calls the mainloop of the connection it's on
        virtual void mainloop();

        // (un)Register a callback handler to handle analog value change
        virtual int register_change_handler(void *userdata,
		vrpn_ANALOGCHANGEHANDLER handler) {
	  return d_callback_list.register_handler(userdata, handler);
	};
        virtual int unregister_change_handler(void *userdata,
		vrpn_ANALOGCHANGEHANDLER handler) {
	  return d_callback_list.unregister_handler(userdata, handler);
	}

    protected:
        vrpn_Callback_List<vrpn_ANALOGCB> d_callback_list;

        static int VRPN_CALLBACK handle_change_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif
