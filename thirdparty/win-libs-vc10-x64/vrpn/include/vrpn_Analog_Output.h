// vrpn_Analog_Output.h
// David Borland, September 2002
//
// These classes are for setting values for an analog output device.  The vrpn_Analog was
// getting overloaded by trying to have functionality for both reading and writing in it.
// If wanting to read analog values from a device, a vrpn_Analog should be used, if 
// wanting ro write analog values to a device, a vrpn_Analog_Output should be used.  
// This is similar to the Tracker/Poser dichotomy.

#ifndef VRPN_ANALOG_OUTPUT_H
#define VRPN_ANALOG_OUTPUT_H

#include "vrpn_Analog.h"    // just for some #define's and constants

// Similar to vrpn_Analog, but messages are different
// Members beginning with o_ are also found in vrpn_Analog, the o_ is 
// so that you can derive a class from both without getting ambiguities
class VRPN_API vrpn_Analog_Output : public vrpn_BaseClass {
public:
     vrpn_Analog_Output(const char* name, vrpn_Connection* c = NULL);
     
     // Print the status of the analog output device
     void o_print(void);
     
     vrpn_int32 getNumChannels( ) const
     {  return o_num_channel;  }
     
protected:
     vrpn_float64   o_channel[vrpn_CHANNEL_MAX];
     vrpn_int32	    o_num_channel;
     struct timeval o_timestamp;
     vrpn_int32	    request_m_id;	      //< Request to change message from client
     vrpn_int32     request_channels_m_id;    //< Request to change channels message from client
     vrpn_int32     report_num_channels_m_id; //< Report of the number of active channels, from the server
     vrpn_int32	    got_connection_m_id;      //< new-connection notification
     int            o_status; 
     
     virtual	int register_types(void);
};


// A *Sample* Analog output server.  Use this, or derive your own server
// from vrpn_Analog_Output with this as a guide.  You can remove the
// user-level callback code (both the type before this class and the
// list and the handler register/deregister) if the server is controlling
// a device directly.

class VRPN_API vrpn_Analog_Output_Server : public vrpn_Analog_Output {
public:
    vrpn_Analog_Output_Server(const char* name, vrpn_Connection* c, 
                             vrpn_int32 numChannels = vrpn_CHANNEL_MAX );
    virtual ~vrpn_Analog_Output_Server(void);

    virtual void mainloop() { server_mainloop(); }

    /// Sets the size of the array;  returns the size actually set.
    /// (May be clamped to vrpn_CHANNEL_MAX)
    /// This should be used before mainloop is ever called.
    vrpn_int32 setNumChannels (vrpn_int32 sizeRequested);

    /// Exposes an array of values for the user to read from.
    const vrpn_float64* o_channels (void) const { return o_channel; };

protected:
    virtual bool report_num_channels( vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE );
    virtual vrpn_int32 encode_num_channels_to( char* buf, vrpn_int32 num );

    /// Responds to a request to change one of the values by
    /// setting the channel to that value.  Derived class must
    /// either install handlers for this routine or else make
    /// its own routines to handle the request message.
    static int VRPN_CALLBACK handle_request_message( void *userdata,
                                      vrpn_HANDLERPARAM p );

    /// Responds to a request to change a number of channels
    /// Derived class must either install handlers for this
    /// routine or else make its own routines to handle the
    /// multi-channel request message.
    static int VRPN_CALLBACK handle_request_channels_message( void* userdata,
                                               vrpn_HANDLERPARAM p);

    /// Used to notify us when a new connection is requested, so that
    /// we can let the client know how many channels are active
    static int VRPN_CALLBACK handle_got_connection( void* userdata, vrpn_HANDLERPARAM p );
};


// A more complicated analog server that provides a
// user routine to handle a change in analog values.  This is called when
// the analog callback is called (when a message from its counterpart
// across the connection arrives).  This callback is called whenever
// EITHER type of change message arrives (either a single-channel change
// or a multiple-channel change.

typedef	struct _vrpn_ANALOGOUTPUTCB {
	struct timeval	msg_time;	// Timestamp of analog data
	vrpn_int32	num_channel;    // how many channels
	const vrpn_float64  *channel;   // analog values (pointer to channels)
} vrpn_ANALOGOUTPUTCB;

typedef void (VRPN_CALLBACK *vrpn_ANALOGOUTPUTCHANGEHANDLER) (void * userdata,
					  const vrpn_ANALOGOUTPUTCB info);

class VRPN_API vrpn_Analog_Output_Callback_Server : public vrpn_Analog_Output_Server {
public:
    vrpn_Analog_Output_Callback_Server(const char* name, vrpn_Connection* c, 
                             vrpn_int32 numChannels = vrpn_CHANNEL_MAX );
    virtual ~vrpn_Analog_Output_Callback_Server(void);

    // (un)Register a callback handler to handle analog value change.
    // These will be called whenver EITHER type of change message is
    // received, either a single channel or multiple channels.  This is
    // useful for applications that "have a" server, rather than derive
    // from the server.
    virtual int register_change_handler(void *userdata,
	    vrpn_ANALOGOUTPUTCHANGEHANDLER handler) {
      return d_callback_list.register_handler(userdata, handler);
    };
    virtual int unregister_change_handler(void *userdata,
	    vrpn_ANALOGOUTPUTCHANGEHANDLER handler) {
      return d_callback_list.unregister_handler(userdata, handler);
    }

protected:
    /// Handles BOTH types of changes messages, and will be called
    /// after the vrpn_Analog_Output_Server class has already filled
    /// in the values.  It just calls the user callbacks with the
    /// appropriate pointer to the data values.
    static int VRPN_CALLBACK handle_change_message( void *userdata,
                                      vrpn_HANDLERPARAM p );

    /// List of user-level routines that need to be called back to let
    /// them know that the values have changed.
    vrpn_Callback_List<vrpn_ANALOGOUTPUTCB> d_callback_list;
};


// Open an analog output device that is on the other end of a connection
// and send updates to it.  This is the type of analog output device
// that user code will deal with.
class VRPN_API vrpn_Analog_Output_Remote : public vrpn_Analog_Output {
public:
     // The name of the analog device to connect to
     // Optional argument to be used when the Remote should listen on
     // a connection that is already open.
     vrpn_Analog_Output_Remote(const char* name, vrpn_Connection* c = NULL);
     virtual ~vrpn_Analog_Output_Remote (void);
     
     // This routine calls the mainloop of the connection it's on
     virtual void mainloop();

     // Request the analog to change its value to the one specified.
     // Returns false on failure.
     virtual bool request_change_channel_value(unsigned int chan, vrpn_float64 val,
                                               vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE);
     
     // Request the analog to change values all at once.  If more values are given 
     // than we have channels, the extra values are discarded.  If less values are 
     // given than we have channels, the extra channels are set to 0.
     // Returns false on failure
     virtual bool request_change_channels(int num, vrpn_float64* vals,
                                          vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE);
     
protected:
     // How we hear about the number of active channels
     static int VRPN_CALLBACK handle_report_num_channels( void *userdata, vrpn_HANDLERPARAM p );
     
     // Routines used to send requests from the client
     virtual vrpn_int32 encode_change_to(char *buf, vrpn_int32 chan, vrpn_float64 val);
     virtual vrpn_int32 encode_change_channels_to(char* buf, vrpn_int32 num, vrpn_float64* vals);
};

#endif
