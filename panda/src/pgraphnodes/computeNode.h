/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file computeNode.h
 * @author rdb
 * @date 2014-06-19
 */

#ifndef COMPUTENODE_H
#define COMPUTENODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "callbackObject.h"
#include "callbackNode.h"
#include "pointerTo.h"

/**
 * A special node, the sole purpose of which is to invoke a dispatch operation
 * on the assigned compute shader.
 */
class EXPCL_PANDA_PGRAPHNODES ComputeNode : public PandaNode {
PUBLISHED:
  explicit ComputeNode(const std::string &name);

  INLINE void add_dispatch(const LVecBase3i &num_groups);
  INLINE void add_dispatch(int num_groups_x, int num_groups_y, int num_groups_z);

  INLINE size_t get_num_dispatches() const;
  INLINE const LVecBase3i &get_dispatch(size_t i) const;
  INLINE void set_dispatch(size_t i, const LVecBase3i &num_groups);
  INLINE void insert_dispatch(size_t i, const LVecBase3i &num_groups);
  INLINE void remove_dispatch(size_t i);
  INLINE void clear_dispatches();

  MAKE_SEQ(get_dispatches, get_num_dispatches, get_dispatch);
  MAKE_SEQ_PROPERTY(dispatches, get_num_dispatches, get_dispatch, set_dispatch, remove_dispatch, insert_dispatch);

public:
  ComputeNode(const ComputeNode &copy);

  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;

  virtual bool is_renderable() const;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data);

  virtual void output(std::ostream &out) const;

public:
  class EXPCL_PANDA_PGRAPHNODES Dispatcher : public CallbackObject {
    friend class ComputeNode;
  public:
    ALLOC_DELETED_CHAIN(Dispatcher);
    Dispatcher();
    Dispatcher(const Dispatcher &copy);

    virtual void do_callback(CallbackData *cbdata);

    typedef pvector<LVecBase3i> Dispatches;

    class EXPCL_PANDA_PGRAPHNODES CData : public CycleData {
    public:
      INLINE CData();
      INLINE CData(const CData &copy);
      virtual CycleData *make_copy() const;
      virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
      virtual void fillin(DatagramIterator &scan, BamReader *manager);
      virtual TypeHandle get_parent_type() const {
        return CallbackNode::get_class_type();
      }

      Dispatches _dispatches;
    };

    PipelineCycler<CData> _cycler;
    typedef CycleDataReader<CData> CDReader;
    typedef CycleDataWriter<CData> CDWriter;

  };

private:
  // One per ComputeNode.
  PT(Dispatcher) _dispatcher;

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
    PandaNode::init_type();
    register_type(_type_handle, "ComputeNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "computeNode.I"

#endif
