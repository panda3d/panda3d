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
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "renderState.h"
#include "renderEffects.h"
#include "transformState.h"
#include "drawMask.h"
#include "typedWritable.h"
#include "collideMask.h"
#include "namable.h"
#include "referenceCount.h"
#include "luse.h"
#include "ordered_vector.h"
#include "pointerTo.h"
#include "nodePointerTo.h"
#include "pointerToArray.h"
#include "pnotify.h"
#include "updateSeq.h"

#ifdef HAVE_PYTHON

#undef HAVE_LONG_LONG  // NSPR and Python both define this.
#undef _POSIX_C_SOURCE
#include <Python.h>

#endif  // HAVE_PYTHON

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
  virtual bool is_renderable() const;

  INLINE void compose_draw_mask(DrawMask &running_draw_mask) const;
  INLINE bool compare_draw_mask(DrawMask running_draw_mask,
                                DrawMask camera_mask) const;

PUBLISHED:
  PT(PandaNode) copy_subgraph() const;

  INLINE int get_num_parents() const;
  INLINE PandaNode *get_parent(int n) const;
  INLINE int find_parent(PandaNode *node) const;

  INLINE int get_num_children() const;
  INLINE PandaNode *get_child(int n) const;
  INLINE int get_child_sort(int n) const;
  INLINE int find_child(PandaNode *node) const;

  void add_child(PandaNode *child_node, int sort = 0);
  void remove_child(int child_index);
  bool remove_child(PandaNode *child_node);
  bool replace_child(PandaNode *orig_child, PandaNode *new_child);

  INLINE bool stash_child(PandaNode *child_node);
  void stash_child(int child_index);
  INLINE bool unstash_child(PandaNode *child_node);
  void unstash_child(int stashed_index);

  INLINE int get_num_stashed() const;
  INLINE PandaNode *get_stashed(int n) const;
  INLINE int get_stashed_sort(int n) const;
  INLINE int find_stashed(PandaNode *node) const;

  void add_stashed(PandaNode *child_node, int sort = 0);
  void remove_stashed(int child_index);

  void remove_all_children();
  void steal_children(PandaNode *other);
  void copy_children(PandaNode *other);

  void set_attrib(const RenderAttrib *attrib, int override = 0);
  INLINE const RenderAttrib *get_attrib(TypeHandle type) const;
  INLINE bool has_attrib(TypeHandle type) const;
  void clear_attrib(TypeHandle type);

  void set_effect(const RenderEffect *effect);
  INLINE const RenderEffect *get_effect(TypeHandle type) const;
  INLINE bool has_effect(TypeHandle type) const;
  void clear_effect(TypeHandle type);

  void set_state(const RenderState *state);
  INLINE const RenderState *get_state() const;
  INLINE void clear_state();

  void set_effects(const RenderEffects *effects);
  INLINE const RenderEffects *get_effects() const;
  INLINE void clear_effects();

  void set_transform(const TransformState *transform);
  INLINE const TransformState *get_transform() const;
  INLINE void clear_transform();

  void set_prev_transform(const TransformState *transform);
  INLINE const TransformState *get_prev_transform() const;
  void reset_prev_transform();

  void set_tag(const string &key, const string &value);
  INLINE string get_tag(const string &key) const;
  INLINE bool has_tag(const string &key) const;
  void clear_tag(const string &key);

#ifdef HAVE_PYTHON
  void set_python_tag(const string &key, PyObject *value);
  PyObject *get_python_tag(const string &key) const;
  bool has_python_tag(const string &key) const;
  void clear_python_tag(const string &key);
