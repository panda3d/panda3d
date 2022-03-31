/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nodePathComponent.h
 * @author drose
 * @date 2002-02-25
 */

#ifndef NODEPATHCOMPONENT_H
#define NODEPATHCOMPONENT_H

#include "pandabase.h"

#include "pandaNode.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataLockedStageReader.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "lightMutex.h"
#include "deletedChain.h"

/**
 * This is one component of a NodePath.  These are stored on each PandaNode,
 * as many as one for each of the possible instances of the node (but they
 * only exist when they are requested, to minimize memory waste).  A NodePath
 * represents a singly-linked list of these from an arbitrary component in the
 * graph to the root.
 *
 * This whole NodePath system is used to disambiguate instances in the scene
 * graph, and the NodePathComponents are stored in the nodes themselves to
 * allow the nodes to keep these up to date as the scene graph is manipulated.
 */
class EXPCL_PANDA_PGRAPH NodePathComponent final : public ReferenceCount {
private:
  NodePathComponent(PandaNode *node, NodePathComponent *next,
                    int pipeline_stage, Thread *current_thread);

public:
  NodePathComponent(const NodePathComponent &copy) = delete;
  INLINE ~NodePathComponent();

  ALLOC_DELETED_CHAIN(NodePathComponent);

  NodePathComponent &operator = (const NodePathComponent &copy) = delete;

  INLINE PandaNode *get_node() const;
  INLINE bool has_key() const;
  int get_key() const;
  bool is_top_node(int pipeline_stage, Thread *current_thread) const;

  INLINE NodePathComponent *get_next(int pipeline_stage, Thread *current_thread) const;
  int get_length(int pipeline_stage, Thread *current_thread) const;

  bool fix_length(int pipeline_stage, Thread *current_thread);

  void output(std::ostream &out) const;

private:
  void set_next(NodePathComponent *next, int pipeline_stage, Thread *current_thread);
  void set_top_node(int pipeline_stage, Thread *current_thread);

  // We don't have to cycle the _node and _key elements, since these are
  // permanent properties of this object.  (Well, the _key is semi-permanent:
  // it becomes permanent after it has been set the first time.)
  PT(PandaNode) _node;
  int _key;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PGRAPH CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    ALLOC_DELETED_CHAIN(CData);
    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return NodePathComponent::get_class_type();
    }

    PT(NodePathComponent) _next;
    int _length;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "NodePathComponent::CData");
    }

  private:
    static TypeHandle _type_handle;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataLockedStageReader<CData> CDLockedStageReader;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  static int _next_key;
  static LightMutex _key_lock;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "NodePathComponent",
                  ReferenceCount::get_class_type());
    CData::init_type();
  }

private:
  static TypeHandle _type_handle;
  friend class PandaNode;
  friend class NodePath;
};

// We can safely redefine this as a no-op.
template<>
INLINE void PointerToBase<NodePathComponent>::update_type(To *ptr) {}

INLINE std::ostream &operator << (std::ostream &out, const NodePathComponent &comp);

#include "nodePathComponent.I"

#endif
