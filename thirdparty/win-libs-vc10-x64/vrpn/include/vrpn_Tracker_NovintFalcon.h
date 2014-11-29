// -*- c++ -*-
// This file provides an interface to a Novint Falcon.
// http://home.novint.com/products/novint_falcon.php
// It uses libnifalcon to communicate with the device.
// http://libnifalcon.nonpolynomial.org/
//
// file:        vrpn_Tracker_NovintFalcon.h
// author:      Axel Kohlmeyer akohlmey@gmail.com 2010-04-14
// copyright:   (C) 2010 Axel Kohlmeyer
// license:     Released to the Public Domain.
// depends:     libnifalcon-1.0.1, libusb-1.0, VRPN 07_26
// tested on:   Linux x86_64 w/ gcc 4.4.1

#ifndef __TRACKER_NOVINTFALCON_H
#define __TRACKER_NOVINTFALCON_H

#include "vrpn_Configure.h"

#if defined(VRPN_USE_LIBNIFALCON)

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"
#include "vrpn_ForceDevice.h"

// Forward declaration for proxy class that wraps
// the device management of the falcon.
class vrpn_NovintFalcon_Device;

// Forward declaration for proxy class that maintains
// the list of objects that contribute to the force.
class vrpn_NovintFalcon_ForceObjects;

class VRPN_API vrpn_Tracker_NovintFalcon
    : public vrpn_Tracker, public vrpn_Button, public vrpn_ForceDevice {

public:
    /// custom constructor
    vrpn_Tracker_NovintFalcon(const char *name,
                              vrpn_Connection *c = NULL,
                              const int devidx = 0,
                              const char *grip = NULL,
                              const char *kine = NULL,
                              const char *damp = NULL);

    /// destructor
    ~vrpn_Tracker_NovintFalcon();

    /// Called once through each main loop iteration to handle updates.
    virtual void mainloop();

protected: // methods for tracker and button functionality
    virtual void reset();
    virtual int get_report(void);
    virtual void send_report(void);
    virtual void clear_values(void);

protected: // methods for force feedback functionality
    /// apply forces from known objects
    virtual void handle_forces(void);
public:
    /// apply received information about force field effects.
    virtual int  update_forcefield_effect(vrpn_HANDLERPARAM p);
protected:
    int m_devflags;                         //< device configuration flags
    vrpn_float64 m_update_rate;             //< update rate of device
    vrpn_float64 m_damp;                    //< damping factor for force updates
    struct timeval m_timestamp;             //< last update of device status
    vrpn_NovintFalcon_Device *m_dev;        //< device handle
    vrpn_NovintFalcon_ForceObjects *m_obj;  //< handle to force generating objects
};

#endif /* defined(VRPN_USE_LIBNIFALCON) */
#endif