#endif  // HAVE_PYTHON

  INLINE bool has_tags() const;
  void copy_tags(PandaNode *other);
  void list_tags(ostream &out, const string &separator = "\n") const;

  INLINE static DrawMask get_overall_bit();
  INLINE bool is_overall_hidden() const;
  INLINE void set_overall_hidden(bool overall_hidden);

  void adjust_draw_mask(DrawMask show_mask,
                        DrawMask hide_mask,
                        DrawMask clear_mask);
  INLINE DrawMask get_draw_control_mask() const;
  INLINE DrawMask get_draw_show_mask() const;

  CollideMask get_net_draw_control_mask() const;
  CollideMask get_net_draw_show_mask() const;

  void set_into_collide_mask(CollideMask mask);
  INLINE CollideMask get_into_collide_mask() const;
  virtual CollideMask get_legal_collide_mask() const;

  CollideMask get_net_collide_mask() const;
  CPT(RenderAttrib) get_off_clip_planes() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

  INLINE void ls(ostream &out, int indent_level) const;

  // A node has three bounding volumes: an "external" bounding volume
  // that represents the node and all of its children, an "internal"
  // bounding volume which represents only the node itself (and is
  // usually empty, unless a specific node type sets it otherwise),
  // and a "user" bounding volume which is specified by the user.

  // We define set_bounds() and get_bounds() functions so that
  // set_bounds() sets the user bounding volume, while get_bounds()
  // returns the external bounding volume.  Although it might seem
  // strange and confusing to do this, this is actually the natural
  // way the user thinks about nodes and bounding volumes.
  void set_bounds(const BoundingVolume *volume);
  void set_bound(const BoundingVolume *volume);
  INLINE void clear_bounds();
  CPT(BoundingVolume) get_bounds() const;
  INLINE CPT(BoundingVolume) get_internal_bounds() const;

  void mark_bounds_stale() const;

  INLINE void set_final(bool flag);
  INLINE bool is_final() const;

  virtual bool is_geom_node() const;
  virtual bool is_lod_node() const;
  virtual Light *as_light();

protected:
  INLINE CPT(BoundingVolume) get_user_bounds(int pipeline_stage) const;
  CPT(BoundingVolume) get_internal_bounds(int pipeline_stage) const;
  void set_internal_bounds(const BoundingVolume *volume);

  INLINE void mark_bounds_stale(int pipeline_stage) const;
  void force_bounds_stale();
  void force_bounds_stale(int pipeline_stage);
  void mark_internal_bounds_stale();
  INLINE void mark_internal_bounds_stale(int pipeline_stage);

  virtual PT(BoundingVolume) compute_internal_bounds(int pipeline_stage) const;
  virtual void parents_changed();
  virtual void children_changed();
  virtual void transform_changed();
  virtual void state_changed();
  virtual void draw_mask_changed();

  typedef pmap<PandaNode *, PandaNode *> InstanceMap;
  virtual PT(PandaNode) r_copy_subgraph(InstanceMap &inst_map) const;
  virtual void r_copy_children(const PandaNode *from, InstanceMap &inst_map);

private:
  class CDataLight;
  class CDataHeavy;
  class CDataBounds;
  class CDataLinks;

  INLINE int do_find_parent(PandaNode *node, const CDataLinks *cdata) const;
  int do_find_child(PandaNode *node, const CDataLinks *cdata) const;
  int do_find_stashed(PandaNode *node, const CDataLinks *cdata) const;
  bool stage_remove_child(PandaNode *child_node, int pipeline_stage);
  bool stage_replace_child(PandaNode *orig_child, PandaNode *new_child,
                           int pipeline_stage);

  // parent-child manipulation for NodePath support.  Don't try to
  // call these directly.
  static PT(NodePathComponent) attach(NodePathComponent *parent, 
				      PandaNode *child, int sort,
				      int pipeline_stage);
  static void detach(NodePathComponent *child, int pipeline_stage);
  static void detach_one_stage(NodePathComponent *child, int pipeline_stage);
  static bool reparent(NodePathComponent *new_parent,
                       NodePathComponent *child, int sort, bool as_stashed,
		       int pipeline_stage);
  static bool reparent_one_stage(NodePathComponent *new_parent,
				 NodePathComponent *child, int sort, 
				 bool as_stashed, int pipeline_stage);
  static PT(NodePathComponent) get_component(NodePathComponent *parent,
					     PandaNode *child,
					     int pipeline_stage);
  static PT(NodePathComponent) get_top_component(PandaNode *child, bool force,
						 int pipeline_stage);
  PT(NodePathComponent) get_generic_component(bool accept_ambiguity,
					      int pipeline_stage);
  PT(NodePathComponent) r_get_generic_component(bool accept_ambiguity, 
						bool &ambiguity_detected,
						int pipeline_stage);
  void delete_component(NodePathComponent *component);
  static void sever_connection(PandaNode *parent_node, PandaNode *child_node,
                               int pipeline_stage);
  static void new_connection(PandaNode *parent_node, PandaNode *child_node,
                             int pipeline_stage);
  void fix_path_lengths(int pipeline_stage);
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
  typedef phash_set<NodePathComponent *, pointer_hash> Paths;

  // We don't cycle the set of Paths, since these are across all
  // threads.  A NodePathComponent, once created, is always associated
  // with the same node.  We do, however, protect the Paths under a
  // mutex.
  Paths _paths;
  Mutex _paths_lock;

  // This is used to maintain a table of keyed data on each node, for
  // the user's purposes.
  typedef phash_map<string, string, string_hash> TagData;
