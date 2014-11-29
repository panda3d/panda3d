// vrpn_Button_NI_DIO24.h
//
//      This is a driver for National Instruments DAQCard
// DIO-24, a PCMCIA card, which provides 24-bit digital I/O.  
// The I/O is accessed in 3 "ports" with 8 bits per port,
// though the user is protected from that detail.  The
// user of this class need only request inputs 1 thru 24.
//
// Unlike the other National Instrument devices currently
// in vrpn, this uses their new "mx" library.  To access
// that library, install their software from the NI-DAQmx
// CD.  Then uncomment the following line in vrpn_configure.h:
//   #define VRPN_USE_NATIONAL_INSTRUMENTS_MX
//
// Note that because the 3rd party library is used, this class
// will only work under Windows.
//
// You must also include the following in your compilers include
// path for the 'vrpn' and 'vrpn_server' projects:
//   $(SYSTEMDRIVE)\Program Files\National Instruments\NI-DAQ\DAQmx ANSI C DEV\include
//
// Finally, the following must be included in vrpn.cfg to use
// the generic server:
//   
//   ################################################################################
//   #      This is a driver for National Instruments DAQCard-
//   # DIO-24, a PCMCIA card, which provides 24-bit digital I/O.
//   #
//   # Arguments:
//   #       char    name_of_this_device[]
//   #       int     number_of_channls to read: 1-24 (optional.  default=24)
//   
//   vrpn_Button_NI_DIO24    Button0 1
//
// This code was written in October 2006 by Bill West, based on some example
// code provided by National Instruments.

#ifndef VRPN_BUTTON_NI_DIO24_H
#define VRPN_BUTTON_NI_DIO24_H

#include "vrpn_Button.h"
#ifdef VRPN_USE_NATIONAL_INSTRUMENTS_MX
#include <NIDAQmx.h>
#endif

class VRPN_API vrpn_Button_NI_DIO24 : public vrpn_Button_Filter {

  public:

    //  Public constant used by this class
    static const vrpn_int32    vrpn_Button_NI_DIO24_CHANNEL_MAX ;

    //  Constructor
    vrpn_Button_NI_DIO24 (const char * name, vrpn_Connection * c,
                          vrpn_int32 numChannels=vrpn_Button_NI_DIO24_CHANNEL_MAX) ;

    //  Destructor
    virtual ~vrpn_Button_NI_DIO24() ;

    //  Here's where the buttons are actually read
    virtual void mainloop () ;

  private:
  //  Addresses of the devices
#ifdef VRPN_USE_NATIONAL_INSTRUMENTS_MX
    TaskHandle    _taskHandle ;
#endif

    /// Sets the number of channels and ports, clamped to maximums if needed.
    /// This should be used before mainloop is ever called.
    vrpn_int32 setNumChannels (vrpn_int32 sizeRequested);

    //  THis handles error reporting, and halts the
    //  program if the error is irrecoverable
#ifdef VRPN_USE_NATIONAL_INSTRUMENTS_MX
    void reportError(int32 errnumber, vrpn_bool exitProgram) ;
#endif // def(_WIN32) || def(WIN32)
};

#endif    //  VRPN_BUTTON_NI_DIO24_H
