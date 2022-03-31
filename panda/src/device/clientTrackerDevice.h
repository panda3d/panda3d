/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientTrackerDevice.h
 * @author drose
 * @date 2001-01-25
 */

#ifndef CLIENTTRACKERDEVICE_H
#define CLIENTTRACKERDEVICE_H

#include "pandabase.h"

#include "clientDevice.h"
#include "trackerData.h"

/**
 * A device, attached to the ClientBase by a TrackerNode, that records the
 * data from a single tracker device.
 */
class EXPCL_PANDA_DEVICE ClientTrackerDevice : public ClientDevice {
protected:
  INLINE ClientTrackerDevice(ClientBase *client, const std::string &device_name);

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
