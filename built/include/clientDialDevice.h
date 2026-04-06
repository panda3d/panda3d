/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientDialDevice.h
 * @author drose
 * @date 2001-01-26
 */

#ifndef CLIENTDIALDEVICE_H
#define CLIENTDIALDEVICE_H

#include "pandabase.h"

#include "clientDevice.h"

/**
 * A device, attached to the ClientBase by a DialNode, that records the data
 * from a single named dial device.  The named device can contain any number
 * of dials, numbered in sequence beginning at zero.
 *
 * A dial is a rotating device that does not have stops--it can keep rotating
 * any number of times.  Therefore it does not have a specific position at any
 * given time, unlike an AnalogDevice.
 */
class EXPCL_PANDA_DEVICE ClientDialDevice : public ClientDevice {
protected:
  INLINE ClientDialDevice(ClientBase *client, const std::string &device_name);

public:
  INLINE int get_num_dials() const;

  INLINE void push_dial(int index, double offset);
  INLINE double read_dial(int index);
  INLINE bool is_dial_known(int index) const;

private:
  void ensure_dial_index(int index);

protected:
  class DialState {
  public:
    INLINE DialState();

    double _offset;
    bool _known;
  };

  typedef pvector<DialState> Dials;
  Dials _dials;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ClientDevice::init_type();
    register_type(_type_handle, "ClientDialDevice",
                  ClientDevice::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "clientDialDevice.I"

#endif
