// Filename: pandaNode.h
// Created by:  drose (20Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
#include "renderEffects.h"
#include "transformState.h"
#include "drawMask.h"
#include "typedWritable.h"
#include "boundedObject.h"
#include "collideMask.h"
#include "namable.h"
#include "referenceCount.h"
#include "luse.h"
#include "ordered_vector.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "notify.h"

class NodePathComponent;
class CullTraverser;
class CullTraverserData;
class Light;
class FactoryParams;
class AccumulatedAttribs;
class GeomTransformer;

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

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_flatten_below() const;
  virtual bool preserve_name() const;
  virtual int get_unsafe_to_apply_attribs() const;
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);
  virtual void xform(const LMatrix4f &mat);
  virtual PandaNode *combine_with(PandaNode *other); 
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                      bool &found_any,
                      const TransformState *transform) const;
  
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool has_selective_visibility() const;
  virtual int get_first_visible_child() const;
  virtual int get_next_visible_child(int n) const;
  virtual bool has_single_child_visibility() const;
  virtual int get_visible_child() const;

PUBLISHED:
  PT(PandaNode) copy_subgraph() const;

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
  bool replace_child(PandaNode *orig_child, PandaNode *new_child);

  INLINE bool stash_child(PandaNode *child_node);
  void stash_child(int child_index);
  INLINE bool unstash_child(PandaNode *child_node);
  void unstash_child(int stashed_index);

  INLINE int get_num_stashed() const;
  INLINE PandaNode *get_stashed(int n) const;
  INLINE int get_stashed_sort(int n) const;
  int find_stashed(PandaNode *node) const;

  void add_stashed(PandaNode *child_node, int sort = 0);
  void remove_stashed(int n);

  void remove_all_children();
  void steal_children(PandaNode *other);
  void copy_children(PandaNode *other);

  INLINE void set_attrib(const RenderAttrib *attrib, int override = 0);
  INLINE const RenderAttrib *get_attrib(TypeHandle type) const;
  INLINE bool has_attrib(TypeHandle type) const;
  INLINE void clear_attrib(TypeHandle type);

  INLINE void set_effect(const RenderEffect *effect);
  INLINE const RenderEffect *get_effect(TypeHandle type) const;
  INLINE bool has_effect(TypeHandle type) const;
  INLINE void clear_effect(TypeHandle type);

  INLINE void set_state(const RenderState *state);
  INLINE const RenderState *get_state() const;
  INLINE void clear_state();

  INLINE void set_effects(const RenderEffects *effects);
  INLINE const RenderEffects *get_effects() const;
  INLINE void clear_effects();

  INLINE void set_transform(const TransformState *transform);
  INLINE const TransformState *get_transform() const;
  INLINE void clear_transform();

  INLINE void set_prev_transform(const TransformState *transform);
  INLINE const TransformState *get_prev_transform() const;
  INLINE void reset_prev_transform();

  INLINE void set_tag(const string &key, const string &value);
  INLINE string get_tag(const string &key) const;
  INLINE bool has_tag(const string &key) const;
  INLINE void clear_tag(const string &key);
  void copy_tags(PandaNode *other);
  void list_tags(ostream &out, const string &separator = "\n") const;

  INLINE void set_draw_mask(DrawMask mask);
  INLINE DrawMask get_draw_mask() const;

  INLINE void set_into_collide_mask(CollideMask mask);
  INLINE CollideMask get_into_collide_mask() const;
  virtual CollideMask get_legal_collide_mask() const;

  INLINE CollideMask get_net_collide_mask() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  INLINE void ls(ostream &out, int indent_level) const;

  // A node has two bounding volumes: the BoundedObject it inherits
  // from is the "external" bound and represents the node and all of
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

  virtual bool is_geom_node() const;
  virtual bool is_lod_node() const;
  virtual Light *as_light();

protected:
  // Inherited from BoundedObject
  virtual void propagate_stale_bound();
  virtual BoundingVolume *recompute_bound();

  // Local to PandaNode
  virtual BoundingVolume *recompute_internal_bound();
  INLINE void changed_internal_bound();
  virtual void parents_changed();
  virtual void children_changed();
  virtual void transform_changed();
  INLINE void add_net_collide_mask(CollideMask mask);

  typedef pmap<PandaNode *, PandaNode *> InstanceMap;
  virtual PT(PandaNode) r_copy_subgraph(InstanceMap &inst_map) const;
  virtual void r_copy_children(const PandaNode *from, InstanceMap &inst_map);

  // This is the bounding volume around the contents of the node
  // itself (without including all of the node's children).
  // BoundedObject is itself cycled, so we don't need to protect it.
  BoundedObject _internal_bound;

