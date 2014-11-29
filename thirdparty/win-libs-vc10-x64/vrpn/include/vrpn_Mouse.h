#ifndef VRPN_MOUSE_H
#define VRPN_MOUSE_H

///////////////////////////////////////////////////////////////////////////
// This file contains a distillation of the various Mouse classes that had
// been spread throughout VRPN.  The interfaces have been rationalized, so
// that they are the same between operating systems and are factored into
// independent interfaces.
///////////////////////////////////////////////////////////////////////////

/* file:	vrpn_Mouse.h
 * author:	Mike Weiblen mew@mew.cx 2004-01-14
 * copyright:	(C) 2003,2004 Michael Weiblen
 * license:	Released to the Public Domain.
 * depends:	gpm 1.19.6, VRPN 06_04
 * tested on:	Linux w/ gcc 2.95.4
 * references:  http://mew.cx/ http://vrpn.org/
 *              http://linux.schottelius.org/gpm/
*/

///////////////////////////////////////////////////////////////////////////
// vrpn_Mouse is a VRPN server class to publish events from the PC's mouse.
// It provides a 2-channel vrpn_Analog for X & Y mouse motion, and a
// 3-channel vrpn_Button for the mouse buttons.
//
// vrpn_Mouse makes it easy to use the diverse array of commodity input
// devices that masquerade as a mouse, such as PS/2 trackballs, gyroscopic
// free-space pointers, and force-sensing touchpads.
//
// This version includes a Linux-specific implementation that leverages the Linux GPM
// (General Purpose Mouse) server to handle the low-level hardware interfaces
// and device protocols.  GPM is commonly included in Linux distributions.
// The GPM homepage is http://linux.schottelius.org/gpm/
//
// It also includes a Windows interface to the mouse.
//
// The interface reports mouse position in fraction of the screen.
// The previous version of the Windows implementation had reported them
// in pixels, but this has been changed to match on both platforms.
// 
// vrpn_Mouse must be run on a Linux console, not an xterm.  Rationale:
// 1) Since the console environment doesn't presume the existance of a mouse,
//    it avoids issues about mapping mouse events to window focus, etc.
// 2) With the mouse movement controlled by a different user, it's really
//    not possible to also use a mouse-based user interface anyway.
// 3) My VRPN server machine is headless, and doesn't even have an X server.
///////////////////////////////////////////////////////////////////////////

#include "vrpn_Analog.h"
#include "vrpn_Button.h"

class VRPN_API vrpn_Mouse :
	public vrpn_Analog,
	public vrpn_Button_Filter
{
public:
    vrpn_Mouse( const char* name, vrpn_Connection* cxn );
    virtual ~vrpn_Mouse();

    virtual void mainloop();

    class GpmOpenFailure {};    // thrown when cant open GPM server

protected:  // methods
    /// Try to read reports from the device.
    /// Returns 1 if msg received, or 0 if none received.
    virtual int get_report();

    /// send report iff changed
    virtual void report_changes( vrpn_uint32 class_of_service
		    = vrpn_CONNECTION_LOW_LATENCY );

    /// send report whether or not changed
    virtual void report( vrpn_uint32 class_of_service
		    = vrpn_CONNECTION_LOW_LATENCY );

protected:  // data
    struct timeval timestamp;	///< time of last report from device

private:  // disable unwanted default methods
    vrpn_Mouse();
    vrpn_Mouse(const vrpn_Mouse&);
    const vrpn_Mouse& operator=(const vrpn_Mouse&);
};

///////////////////////////////////////////////////////////////////////////
// (RDK) Button device that is connected to a serial port.  This is a
// raw driver that can be used with any of the serial-enabled architectures,
// and does not go through the mouse driver -- it talks to the mouse
// directly as a serial device.  This enables the user to use a mouse, or
// a rewired serial mouse, as a simple button device.

enum vrpn_MOUSETYPE {MOUSESYSTEMS, THREEBUTTON_EMULATION, MAX_MOUSE_TYPES};

class VRPN_API vrpn_Button_SerialMouse: public vrpn_Button_Filter {
public:
    // Open a serial mouse button device connected to the local machine
    
    vrpn_Button_SerialMouse(const char *name, vrpn_Connection *connection,
			    const char *port, int baud, vrpn_MOUSETYPE type);
     
    virtual void mainloop(void);

protected:
    char    portname[512];
    int     baudrate;
    int     serial_fd;
    int	    status;
    int     lastL, lastM, lastR;  //used in 3 button emulator
    vrpn_MOUSETYPE mousetype;

    void read(void);
};

#endif
