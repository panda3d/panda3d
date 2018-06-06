/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnClient.h
 * @author jason
 * @date 2000-08-04
 */

#ifndef VRPNCLIENT_H
#define VRPNCLIENT_H

#include "pandabase.h"
#include "clientBase.h"

#include "vrpn_interface.h"

class VrpnTracker;
class VrpnTrackerDevice;
class VrpnButton;
class VrpnButtonDevice;
class VrpnAnalog;
class VrpnAnalogDevice;
class VrpnDial;
class VrpnDialDevice;

/**
 * A specific ClientBase that connects to a VRPN server and records
 * information on the connected VRPN devices.
 */
class EXPCL_VRPN VrpnClient : public ClientBase {
PUBLISHED:
  explicit VrpnClient(const std::string &server_name);
  ~VrpnClient();

  INLINE const std::string &get_server_name() const;
  INLINE bool is_valid() const;
  INLINE bool is_connected() const;

  void write(std::ostream &out, int indent_level = 0) const;

public:
  INLINE static double convert_to_secs(struct timeval msg_time);

protected:
  virtual PT(ClientDevice) make_device(TypeHandle device_type,
                                       const std::string &device_name);

  virtual bool disconnect_device(TypeHandle device_type,
                                 const std::string &device_name,
                                 ClientDevice *device);

  virtual void do_poll();

private:
  PT(ClientDevice) make_tracker_device(const std::string &device_name);
  PT(ClientDevice) make_button_device(const std::string &device_name);
  PT(ClientDevice) make_analog_device(const std::string &device_name);
  PT(ClientDevice) make_dial_device(const std::string &device_name);
  void disconnect_tracker_device(VrpnTrackerDevice *device);
  void disconnect_button_device(VrpnButtonDevice *device);
  void disconnect_analog_device(VrpnAnalogDevice *device);
  void disconnect_dial_device(VrpnDialDevice *device);

  VrpnTracker *get_tracker(const std::string &tracker_name);
  void free_tracker(VrpnTracker *vrpn_tracker);

  VrpnButton *get_button(const std::string &button_name);
  void free_button(VrpnButton *vrpn_button);

  VrpnAnalog *get_analog(const std::string &analog_name);
  void free_analog(VrpnAnalog *vrpn_analog);

  VrpnDial *get_dial(const std::string &dial_name);
  void free_dial(VrpnDial *vrpn_dial);

private:
  std::string _server_name;
  vrpn_Connection *_connection;

  typedef pmap<std::string, VrpnTracker *> Trackers;
  typedef pmap<std::string, VrpnButton *> Buttons;
  typedef pmap<std::string, VrpnAnalog *> Analogs;
  typedef pmap<std::string, VrpnDial *> Dials;

  Trackers _trackers;
  Buttons _buttons;
  Analogs _analogs;
  Dials _dials;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientBase::init_type();
    register_type(_type_handle, "VrpnClient",
                  ClientBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "vrpnClient.I"

#endif
