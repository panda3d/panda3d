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
#include "transformState.h"

#include "typedWritable.h"
#include "boundedObject.h"
#include "namable.h"
#include "referenceCount.h"
#include "luse.h"
#include "ordered_vector.h"
#include "pointerTo.h"
#include "notify.h"

class qpNodePathComponent;
class CullTraverserData;

////////////////////////////////////////////////////////////////////
//       Class : PandaNode
// Description : A basic node of the scene graph or data graph.  This
//               is the base class of all specialized nodes, and also
//               serves as a generic node with no special properties.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PandaNode : public TypedWritable, public Namable,
                              public BoundedObject,
                              virtual public ReferenceCount {
PUBLISHED:
  PandaNode(const string &name);
  virtual ~PandaNode();

protected:
  PandaNode(const PandaNode &copy);
private:
  void operator = (const PandaNode &copy);

public:
  virtual PandaNode *make_copy() const;
  PandaNode *copy_subgraph() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_combine() const;
  virtual void xform(const LMatrix4f &mat);
  virtual PandaNode *combine_with(PandaNode *other); 

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverserData &data);
  virtual bool has_selective_visibility() const;
  virtual int get_first_visible_child() const;
  virtual int get_next_visible_child(int n) const;

PUBLISHED:
  INLINE int get_num_parents() const;
  INLINE PandaNode *get_parent(int n) const;
  INLINE int find_parent(PandaNode *node) const;

  INLINE int get_num_children() const;
  INLINE PandaNode *get_child(int n) const;
  INLINE int get_child_sort(int n) const;
  int find_child(PandaNode *node) const;

  void add_child(PandaNode *child_node, int sort = 0);
  void remove_child(int n);
  bool remove_child(PandaNode *child_node);

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
  INLINE bool has_attrib(TypeHandle type) const;
  INLINE void clear_attrib(TypeHandle type);

  INLINE void set_state(const RenderState *state);
  INLINE const RenderState *get_state() const;
  INLINE void clear_state();

  INLINE void set_transform(const TransformState *transform);
  INLINE const TransformState *get_transform() const;
  INLINE void clear_transform();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  INLINE void ls(ostream &out, int indent_level) const;

  // A node has two bounding volumes: the BoundedObject it inherits
  // from is the "external" bound and represnts the node and all of
  // its children, while the _internal_bound object is the "internal"
  // bounds and represents only the node itself.

  // We remap the inherited set_bound() and get_bound() functions so
  // that set_bound() to a type sets the type of the external bound,
  // while set_bound() to a specific bounding volume sets the volume
  // of the *internal* bound.  At the same time, get_bound() returns
  // the external bound.  Although it might seem strange and confusing
  // to do this, this is actually the natural way the user thinks
  // about nodes and bounding volumes.
  INLINE void set_bound(BoundingVolumeType type);
  INLINE void set_bound(const BoundingVolume &volume);
  INLINE const BoundingVolume &get_bound() const;
  INLINE const BoundingVolume &get_internal_bound() const;

public:
  virtual bool is_geom_node() const;

protected:
  // Inherited from BoundedObject
  virtual void propagate_stale_bound();
  virtual BoundingVolume *recompute_bound();

  // Local to PandaNode
  virtual BoundingVolume *recompute_internal_bound();
  INLINE void changed_internal_bound();
  virtual void parents_changed();
  virtual void children_changed();

  // This is the bounding volume around the contents of the node
  // itself (without including all of the node's children).
  // BoundedObject is itself cycled, so we don't need to protect it.
  BoundedObject _internal_bound;

private:
  // parent-child manipulation for qpNodePath support.  Don't try to
  // call these directly.
  static PT(qpNodePathComponent) attach(qpNodePathComponent *parent, 
                                       PandaNode *child, int sort);
  static void detach(qpNodePathComponent *child);
  static void reparent(qpNodePathComponent *new_parent,
                       qpNodePathComponent *child, int sort);
  static PT(qpNodePathComponent) get_component(qpNodePathComponent *parent,
                                              PandaNode *child);
  static PT(qpNodePathComponent) get_top_component(PandaNode *child);
  PT(qpNodePathComponent) get_generic_component();
  void delete_component(qpNodePathComponent *component);
  class CData;
  void fix_chain_lengths(const CData *cdata);
  void r_list_descendants(ostream &out, int indent_level) const;

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

  class EXPCL_PANDA UpConnection {
  public:
    INLINE UpConnection(PandaNode *child);
    INLINE bool operator < (const UpConnection &other) const;
    INLINE PandaNode *get_parent() const;

  private:
    // Parent pointers are not reference counted.  That way, parents and
    // children do not circularly reference each other.
    PandaNode *_parent;
  };
  typedef ov_set<UpConnection> Up;

  // We also maintain a set of qpNodePathComponents in the node.  This
  // represents the set of instances of this node that we have
  // requested a qpNodePath for.  We don't keep reference counts; when
  // each qpNodePathComponent destructs, it removes itself from this
  // set.
  typedef pset<qpNodePathComponent *> Chains;
  
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    Down _down;
    Up _up;
    Chains _chains;

    CPT(RenderState) _state;
    CPT(TransformState) _transform;
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
    BoundedObject::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "PandaNode",
                  TypedWritable::get_class_type(),
                  BoundedObject::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PandaNode::Children;
  friend class qpNodePath;
  friend class qpNodePathComponent;
};

INLINE ostream &operator << (ostream &out, const PandaNode &node) {
  node.output(out);
  return out;
}

#include "pandaNode.I"

#endif

