// Filename: vrpnDialDevice.h
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VRPNDIALDEVICE_H
#define VRPNDIALDEVICE_H

#include <pandabase.h>

#include <clientDialDevice.h>

class VrpnClient;
class VrpnDial;

////////////////////////////////////////////////////////////////////
//       Class : VrpnDialDevice
// Description : The Panda interface to a VRPN dial device.  This
//               object will be returned by VrpnClient::make_device(),
//               for attaching to a DialNode.
//
//               This class does not need to be exported from the DLL.
////////////////////////////////////////////////////////////////////
class VrpnDialDevice : public ClientDialDevice {
public:
  VrpnDialDevice(VrpnClient *client, const string &device_name,
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
