/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDeviceSet.h
 * @author rdb
 * @date 2015-12-16
 */

#ifndef INPUTDEVICESET_H
#define INPUTDEVICESET_H

#include "pandabase.h"
#include "ordered_vector.h"
#include "inputDevice.h"

/**
 * Manages a list of InputDevice objects, as returned by various
 * InputDeviceManager methods.  This is implemented like a set, meaning the
 * same device cannot occur more than once.
 */
class EXPCL_PANDA_DEVICE InputDeviceSet {
PUBLISHED:
  InputDeviceSet();
  InputDeviceSet(const InputDeviceSet &copy);
  void operator = (const InputDeviceSet &copy);
  INLINE ~InputDeviceSet();

public:
  void add_device(InputDevice *device);
  bool remove_device(InputDevice *device);
  void add_devices_from(const InputDeviceSet &other);
  void remove_devices_from(const InputDeviceSet &other);
  bool has_device(InputDevice *device) const;

PUBLISHED:
  void clear();
  void reserve(size_t num);

  INLINE InputDevice *operator [] (size_t index) const;
  INLINE size_t size() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  // This is currently implemented as ov_set instead of a regular set so that
  // we can still support random access.
  typedef ov_set<PT(InputDevice)> InputDevices;
  InputDevices _devices;
};

INLINE std::ostream &operator << (std::ostream &out, const InputDeviceSet &col) {
  col.output(out);
  return out;
}

#include "inputDeviceSet.I"

#endif