#ifdef HAVE_PYTHON
  typedef phash_map<string, PyObject *, string_hash> PythonTagData;
#endif  // HAVE_PYTHON

  
  // This is the data that must be cycled between pipeline stages.  We
  // store it in several different CData objects, in an attempt to
  // minimize overhead caused by cycling data unnecessarily.  The
  // things that are likely to change often (and at the same time) are
  // grouped into the same CData object; things that change less often
  // are grouped into a different one.

  // The CDataLight object stores the lightweight parts of the node
  // that are likely to change fairly often: transform and stage.
  class EXPCL_PANDA CDataLight : public CycleData {
  public:
    INLINE CDataLight();
    CDataLight(const CDataLight &copy);
    virtual ~CDataLight();

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return PandaNode::get_class_type();
    }

    NCPT(RenderState) _state;
    NCPT(TransformState) _transform;
    NCPT(TransformState) _prev_transform;
  };

  PipelineCycler<CDataLight> _cycler_light;
  typedef CycleDataReader<CDataLight> CDLightReader;
  typedef CycleDataWriter<CDataLight> CDLightWriter;
  typedef CycleDataStageReader<CDataLight> CDLightStageReader;
  typedef CycleDataStageWriter<CDataLight> CDLightStageWriter;

  // The CDataHeavy object stores the heavierweight parts of the node
  // that are less likely to change as often: tags, collide mask.
  class EXPCL_PANDA CDataHeavy : public CycleData {
  public:
    INLINE CDataHeavy();
    CDataHeavy(const CDataHeavy &copy);
    virtual ~CDataHeavy();

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return PandaNode::get_class_type();
    }

#ifdef HAVE_PYTHON
    void inc_py_refs();
    void dec_py_refs();
#endif

    CPT(RenderEffects) _effects;

    TagData _tag_data;
#ifdef HAVE_PYTHON
    PythonTagData _python_tag_data;
#endif  // HAVE_PYTHON

    // These two together determine the per-camera visibility of this
    // node.  See adjust_draw_mask() for details.
    DrawMask _draw_control_mask, _draw_show_mask;

    // This is the mask that indicates which CollisionNodes may detect
    // a collision with this particular node.  By default it is zero
    // for an ordinary PandaNode, and all bits on for a CollisionNode
    // or GeomNode.
    CollideMask _into_collide_mask;

    // This is the user bounding volume, which is only specified by a
    // user.  It defaults to NULL, which means an empty volume.
    CPT(BoundingVolume) _user_bounds;

    // This is the "internal" bounding volume, which is normally
    // empty, but which a particular PandaNode subclass may define to
    // be any arbitrary volume, by calling set_internal_bounds() or by
    // overriding compute_internal_bounds().
    CPT(BoundingVolume) _internal_bounds;
    bool _internal_bounds_stale;

    // This is true if the external bounds of this node should be
    // deemed "final".  See set_final().
    bool _final_bounds;
  };

  PipelineCycler<CDataHeavy> _cycler_heavy;
  typedef CycleDataReader<CDataHeavy> CDHeavyReader;
  typedef CycleDataWriter<CDataHeavy> CDHeavyWriter;
  typedef CycleDataStageReader<CDataHeavy> CDHeavyStageReader;
  typedef CycleDataStageWriter<CDataHeavy> CDHeavyStageWriter;

  // The CDataBounds object stores the data that is accumulated upward
  // from the node's children: that is, the external bounding volume,
  // and conceptually similar things like the net_collide_mask, etc.
  // None of the data in this object is preserved in a bam file.
  class EXPCL_PANDA CDataBounds : public CycleData {
  public:
    INLINE CDataBounds();
    CDataBounds(const CDataBounds &copy);
    virtual ~CDataBounds();

    virtual CycleData *make_copy() const;
    virtual TypeHandle get_parent_type() const {
      return PandaNode::get_class_type();
    }

    // This is the union of all into_collide_mask bits for any nodes
    // at and below this level.
    CollideMask _net_collide_mask;

    // These are similar, for the draw mask.
    DrawMask _net_draw_control_mask, _net_draw_show_mask;

    // This is a ClipPlaneAttrib that represents the union of all clip
    // planes that have been turned *off* at and below this level.
    // TODO: fix the circular reference counts involved here.
    CPT(RenderAttrib) _off_clip_planes;

    // This is the bounding volume around the _user_bounds, the
    // _internal_bounds, and all of the children's external bounding
    // volumes.
    CPT(BoundingVolume) _external_bounds;

    // When _last_update != _next_update, this cache is stale.
    UpdateSeq _last_update, _next_update;
  };

  PipelineCycler<CDataBounds> _cycler_bounds;
  typedef CycleDataReader<CDataBounds> CDBoundsReader;
  typedef CycleDataWriter<CDataBounds> CDBoundsWriter;
  typedef CycleDataStageReader<CDataBounds> CDBoundsStageReader;
  typedef CycleDataStageWriter<CDataBounds> CDBoundsStageWriter;

  // The CDataLinks object stores the links to other nodes above and
  // below this node in the graph.
  class EXPCL_PANDA CDataLinks : public CycleData {
  public:
    INLINE CDataLinks();
    CDataLinks(const CDataLinks &copy);
    virtual ~CDataLinks();

    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return PandaNode::get_class_type();
    }

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
  };

  PipelineCycler<CDataLinks> _cycler_links;
  typedef CycleDataReader<CDataLinks> CDLinksReader;
  typedef CycleDataWriter<CDataLinks> CDLinksWriter;
  typedef CycleDataStageReader<CDataLinks> CDLinksStageReader;
  typedef CycleDataStageWriter<CDataLinks> CDLinksStageWriter;

  CDBoundsStageWriter update_bounds(int pipeline_stage, 
				    CDBoundsStageReader &cdata);

  static DrawMask _overall_bit;