private:
  class CData;

  // parent-child manipulation for NodePath support.  Don't try to
  // call these directly.
  static PT(NodePathComponent) attach(NodePathComponent *parent, 
                                       PandaNode *child, int sort);
  static void detach(NodePathComponent *child);
  static bool reparent(NodePathComponent *new_parent,
                       NodePathComponent *child, int sort, bool as_stashed);
  static PT(NodePathComponent) get_component(NodePathComponent *parent,
                                              PandaNode *child);
  static PT(NodePathComponent) get_top_component(PandaNode *child,
                                                   bool force);
  PT(NodePathComponent) get_generic_component(bool accept_ambiguity);
  PT(NodePathComponent) r_get_generic_component(bool accept_ambiguity, bool &ambiguity_detected);
  void delete_component(NodePathComponent *component);
  static void sever_connection(PandaNode *parent_node, PandaNode *child_node);
  static void new_connection(PandaNode *parent_node, PandaNode *child_node);
  void fix_path_lengths(const CData *cdata);
  void r_list_descendants(ostream &out, int indent_level) const;

public:
  // This must be declared public so that VC6 will allow the nested
  // CData class to access it.
  class EXPCL_PANDA DownConnection {
  public:
    INLINE DownConnection(PandaNode *child, int sort);
    INLINE bool operator < (const DownConnection &other) const;
    INLINE PandaNode *get_child() const;
    INLINE void set_child(PandaNode *child);
    INLINE int get_sort() const;

  private:
    // Child pointers are reference counted.  That way, holding a
    // pointer to the root of a subgraph keeps the entire subgraph
    // around.
    PT(PandaNode) _child;
    int _sort;
  };

private:
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

  // We also maintain a set of NodePathComponents in the node.  This
  // represents the set of instances of this node that we have
  // requested a NodePath for.  We don't keep reference counts; when
  // each NodePathComponent destructs, it removes itself from this
  // set.
  typedef pset<NodePathComponent *> Paths;

  // This is used to maintain a table of keyed data on each node, for
  // the user's purposes.
  typedef pmap<string, string> TagData;

  
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    void write_up_list(const Up &up_list,
                       BamWriter *manager, Datagram &dg) const;
    void write_down_list(const Down &down_list,
                         BamWriter *manager, Datagram &dg) const;
    int complete_up_list(Up &up_list,
                         TypedWritable **p_list, BamReader *manager);
    int complete_down_list(Down &down_list,
                           TypedWritable **p_list, BamReader *manager);
    void fillin_up_list(Up &up_list,
                        DatagramIterator &scan, BamReader *manager);
    void fillin_down_list(Down &down_list,
                          DatagramIterator &scan, BamReader *manager);

    Down _down;
    Down _stashed;
    Up _up;
    Paths _paths;

    CPT(RenderState) _state;
    CPT(RenderEffects) _effects;
    CPT(TransformState) _transform;
    CPT(TransformState) _prev_transform;

    TagData _tag_data;

    // This is the draw_mask of this particular node.
    DrawMask _draw_mask;

    // This is the mask that indicates which CollisionNodes may detect
    // a collision with this particular node.  By default it is zero
    // for an ordinary PandaNode, and all bits on for a CollisionNode
    // or GeomNode.
    CollideMask _into_collide_mask;

    // This is the union of all into_collide_mask bits for any nodes
    // at and below this level.  It's conceptually similar to a
    // bounding volume--it represents the bounding volume of this node
    // in the space of collision bits--and it needs to be updated for
    // the same reasons the bounding volume needs to be updated.  So
    // we update them together.
    CollideMask _net_collide_mask;

    bool _fixed_internal_bound;
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
    INLINE void operator = (const Children &copy);

    INLINE int get_num_children() const;
    INLINE PandaNode *get_child(int n) const;

  private:
    CDReader _cdata;
  };

  INLINE Children get_children() const;

  // This interface *does* protect you from self-modifying loops, by
  // copying the list of children.
  class EXPCL_PANDA ChildrenCopy {
  public:
    ChildrenCopy(const CDReader &cdata);
    INLINE ChildrenCopy(const ChildrenCopy &copy);
    INLINE void operator = (const ChildrenCopy &copy);

    INLINE int get_num_children() const;
    INLINE PandaNode *get_child(int n) const;

  private:
    typedef PTA(PT(PandaNode)) List;
    List _list;
  };

  INLINE ChildrenCopy get_children_copy() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  void write_recorder(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  void fillin_recorder(DatagramIterator &scan, BamReader *manager);
  
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
  friend class NodePath;
  friend class NodePathComponent;
  friend class WorkingNodePath;
};

INLINE ostream &operator << (ostream &out, const PandaNode &node) {
  node.output(out);
  return out;
}

#include "pandaNode.I"

#endif

