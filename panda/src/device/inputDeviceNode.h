/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file InputDeviceNode.h
 * @author fireclaw
 * @date 2016-07-14
 */

#ifndef INPUTDEVICENODE_H
#define INPUTDEVICENODE_H

#include "pandabase.h"

#include "dataNode.h"
#include "inputDeviceManager.h"
#include "linmath_events.h"

/**
 * Reads the controller data sent from the InputDeviceManager, and transmits
 * it down the data graph.
 *
 * This is intended to only be accessed from the app thread.
 */
class EXPCL_PANDA_DEVICE InputDeviceNode : public DataNode {
PUBLISHED:
  InputDeviceNode(InputDevice *device, const std::string &name);

public:
  void set_device(InputDevice *device);
  PT(InputDevice) get_device() const;

PUBLISHED:
  MAKE_PROPERTY(device, get_device, set_device);

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  pmap<ButtonHandle, bool> _button_states;
  pmap<InputDevice::Axis, double> _control_states;

  // outputs
  int _button_events_output;

  PT(InputDevice) _device;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "InputDeviceNode",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // INPUTDEVICENODE_H
