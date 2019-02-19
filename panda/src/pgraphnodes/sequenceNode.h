/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sequenceNode.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef SEQUENCENODE_H
#define SEQUENCENODE_H

#include "pandabase.h"

#include "selectiveChildNode.h"
#include "animInterface.h"
#include "clockObject.h"

/**
 * A node that automatically cycles through rendering each one of its children
 * according to its frame rate.
 */
class EXPCL_PANDA_PGRAPHNODES SequenceNode : public SelectiveChildNode, public AnimInterface {
PUBLISHED:
  INLINE explicit SequenceNode(const std::string &name);

protected:
  SequenceNode(const SequenceNode &copy);

PUBLISHED:
  virtual int get_num_frames() const;
  INLINE void set_frame_rate(double frame_rate);

  MAKE_PROPERTY(frame_rate, get_frame_rate, set_frame_rate);

public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual int get_first_visible_child() const;
  virtual bool has_single_child_visibility() const;
  virtual int get_visible_child() const;

  virtual void output(std::ostream &out) const;

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
    SelectiveChildNode::init_type();
    AnimInterface::init_type();
    register_type(_type_handle, "SequenceNode",
                  SelectiveChildNode::get_class_type(),
                  AnimInterface::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sequenceNode.I"

#endif
