// Filename: vrpnButtonDevice.h
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef VRPNBUTTONDEVICE_H
#define VRPNBUTTONDEVICE_H

#include <pandabase.h>

#include <clientButtonDevice.h>

class VrpnClient;
class VrpnButton;

////////////////////////////////////////////////////////////////////
//       Class : VrpnButtonDevice
// Description : The Panda interface to a VRPN button.  This object
//               will be returned by VrpnClient::make_device(), for
//               attaching to a ButtonNode.
//
//               This class does not need to be exported from the DLL.
////////////////////////////////////////////////////////////////////
class VrpnButtonDevice : public ClientButtonDevice {
public:
  VrpnButtonDevice(VrpnClient *client, const string &device_name,
		   VrpnButton *vrpn_button);
  virtual ~VrpnButtonDevice();

  INLINE VrpnButton *get_vrpn_button() const;

private:
  VrpnButton *_vrpn_button;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientButtonDevice::init_type();
    register_type(_type_handle, "VrpnButtonDevice",
                  ClientButtonDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

  friend class VrpnButton;
};

#include "vrpnButtonDevice.I"

#endif
