/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file forceNode.h
 * @author charles
 * @date 2000-08-02
 */

#ifndef FORCENODE_H
#define FORCENODE_H

#include "pandaNode.h"
#include "pvector.h"

#include "baseForce.h"

/**
 * A force that lives in the scene graph and is therefore subject to local
 * coordinate systems.  An example of this would be simulating gravity in a
 * rotating space station.  or something.
 */
class EXPCL_PANDA_PHYSICS ForceNode : public PandaNode {
PUBLISHED:
  explicit ForceNode(const std::string &name);
  INLINE void clear();
  INLINE BaseForce *get_force(size_t index) const;
  INLINE size_t get_num_forces() const;
  MAKE_SEQ(get_forces, get_num_forces, get_force);
  INLINE void add_force(BaseForce *force);

  void add_forces_from(const ForceNode &other);
  void set_force(size_t index, BaseForce *force);
  void insert_force(size_t index, BaseForce *force);
  void remove_force(BaseForce *force);
  void remove_force(size_t index);

  MAKE_SEQ_PROPERTY(forces, get_num_forces, get_force, set_force, remove_force, insert_force);

  virtual void output(std::ostream &out) const;
  virtual void write_forces(std::ostream &out, int indent=0) const;
  virtual void write(std::ostream &out, int indent=0) const;

public:
  virtual ~ForceNode();
  virtual bool safe_to_flatten() const { return false; }
  virtual PandaNode *make_copy() const;

protected:
  ForceNode(const ForceNode &copy);

private:
  typedef pvector< PT(BaseForce) > ForceVector;
  ForceVector _forces;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ForceNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "forceNode.I"

#endif // FORCENODE_H