public:
  // This class is returned from get_children_copy().  Use it to walk
  // through the list of children, particularly with a self-modifying
  // loop, since the list of children is copied first and is thus safe
  // from self modification.
  class EXPCL_PANDA ChildrenCopy {
  public:
    ChildrenCopy(const CDLinksReader &cdata);
    INLINE ChildrenCopy(const ChildrenCopy &copy);
    INLINE void operator = (const ChildrenCopy &copy);

    INLINE int get_num_children() const;
    INLINE PandaNode *get_child(int n) const;

  private:
    typedef PTA(PT(PandaNode)) List;
    List _list;
  };

  // This class is returned from get_children().  Use this interface
  // when you want to walk through the list of children with less
  // overhead than get_children_copy(); in single-threaded Panda, a
  // copy is not made first, so this interface is fast.  However, this
  // does not protect you from self-modifying loops.  Also, since the
  // lock is kept open the whole time, this is generally unsafe to use
  // in threaded code (which would then hold the lock on the root node
  // during an entire traversal), so when pipelining is enabled,
  // get_children() returns the same thing as get_children_copy().
#ifdef DO_PIPELINING
  typedef ChildrenCopy Children;
#else
  class EXPCL_PANDA Children {
  public:
    INLINE Children(const CDLinksReader &cdata);
    INLINE Children(const Children &copy);
    INLINE void operator = (const Children &copy);

    INLINE int get_num_children() const;
    INLINE PandaNode *get_child(int n) const;

  private:
    CDLinksReader _cdata;
  };
#endif  // DO_PIPELINING

  // Similarly for stashed children.
  class EXPCL_PANDA StashedCopy {
  public:
    StashedCopy(const CDLinksReader &cdata);
    INLINE StashedCopy(const StashedCopy &copy);
    INLINE void operator = (const StashedCopy &copy);

    INLINE int get_num_stashed() const;
    INLINE PandaNode *get_stashed(int n) const;

  private:
    typedef PTA(PT(PandaNode)) List;
    List _list;
  };

#ifdef DO_PIPELINING
  typedef StashedCopy Stashed;
#else
  class EXPCL_PANDA Stashed {
  public:
    INLINE Stashed(const CDLinksReader &cdata);
    INLINE Stashed(const Stashed &copy);
    INLINE void operator = (const Stashed &copy);

    INLINE int get_num_stashed() const;
    INLINE PandaNode *get_stashed(int n) const;

  private:
    CDLinksReader _cdata;
  };
#endif  // DO_PIPELINING

  // This class is returned from get_parents_copy().
  class EXPCL_PANDA ParentsCopy {
  public:
    ParentsCopy(const CDLinksReader &cdata);
    INLINE ParentsCopy(const ParentsCopy &copy);
    INLINE void operator = (const ParentsCopy &copy);

    INLINE int get_num_parents() const;
    INLINE PandaNode *get_parent(int n) const;

  private:
    typedef PTA(PT(PandaNode)) List;
    List _list;
  };

  INLINE ChildrenCopy get_children_copy() const;
  INLINE Children get_children() const;
  INLINE StashedCopy get_stashed_copy() const;
  INLINE Stashed get_stashed() const;
  INLINE ParentsCopy get_parents_copy() const;

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

#ifndef DO_PIPELINING
  friend class PandaNode::Children;
  friend class PandaNode::Stashed;
#endif
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

