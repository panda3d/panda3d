/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file analogNode.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef ANALOGNODE_H
#define ANALOGNODE_H

#include "pandabase.h"

#include "clientBase.h"
#include "clientAnalogDevice.h"
#include "dataNode.h"
#include "linmath_events.h"


/**
 * This is the primary interface to analog controls like sliders and joysticks
 * associated with a ClientBase.  This creates a node that connects to the
 * named analog device, if it exists, and provides hooks to the user to read
 * the state of any of the sequentially numbered controls associated with that
 * device.
 *
 * Each control can return a value ranging from -1 to 1, reflecting the
 * current position of the control within its total range of motion.
 *
 * The user may choose up to two analog controls to place on the data graph as
 * the two channels of an xy datagram, similarly to the way a mouse places its
 * position data.  In this way, an AnalogNode may be used in place of a mouse.
 */
class EXPCL_PANDA_DEVICE AnalogNode : public DataNode {
PUBLISHED:
  explicit AnalogNode(ClientBase *client, const std::string &device_name);
  explicit AnalogNode(InputDevice *device);
  virtual ~AnalogNode();

  INLINE bool is_valid() const;

  INLINE int get_num_controls() const;

  INLINE double get_control_state(int index) const;
  INLINE bool is_control_known(int index) const;

  INLINE void set_output(int channel, int index, bool flip);
  INLINE void clear_output(int channel);
  INLINE int get_output(int channel) const;
  INLINE bool is_output_flipped(int channel) const;

public:
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  class OutputData {
  public:
    INLINE OutputData();
    int _index;
    bool _flip;
  };

  enum { max_outputs = 2 };
  OutputData _outputs[max_outputs];

  PT(InputDevice) _analog;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // outputs
  int _xy_output;

  PT(EventStoreVec2) _xy;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "AnalogNode",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "analogNode.I"

#endif
