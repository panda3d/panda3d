// Filename: clientButtonDevice.h
// Created by:  drose (26Jan01)
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

#ifndef CLIENTBUTTONDEVICE_H
#define CLIENTBUTTONDEVICE_H

#include "pandabase.h"

#include "clientDevice.h"

#include "buttonHandle.h"
#include "buttonEvent.h"
#include "buttonEventList.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : ClientButtonDevice
// Description : A device, attached to the ClientBase by a
//               ButtonNode, that records the data from a single
//               named button device.  The named device can contain
//               any number of up/down style buttons, numbered in
//               sequence beginning at zero; these are mapped by this
//               class to a sequence of ButtonHandles specified by the
//               user.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DEVICE ClientButtonDevice : public ClientDevice {
protected:
  ClientButtonDevice(ClientBase *client, const string &device_name);

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientDevice::init_type();
    register_type(_type_handle, "ClientButtonDevice",
                  ClientDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "clientButtonDevice.I"

#endif
