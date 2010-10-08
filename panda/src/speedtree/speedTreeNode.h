// Filename: speedTreeNode.h
// Created by:  drose (30Sep10)
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

#ifndef SPEEDTREENODE_H
#define SPEEDTREENODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "stTree.h"
#include "stTransform.h"
#include "callbackObject.h"
#include "loaderOptions.h"

#include "speedtree_api.h"

class Loader;

// There is a SpeedTree bug that prevents reliably deleting a
// CForestRender object, as of version 5.2.  Presumably it will be
// fixed beginning in version 5.3.
#if SPEEDTREE_VERSION_MAJOR > 5 || (SPEEDTREE_VERSION_MAJOR == 5 && SPEEDTREE_VERSION_MINOR >= 3)
#undef ST_DELETE_FOREST_HACK
#else
#define ST_DELETE_FOREST_HACK
#endif

////////////////////////////////////////////////////////////////////
//       Class : SpeedTreeNode
// Description : Interfaces with the SpeedTree library to render
//               SpeedTree objects like a collection of trees,
//               terrain, or grass within the Panda3D scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDASPEEDTREE SpeedTreeNode : public PandaNode {
private:
  // This definition is required by InstanceList, below.
  typedef pvector<SpeedTree::CInstance> STInstances;

PUBLISHED:
  // This nested class keeps a linear list of transforms, for the
  // purpose of recording instances of a particular STTree.  It is
  // used below.
  class InstanceList {
  public:
    INLINE InstanceList(const STTree *tree);
    INLINE bool operator < (const InstanceList &other) const;

  PUBLISHED:
    INLINE const STTree *get_tree() const;

    INLINE int get_num_instances() const;
    INLINE STTransform get_instance(int n) const;
    MAKE_SEQ(get_instances, get_num_instances, get_instance);

    INLINE int add_instance(const STTransform &transform);
    INLINE void remove_instance(int n);

    void output(ostream &out) const;
    void write(ostream &out, int indent_level = 0) const;

  private:
    PT(STTree) _tree;
    STInstances _instances;
    friend class SpeedTreeNode;
  };

PUBLISHED:
  SpeedTreeNode(const string &name);
  virtual ~SpeedTreeNode();

  INLINE bool is_valid() const;

  INLINE int get_num_trees() const;
  INLINE const STTree *get_tree(int n) const;
  MAKE_SEQ(get_trees, get_num_trees, get_tree);
  const InstanceList &get_instance_list(int n) const;
  MAKE_SEQ(get_instance_lists, get_num_trees, get_instance_list);
  INLINE STTree *modify_tree(int n);

  int count_total_instances() const;

  InstanceList &add_tree(const STTree *tree);
  int remove_tree(const STTree *tree);
  void remove_all_trees();

  bool has_instance_list(const STTree *tree) const;
  const InstanceList &get_instance_list(const STTree *tree) const;
  InstanceList &modify_instance_list(const STTree *tree);

  void add_instance(const STTree *tree, const STTransform &transform);
  void add_instances(const NodePath &root, const TransformState *transform = TransformState::make_identity());
  bool add_from_stf(const Filename &pathname, 
		    const LoaderOptions &options = LoaderOptions());
  bool add_from_stf(istream &in, const Filename &pathname, 
		    const LoaderOptions &options = LoaderOptions(),
		    Loader *loader = NULL);

  static bool authorize(const string &license = "");

public:
  SpeedTreeNode(const SpeedTreeNode &copy);

  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data);

  void prepare_scene(GraphicsStateGuardianBase *gsgbase, const RenderState *net_state);

  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level) const;

private:
  void init_node();
  void r_add_instances(PandaNode *node, const TransformState *transform,
		       Thread *current_thread);

  void repopulate();
  bool validate_api(GraphicsStateGuardian *gsg);
  void draw_callback(CallbackData *cbdata);
  void setup_for_render(GraphicsStateGuardian *gsg);
  void print_forest_stats(const SpeedTree::CForest::SPopulationStats &forest_stats) const;

private:
  class DrawCallback : public CallbackObject {
  public:
    ALLOC_DELETED_CHAIN(DrawCallback);
    INLINE DrawCallback(SpeedTreeNode *node);
    virtual void do_callback(CallbackData *cbdata);

  private:
    PT(SpeedTreeNode) _node;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      CallbackObject::init_type();
      register_type(_type_handle, "SpeedTreeNode::DrawCallback",
		    CallbackObject::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

private:
  // A list of instances per each unique tree.
  typedef ov_set<InstanceList *, IndirectLess<InstanceList> > Trees;
  Trees _trees;

#ifdef ST_DELETE_FOREST_HACK
  SpeedTree::CForestRender &_forest;
#else
  SpeedTree::CForestRender _forest;
#endif  // ST_DELETE_FOREST_HACK
  SpeedTree::CView _view;
  SpeedTree::SForestCullResultsRender _visible_trees;
  SpeedTree::CForest::SPopulationStats _population_stats;
  bool _needs_repopulate;
  bool _is_valid;

  static bool _authorized;
  static bool _done_first_init;

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
    register_type(_type_handle, "SpeedTreeNode",
                  PandaNode::get_class_type());
    DrawCallback::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class SpeedTreeNode::DrawCallback;
};

#include "speedTreeNode.I"

#endif
