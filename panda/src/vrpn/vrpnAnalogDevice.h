// Filename: vrpnAnalogDevice.h
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VRPNANALOGDEVICE_H
#define VRPNANALOGDEVICE_H

#include <pandabase.h>

#include <clientAnalogDevice.h>

class VrpnClient;
class VrpnAnalog;

////////////////////////////////////////////////////////////////////
//       Class : VrpnAnalogDevice
// Description : The Panda interface to a VRPN analog device.  This
//               object will be returned by VrpnClient::make_device(),
//               for attaching to a AnalogNode.
//
//               This class does not need to be exported from the DLL.
////////////////////////////////////////////////////////////////////
class VrpnAnalogDevice : public ClientAnalogDevice {
public:
  VrpnAnalogDevice(VrpnClient *client, const string &device_name,
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
