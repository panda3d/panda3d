/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file selectiveChildNode.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef SELECTIVECHILDNODE_H
#define SELECTIVECHILDNODE_H

#include "pandabase.h"

#include "pandaNode.h"

/**
 * A base class for nodes like LODNode and SequenceNode that select only one
 * visible child at a time.
 *
 * This class is now vestigial.
 */
class EXPCL_PANDA_PGRAPHNODES SelectiveChildNode : public PandaNode {
PUBLISHED:
  INLINE explicit SelectiveChildNode(const std::string &name);

protected:
  INLINE SelectiveChildNode(const SelectiveChildNode &copy);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "SelectiveChildNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "selectiveChildNode.I"

#endif
