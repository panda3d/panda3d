// Filename: clientTrackerDevice.h
// Created by:  drose (25Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLIENTTRACKERDEVICE_H
#define CLIENTTRACKERDEVICE_H

#include <pandabase.h>

#include "clientDevice.h"
#include "trackerData.h"

////////////////////////////////////////////////////////////////////
//       Class : ClientTrackerDevice
// Description : A device, attached to the ClientBase by a
//               TrackerNode, that records the data from a single
//               tracker device.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ClientTrackerDevice : public ClientDevice {
protected:
  INLINE ClientTrackerDevice(ClientBase *client, const string &device_name);

public:
  INLINE const TrackerData &get_data() const;

protected:
  TrackerData _data;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientDevice::init_type();
    register_type(_type_handle, "ClientTrackerDevice",
                  ClientDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "clientTrackerDevice.I"

#endif
