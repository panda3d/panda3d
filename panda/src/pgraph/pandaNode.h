// Filename: pandaNode.h
// Created by:  drose (20Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PANDANODE_H
#define PANDANODE_H

#include "pandabase.h"

#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "renderState.h"

#include "typedWritable.h"
#include "boundedObject.h"
#include "namable.h"
#include "referenceCount.h"
#include "luse.h"
#include "ordered_vector.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : PandaNode
// Description : A basic node of the scene graph or data graph.  This
//               is the base class of all specialized nodes, and also
//               serves as a generic node with no special properties.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PandaNode : public TypedWritable, public Namable,
                              virtual public ReferenceCount {
PUBLISHED:
  PandaNode(const string &name);
  PandaNode(const PandaNode &copy);
  void operator = (const PandaNode &copy);
  virtual ~PandaNode();

  INLINE int get_num_parents() const;
  INLINE PandaNode *get_parent(int n) const;
  INLINE int find_parent(PandaNode *node) const;

  INLINE int get_num_children() const;
  INLINE PandaNode *get_child(int n) const;
  INLINE int get_child_sort(int n) const;
  int find_child(PandaNode *node) const;

  int add_child(PandaNode *child, int sort = 0);
  void remove_child(int n);
  bool remove_child(PandaNode *child);

  /*
  bool stash_child(PandaNode *child);
  bool unstash_child(PandaNode *child);
  INLINE int get_num_stashed() const;
  INLINE PandaNode *get_stashed(int n) const;
  INLINE PandaNode *get_stashed_sort(int n) const;
  */

  void remove_all_children();

  INLINE void set_attrib(const RenderAttrib *attrib, int override = 0);
  INLINE const RenderAttrib *get_attrib(TypeHandle type) const;
  INLINE void clear_attrib(TypeHandle type);

  INLINE void set_state(const RenderState *state);
  INLINE const RenderState *get_state() const;
  INLINE void clear_state();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

public:
  virtual bool is_geom_node() const;

private:
  class EXPCL_PANDA DownConnection {
  public:
    INLINE DownConnection(PandaNode *child, int sort);
    INLINE bool operator < (const DownConnection &other) const;
    INLINE PandaNode *get_child() const;
    INLINE int get_sort() const;

  private:
    // Child pointers are reference counted.  That way, holding a
    // pointer to the root of a subgraph keeps the entire subgraph
    // around.
    PT(PandaNode) _child;
    int _sort;
  };
  typedef ov_multiset<DownConnection> Down;
  // Parent pointers are not reference counted.  That way, parents and
  // children do not circularly reference each other.  In fact, parent
  // pointers are just simple pointers, with no additional data.  We
  // don't really need to keep the parent pointers around, but it's
  // nice to be able to walk up the graph.
  typedef ov_set<PandaNode *> Up;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;

    Down _down;
    Up _up;

    BoundedObject _node_bounds;
    BoundedObject _subgraph_bounds;
    CPT(RenderState) _state_changes;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  // Use this interface when you want to walk through the list of
  // children.  This saves a tiny bit of overhead between each step,
  // by keeping the PipelineCycler open for reading the whole time.
  // However, it does not protect you from self-modifying loops.
  class EXPCL_PANDA Children {
  public:
    INLINE Children(const CDReader &cdata);
    INLINE Children(const Children &copy);

    INLINE int get_num_children() const;
    INLINE PandaNode *get_child(int n) const;

  private:
    CDReader _cdata;
  };

  INLINE Children get_children() const;

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
    TypedWritable::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "PandaNode",
                  TypedWritable::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PandaNode::Children;
};

INLINE ostream &operator << (ostream &out, const PandaNode &node) {
  node.output(out);
  return out;
}

#include "pandaNode.I"

#endif

