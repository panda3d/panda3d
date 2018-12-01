/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnAnalogDevice.h
 * @author drose
 * @date 2001-01-26
 */

#ifndef VRPNANALOGDEVICE_H
#define VRPNANALOGDEVICE_H

#include "pandabase.h"

#include "clientAnalogDevice.h"

class VrpnClient;
class VrpnAnalog;

/**
 * The Panda interface to a VRPN analog device.  This object will be returned
 * by VrpnClient::make_device(), for attaching to a AnalogNode.
 *
 * This class does not need to be exported from the DLL.
 */
class VrpnAnalogDevice : public ClientAnalogDevice {
public:
  VrpnAnalogDevice(VrpnClient *client, const std::string &device_name,
                   VrpnAnalog *vrpn_analog);
  virtual ~VrpnAnalogDevice();

  INLINE VrpnAnalog *get_vrpn_analog() const;

private:
  VrpnAnalog *_vrpn_analog;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientAnalogDevice::init_type();
    register_type(_type_handle, "VrpnAnalogDevice",
                  ClientAnalogDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class VrpnAnalog;
};

#include "vrpnAnalogDevice.I"

#endif
