/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ambientLight.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef AMBIENTLIGHT_H
#define AMBIENTLIGHT_H

#include "pandabase.h"

#include "lightNode.h"

/**
 * A light source that seems to illuminate all points in space at once.  This
 * kind of light need not actually be part of the scene graph, since it has no
 * meaningful position.
 */
class EXPCL_PANDA_PGRAPHNODES AmbientLight : public LightNode {
PUBLISHED:
  explicit AmbientLight(const std::string &name);

protected:
  AmbientLight(const AmbientLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void write(std::ostream &out, int indent_level) const;
  virtual bool is_ambient_light() const final;

PUBLISHED:
  virtual int get_class_priority() const;

public:
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id);

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LightNode::init_type();
    register_type(_type_handle, "AmbientLight",
                  LightNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const AmbientLight &light) {
  light.output(out);
  return out;
}

#include "ambientLight.I"

#endif
