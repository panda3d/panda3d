/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transform2sg.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef TRANSFORM2SG_H
#define TRANSFORM2SG_H

#include "pandabase.h"

#include "dataNode.h"
#include "pandaNode.h"

/**
 * input: Transform (matrix)
 *
 * output: none, but applies the matrix as the transform transition for a
 * given arc of the scene graph.
 */
class EXPCL_PANDA_TFORM Transform2SG : public DataNode {
PUBLISHED:
  explicit Transform2SG(const std::string &name);

  void set_node(PandaNode *node);
  PandaNode *get_node() const;

private:
  PT(PandaNode) _node;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _transform_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "Transform2SG",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
