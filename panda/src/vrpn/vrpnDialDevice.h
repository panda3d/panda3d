/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnDialDevice.h
 * @author drose
 * @date 2001-01-26
 */

#ifndef VRPNDIALDEVICE_H
#define VRPNDIALDEVICE_H

#include "pandabase.h"

#include "clientDialDevice.h"

class VrpnClient;
class VrpnDial;

/**
 * The Panda interface to a VRPN dial device.  This object will be returned by
 * VrpnClient::make_device(), for attaching to a DialNode.
 *
 * This class does not need to be exported from the DLL.
 */
class VrpnDialDevice : public ClientDialDevice {
public:
  VrpnDialDevice(VrpnClient *client, const std::string &device_name,
                   VrpnDial *vrpn_dial);
  virtual ~VrpnDialDevice();

  INLINE VrpnDial *get_vrpn_dial() const;

private:
  VrpnDial *_vrpn_dial;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientDialDevice::init_type();
    register_type(_type_handle, "VrpnDialDevice",
                  ClientDialDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class VrpnDial;
};

#include "vrpnDialDevice.I"

#endif
