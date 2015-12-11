// Filename: clientDevice.h
// Created by:  drose (25Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CLIENTDEVICE_H
#define CLIENTDEVICE_H

#include "pandabase.h"
#include "inputDevice.h"

class ClientBase;

////////////////////////////////////////////////////////////////////
//       Class : ClientDevice
// Description : Any of a number of different devices that might be
//               attached to a ClientBase, including trackers, etc.
//               This is an abstract interface; the actual
//               implementations are in ClientTrackerDevice, etc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE ClientDevice : public InputDevice {
protected:
  ClientDevice(ClientBase *client, TypeHandle device_type,
               const string &device_name, int device_flags=0);

public:
  virtual ~ClientDevice();

  INLINE ClientBase *get_client() const;
  INLINE TypeHandle get_device_type() const;

  void disconnect();

  virtual void do_poll() FINAL;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  ClientBase *_client;
  TypeHandle _device_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    InputDevice::init_type();
    register_type(_type_handle, "ClientDevice",
                  InputDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class ClientBase;
};

INLINE ostream &operator <<(ostream &out, const ClientDevice &device) {
  device.output(out);
  return out;
}

#include "clientDevice.I"

#endif
