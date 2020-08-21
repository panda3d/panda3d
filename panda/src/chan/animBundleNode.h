/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animBundleNode.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef ANIMBUNDLENODE_H
#define ANIMBUNDLENODE_H

#include "pandabase.h"

#include "animBundle.h"

#include "pandaNode.h"
#include "dcast.h"

/**
 * This is a node that contains a pointer to an AnimBundle.  Like
 * PartBundleNode, it exists solely to make it easy to store AnimBundles in
 * the scene graph.
 */
class EXPCL_PANDA_CHAN AnimBundleNode : public PandaNode {
PUBLISHED:
  INLINE explicit AnimBundleNode(const std::string &name, AnimBundle *bundle);

protected:
  INLINE AnimBundleNode();
  INLINE AnimBundleNode(const AnimBundleNode &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_flatten() const;

PUBLISHED:
  INLINE AnimBundle *get_bundle() const;

  MAKE_PROPERTY(bundle, get_bundle);

  static AnimBundle *find_anim_bundle(PandaNode *root);

private:
  PT(AnimBundle) _bundle;

public:
  virtual void output(std::ostream &out) const;

  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "AnimBundleNode",
                  PandaNode::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animBundleNode.I"

#endif
