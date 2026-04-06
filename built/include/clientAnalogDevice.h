/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientAnalogDevice.h
 * @author drose
 * @date 2001-01-26
 */

#ifndef CLIENTANALOGDEVICE_H
#define CLIENTANALOGDEVICE_H

#include "pandabase.h"

#include "clientDevice.h"

/**
 * A device, attached to the ClientBase by a AnalogNode, that records the data
 * from a single named analog device.  The named device can contain any number
 * of analog controls, numbered in sequence beginning at zero.
 *
 * Each analog control returns a value ranging from -1 to 1, reflecting the
 * current position of the control within its total range of motion.
 */
class EXPCL_PANDA_DEVICE ClientAnalogDevice : public ClientDevice {
protected:
  INLINE ClientAnalogDevice(ClientBase *client, const std::string &device_name);

public:
  virtual void write(std::ostream &out, int indent_level = 0) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientDevice::init_type();
    register_type(_type_handle, "ClientAnalogDevice",
                  ClientDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "clientAnalogDevice.I"

#endif
