/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightNode.h
 * @author drose
 * @date 2002-03-26
 */

#ifndef LIGHTNODE_H
#define LIGHTNODE_H

#include "pandabase.h"

#include "light.h"
#include "pandaNode.h"

/**
 * A derivative of Light and of PandaNode.  All kinds of Light except
 * Spotlight (which must inherit from LensNode instead) inherit from this
 * class.
 */
class EXPCL_PANDA_PGRAPHNODES LightNode : public Light, public PandaNode {
PUBLISHED:
  explicit LightNode(const std::string &name);

protected:
  LightNode(const LightNode &copy);

public:
  virtual PandaNode *as_node();
  virtual Light *as_light();

PUBLISHED:
  // We have to explicitly publish these because they resolve the multiple
  // inheritance.
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Light::init_type();
    PandaNode::init_type();
    register_type(_type_handle, "LightNode",
                  Light::get_class_type(),
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const LightNode &light) {
  light.output(out);
  return out;
}

#include "lightNode.I"

#endif
