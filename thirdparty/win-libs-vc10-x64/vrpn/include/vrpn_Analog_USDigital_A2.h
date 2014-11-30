// vrpn_Analog_USDigital_A2.C
//
//      This is a driver for USDigital A2 Absolute Encoders.
// They can be daisy changed together, and utlimately, one or
// more plug into a serial port and communicate using RS-232.
// You can find out more at www.usdigital.com.
//
// To use this class, install the US Digital software, specifying
// the "SEI Explorer Demo Software" to install.
//
// Then uncomment the following line in vrpn_configure.h:
//   #define VRPN_USE_USDIGITAL
//
// Note that because the 3rd party library is used, this class
// will only work under Windows.
//
// You must also include the following in your compilers include
// path for the 'vrpn' project:
//   $(SYSTEMDRIVE)\Program Files\SEI Explorer
//
// Finally, the following must be included in vrpn.cfg to use
// the generic server:
//
//   ################################################################################
//   # US Digital A2 Absolute Encoder Analog Input server.  This will open the COM
//   # port specified, configure the number of channels specified, and report
//   # Absolute Encoder values in tenths of a degree from 0 to 3599.
//   #
//   # Arguments:
//   #       char    name_of_this_device[]
//   #       int     COM_port.  If 0, search for correct COM port.
//   #       int     number_of_channels
//   #       int     0 to report always, 1 to report on change only (optional, default=0)
//   
//   vrpn_Analog_USDigital_A2        Analog0 0       2
//
// This code was written in October 2006 by Bill West, who
// used the vrpn_Analog_Server sample code written by 
// Tom Hudson in March 1999 as a starting point.  Bill also
// used some ideas from vrpn_Radamec_SPI.[Ch] written by
// Russ Taylor in August 2000.
#ifndef VRPN_ANALOG_USDIGITAL_A2_H
#define VRPN_ANALOG_USDIGITAL_A2_H

#include "vrpn_Analog.h"

class VRPN_API vrpn_Analog_USDigital_A2 : public vrpn_Analog {

  public:

    //  Constants used by this class
    static const vrpn_uint32    vrpn_Analog_USDigital_A2_CHANNEL_MAX ;
    static const vrpn_uint32    vrpn_Analog_USDigital_A2_FIND_PORT ;

    //  Constructor
    vrpn_Analog_USDigital_A2 (const char * name, vrpn_Connection * c,
                              vrpn_uint32 portNum=vrpn_Analog_USDigital_A2_FIND_PORT,
                              vrpn_uint32 numChannels=vrpn_Analog_USDigital_A2_CHANNEL_MAX,
                              vrpn_int32 reportOnChangeOnly=0) ;

    //  Destructor
    virtual ~vrpn_Analog_USDigital_A2() ;

    //  Here's where the encoders are actually read
    virtual void mainloop () ;

  private:
    //  Maintains whether the SEI bus has been opened or not
    vrpn_bool    _SEIopened ;

    //  Addresses of the devices
    long        *_devAddr ;

    //  Whether to report() or report_change() ;
    vrpn_bool    _reportChange ;

    //  Actual number of devices (as opposed to channels)
    vrpn_uint32  _numDevices ;

    /// Exposes an array of values for the user to write into.
    vrpn_float64* channels (void) { return channel; }

    /// Sets the size of the array;  returns the size actually set.
    /// (May be clamped to vrpn_CHANNEL_MAX)   
    /// This should be used before mainloop is ever called.
    vrpn_int32 setNumChannels (vrpn_int32 sizeRequested);
};

#endif    //  VRPN_ANALOG_USDIGITAL_A2_H
