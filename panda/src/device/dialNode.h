/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dialNode.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef DIALNODE_H
#define DIALNODE_H

#include "pandabase.h"

#include "clientBase.h"
#include "clientDialDevice.h"
#include "dataNode.h"


/**
 * This is the primary interface to infinite dial type devices associated with
 * a ClientBase.  This creates a node that connects to the named dial device,
 * if it exists, and provides hooks to the user to read the state of any of
 * the sequentially numbered dial controls associated with that device.
 *
 * A dial is a rotating device that does not have stops--it can keep rotating
 * any number of times.  Therefore it does not have a specific position at any
 * given time, unlike an AnalogDevice.
 */
class EXPCL_PANDA_DEVICE DialNode : public DataNode {
PUBLISHED:
  explicit DialNode(ClientBase *client, const std::string &device_name);
  virtual ~DialNode();

  INLINE bool is_valid() const;

  INLINE int get_num_dials() const;

  INLINE double read_dial(int index);
  INLINE bool is_dial_known(int index) const;

private:
  PT(ClientDialDevice) _dial;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // no inputs or outputs at the moment.

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "DialNode",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dialNode.I"

#endif
