/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file instancedNode.h
 * @author rdb
 * @date 2019-03-09
 */

#ifndef INSTANCEDNODE_H
#define INSTANCEDNODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "copyOnWritePointer.h"
#include "instanceList.h"

/**
 * This is a special node that instances its contents using a list of
 * transforms that get applied on top of the node's own transform.  This is a
 * bit more limited than the regular instance_to mechanism, but it is a better
 * choice for hardware instancing.
 *
 * For best performance, it is highly recommended to flatten the nodes under
 * this (by calling flatten_strong()), since culling will not be performed for
 * individual sub-nodes under each instance.
 *
 * @since 1.11.0
 */
class EXPCL_PANDA_PGRAPH InstancedNode : public PandaNode {
PUBLISHED:
  explicit InstancedNode(const std::string &name);

protected:
  InstancedNode(const InstancedNode &copy);

public:
  virtual ~InstancedNode();
  virtual PandaNode *make_copy() const override;

  INLINE size_t get_num_instances() const;
  INLINE CPT(InstanceList) get_instances(Thread *current_thread = Thread::get_current_thread()) const;
  PT(InstanceList) modify_instances();
  void set_instances(PT(InstanceList) instances);

PUBLISHED:
  MAKE_PROPERTY(instances, modify_instances, set_instances);

public:
  virtual bool safe_to_flatten() const override;
  virtual bool safe_to_combine() const override;
  virtual void xform(const LMatrix4 &mat) override;
  virtual PandaNode *combine_with(PandaNode *other) override;

  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread) const override;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data) override;

  virtual void output(std::ostream &out) const override;

protected:
  virtual void compute_external_bounds(CPT(BoundingVolume) &external_bounds,
                                       BoundingVolume::BoundsType btype,
                                       const BoundingVolume **volumes,
                                       size_t num_volumes,
                                       int pipeline_stage,
                                       Thread *current_thread) const override;

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PGRAPH CData final : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const override;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const override;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;
    virtual void fillin(DatagramIterator &scan, BamReader *manager) override;
    virtual TypeHandle get_parent_type() const override {
      return InstancedNode::get_class_type();
    }

  private:
    COWPT(InstanceList) _instances;

    friend class InstancedNode;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "InstancedNode::CData");
    }

  private:
    static TypeHandle _type_handle;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataLockedStageReader<CData> CDLockedStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "InstancedNode",
                  PandaNode::get_class_type());
    CData::init_type();
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "instancedNode.I"

#endif
