// vrpn_NationalInstruments.h
// Russell Taylor, May 2004
//

#ifndef VRPN_NATIONALINSTRUMENTS_H
#define VRPN_NATIONALINSTRUMENTS_H

#ifdef	VRPN_USE_NATIONAL_INSTRUMENTS_MX
#include <NIDAQmx.h>
#endif

#include "vrpn_Analog.h"
#include "vrpn_Analog_Output.h"

#define vrpn_NI_INPUT_MODE_DIFFERENTIAL (0)
#define vrpn_NI_INPUT_MODE_REF_SINGLE_ENDED (1)
#define vrpn_NI_INPUT_MODE_NON_REF_SINGLE_ENDED (2)
#define vrpn_NI_INPUT_RANGE_5V (5)
#define vrpn_NI_INPUT_RANGE_10V (10)
#define vrpn_NI_INPUT_RANGE_20V (20)

// An Analog and/or Analog_Output server that uses National Instruments cards
// to do its input and output.  It supercedes the vrpn_Analog_Output_Server_NI,
// which is now depracated.

class VRPN_API vrpn_National_Instruments_Server : public vrpn_Analog, public vrpn_Analog_Output {
public:
    vrpn_National_Instruments_Server(const char* name, vrpn_Connection* c, 
			     const char *boardName = "PCI-6713",
			     int numInChannels = vrpn_CHANNEL_MAX,
			     int numOutChannels = vrpn_CHANNEL_MAX,
                             double minInputReportDelaySecs = 0.0,
                             bool inBipolar = false,
                             int inputMode = vrpn_NI_INPUT_MODE_DIFFERENTIAL, // Input parameters (A/D)
                             int inputRange = vrpn_NI_INPUT_RANGE_10V,
                             bool driveAIS = false,
                             int inputGain = 1,
			     bool outBipolar = false,      // Output parameters (D/A)
			     double minOutVoltage = 0.0,
			     double maxOutVoltage = 10.0);
    virtual ~vrpn_National_Instruments_Server();

    virtual void mainloop();

protected:
  //  Addresses of the devices
#ifdef VRPN_USE_NATIONAL_INSTRUMENTS_MX
    TaskHandle    d_analog_task_handle;
    TaskHandle    d_analog_out_task_handle;
	bool setValues();	// Transfer our internal values to the D/A
    void reportError(int32 errnumber, vrpn_bool exitProgram = vrpn_false);
#else
    short   d_device_number;	      //< National Instruments device to use
#endif
    short   d_in_polarity;	      //< Polarity (1 = unipolar, 0 = bipolar)
    int     d_in_gain;                //< Input gain
    double  d_in_min_delay;           //< Minimum delay between two readings
    double  d_out_min_voltage;	      //< Minimum voltage allowed on a channel
    double  d_out_max_voltage;	      //< Maximum voltate allowed on a channel
    short   d_out_polarity;	      //< Polarity (1 = unipolar, 0 = bipolar)
    struct timeval d_last_report_time;//< When was the last analog tracker report sent?

    /// Sets the size of the array;  returns the size actually set.
    /// (May be clamped to vrpn_CHANNEL_MAX)
    /// This should be used before mainloop is ever called.
    int setNumInChannels (int sizeRequested);

    /// Sets the size of the array;  returns the size actually set.
    /// (May be clamped to vrpn_CHANNEL_MAX)
    /// This should be used before mainloop is ever called.
    int setNumOutChannels (int sizeRequested);

    /// Responds to a request to the AnalogOutput to change one of the values by
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

    virtual bool report_num_channels( vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE );
    virtual vrpn_int32 encode_num_channels_to( char* buf, vrpn_int32 num );
};

// An Analog output server that uses National Instruments cards to do its
// output.  It is superceded by vrpn_National_Instruments_Server, and is
// now deprecated.  It only works with the NIDAQ traditional.

class VRPN_API vrpn_Analog_Output_Server_NI : public vrpn_Analog_Output {
public:
    vrpn_Analog_Output_Server_NI(const char* name, vrpn_Connection* c, 
			     const char *boardName = "PCI-6713",
			     vrpn_int16 numChannels = vrpn_CHANNEL_MAX,
			     bool bipolar = false,
			     double minVoltage = 0.0,
			     double maxVoltage = 10.0);
    virtual ~vrpn_Analog_Output_Server_NI(void);

    virtual void mainloop();

protected:
    short   NI_device_number;	//< National Instruments device to use
    short   NI_num_channels;	//< Number of channels on the board
    double  min_voltage;	//< Minimum voltage allowed on a channel
    double  max_voltage;	//< Maximum voltate allowed on a channel
    short   polarity;		//< Polarity (1 = unipolar, 0 = bipolar)

    /// Sets the size of the array;  returns the size actually set.
    /// (May be clamped to vrpn_CHANNEL_MAX)
    /// This should be used before mainloop is ever called.
    vrpn_int32 setNumChannels (vrpn_int32 sizeRequested);

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

    virtual bool report_num_channels( vrpn_uint32 class_of_service = vrpn_CONNECTION_RELIABLE );
    virtual vrpn_int32 encode_num_channels_to( char* buf, vrpn_int32 num );
};

#endif
