/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelNode.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef MODELNODE_H
#define MODELNODE_H

#include "pandabase.h"

#include "pandaNode.h"

/**
 * This node is placed at key points within the scene graph to indicate the
 * roots of "models": subtrees that are conceptually to be treated as a single
 * unit, like a car or a room, for instance.  It doesn't affect rendering or
 * any other operations; it's primarily useful as a high-level model
 * indication.
 *
 * ModelNodes are created in response to a <Model> { 1 } flag within an egg
 * file.
 */
class EXPCL_PANDA_PGRAPH ModelNode : public PandaNode {
PUBLISHED:
  explicit INLINE ModelNode(const std::string &name);

protected:
  INLINE ModelNode(const ModelNode &copy);

public:
  virtual PandaNode *make_copy() const;

  virtual PandaNode *combine_with(PandaNode *other);
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_flatten_below() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool preserve_name() const;
  virtual int get_unsafe_to_apply_attribs() const;

PUBLISHED:
  enum PreserveTransform {
    PT_none,
    PT_local,
    PT_net,
    PT_drop_node,
    PT_no_touch,
  };

  INLINE void set_preserve_transform(PreserveTransform preserve_transform);
  INLINE PreserveTransform get_preserve_transform() const;

  INLINE void set_preserve_attributes(int attrib_mask);
  INLINE int get_preserve_attributes() const;

  void set_transform_limit(PN_stdfloat limit) { _transform_limit = limit; };

private:
  PreserveTransform _preserve_transform;
  int _preserve_attributes;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

  virtual void transform_changed();
  void test_transform(const TransformState *ts) const;

  PN_stdfloat _transform_limit;



public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "ModelNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "modelNode.I"

#endif
