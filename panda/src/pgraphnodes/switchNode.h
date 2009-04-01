// Filename: switchNode.h
// Created by:  drose (31Jul02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef SWITCHNODE_H
#define SWITCHNODE_H

#include "pandabase.h"

#include "selectiveChildNode.h"

////////////////////////////////////////////////////////////////////
//       Class : SwitchNode
// Description : A node that renders only one of its children,
//               according to the user's indication.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPHNODES SwitchNode : public SelectiveChildNode {
PUBLISHED:
  INLINE SwitchNode(const string &name);

public:
  SwitchNode(const SwitchNode &copy);

  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual int get_first_visible_child() const;
  virtual bool has_single_child_visibility() const;

PUBLISHED:
  INLINE void set_visible_child(int index);
  virtual int get_visible_child() const;

private:
  class EXPCL_PANDA_PGRAPHNODES CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return SwitchNode::get_class_type();
    }

    int _visible_child;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

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
    register_type(_type_handle, "SwitchNode",
                  SelectiveChildNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "switchNode.I"

#endif
