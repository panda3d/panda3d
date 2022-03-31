/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnTrackerDevice.h
 * @author drose
 * @date 2001-01-25
 */

#ifndef VRPNTRACKERDEVICE_H
#define VRPNTRACKERDEVICE_H

#include "pandabase.h"

#include "clientTrackerDevice.h"

class VrpnClient;
class VrpnTracker;

/**
 * The Panda interface to a VRPN tracker.  This object will be returned by
 * VrpnClient::make_device(), for attaching to a TrackerNode.
 *
 * It represents the data from just one particular sensor of a named VRPN
 * tracker, and may reflect either the sensor's position, its velocity, or its
 * acceleration.
 *
 * This class does not need to be exported from the DLL.
 */
class VrpnTrackerDevice : public ClientTrackerDevice {
public:
  enum DataType {
    DT_position,
    DT_velocity,
    DT_acceleration
  };

  VrpnTrackerDevice(VrpnClient *client, const std::string &device_name,
                    int sensor, DataType data_type,
                    VrpnTracker *vrpn_tracker);
  virtual ~VrpnTrackerDevice();

  INLINE int get_sensor() const;
  INLINE DataType get_data_type() const;
  INLINE VrpnTracker *get_vrpn_tracker() const;

private:
  int _sensor;
  DataType _data_type;
  VrpnTracker *_vrpn_tracker;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientTrackerDevice::init_type();
    register_type(_type_handle, "VrpnTrackerDevice",
                  ClientTrackerDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class VrpnTracker;
};

#include "vrpnTrackerDevice.I"

#endif
