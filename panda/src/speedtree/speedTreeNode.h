/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file speedTreeNode.h
 * @author drose
 * @date 2010-09-30
 */

#ifndef SPEEDTREENODE_H
#define SPEEDTREENODE_H

#include "pandabase.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "stTree.h"
#include "stTransform.h"
#include "stTerrain.h"
#include "callbackObject.h"
#include "loaderOptions.h"
#include "transformState.h"
#include "nodePath.h"
#include "pStatCollector.h"
#include "randomizer.h"
#include "speedtree_api.h"

class Loader;

// There is a SpeedTree bug that prevents reliably deleting a CForestRender
// object, as of version 5.2.  Presumably it will be fixed beginning in
// version 5.3.
#if SPEEDTREE_VERSION_MAJOR > 5 || (SPEEDTREE_VERSION_MAJOR == 5 && SPEEDTREE_VERSION_MINOR >= 3)
#undef ST_DELETE_FOREST_HACK
#else
#define ST_DELETE_FOREST_HACK
#endif

/**
 * Interfaces with the SpeedTree library to render SpeedTree objects,
 * especially trees, within the Panda3D scene graph.
 *
 * SpeedTree also includes some support for a simple terrain system, which is
 * available here as well.  SpeedTree's rather lame grass system is not
 * presently exposed.
 */
class EXPCL_PANDASPEEDTREE SpeedTreeNode : public PandaNode {
private:
  // This definition is required by InstanceList, below.
  typedef pvector<SpeedTree::CInstance> STInstances;

PUBLISHED:
  // This nested class keeps a linear list of transforms, for the purpose of
  // recording instances of a particular STTree.  It is used below.
  class InstanceList {
  public:
    INLINE InstanceList(const STTree *tree);
    INLINE bool operator < (const InstanceList &other) const;

  PUBLISHED:
    INLINE const STTree *get_tree() const;

    INLINE int get_num_instances() const;
    INLINE STTransform get_instance(int n) const;
    MAKE_SEQ(get_instances, get_num_instances, get_instance);
    INLINE void set_instance(int n, const STTransform &transform);

    INLINE int add_instance(const STTransform &transform);
    INLINE void remove_instance(int n);

    void output(std::ostream &out) const;
    void write(std::ostream &out, int indent_level = 0) const;

  public:
    void write_datagram(BamWriter *manager, Datagram &dg);
    void fillin(DatagramIterator &scan, BamReader *manager);

  private:
    PT(STTree) _tree;
    STInstances _instances;
    friend class SpeedTreeNode;
  };

PUBLISHED:
  explicit SpeedTreeNode(const std::string &name);
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
  void add_instances_from(const SpeedTreeNode *other);
  void add_instances_from(const SpeedTreeNode *other, const TransformState *transform);
  void add_random_instances(const STTree *tree, int quantity,
                            PN_stdfloat x_min, PN_stdfloat x_max,
                            PN_stdfloat y_min, PN_stdfloat y_max,
                            PN_stdfloat scale_min, PN_stdfloat scale_max,
                            PN_stdfloat height_min, PN_stdfloat height_max,
                            PN_stdfloat slope_min, PN_stdfloat slope_max,
                            Randomizer &randomizer = Randomizer());

  bool add_from_stf(const Filename &stf_filename,
                    const LoaderOptions &options = LoaderOptions());
  bool add_from_stf(std::istream &in, const Filename &pathname,
                    const LoaderOptions &options = LoaderOptions(),
                    Loader *loader = nullptr);

  bool setup_terrain(const Filename &terrain_file);
  void set_terrain(STTerrain *terrain);
  INLINE void clear_terrain();
  INLINE bool has_terrain() const;
  INLINE STTerrain *get_terrain() const;

  void snap_to_terrain();

  void reload_config();

  void set_wind(double strength, const LVector3 &direction);

  INLINE void set_time_delta(double delta);
  INLINE double get_time_delta() const;
  INLINE static void set_global_time_delta(double delta);
  INLINE static double get_global_time_delta();
  MAKE_PROPERTY(time_delta, get_time_delta, set_time_delta);
  MAKE_PROPERTY(global_time_delta, get_global_time_delta,
                                   set_global_time_delta);

  static bool authorize(const std::string &license = "");

public:
  SpeedTreeNode(const SpeedTreeNode &copy);

  virtual PandaNode *make_copy() const;
  virtual PandaNode *combine_with(PandaNode *other);
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data);

  void prepare_scene(GraphicsStateGuardianBase *gsgbase, const RenderState *net_state);

  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  static void write_error(std::ostream &out);

protected:
  void set_transparent_texture_mode(SpeedTree::ETextureAlphaRenderMode eMode) const;

private:
  void init_node();
  void r_add_instances(PandaNode *node, const TransformState *transform,
                       Thread *current_thread);

  void repopulate();
  void update_terrain_cells();
  bool validate_api(GraphicsStateGuardian *gsg);
  void draw_callback(CallbackData *cbdata);
  void render_forest_into_shadow_maps();
  void setup_for_render(GraphicsStateGuardian *gsg);
  void cull_forest();

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
  std::string _os_shaders_dir;

  // A list of instances per each unique tree.
  typedef ov_set<InstanceList *, IndirectLess<InstanceList> > Trees;
  Trees _trees;

#ifdef ST_DELETE_FOREST_HACK
  SpeedTree::CForestRender &_forest_render;
#else
  SpeedTree::CForestRender _forest_render;
#endif  // ST_DELETE_FOREST_HACK
  SpeedTree::CView _view;
  SpeedTree::SForestCullResultsRender _visible_trees;
  SpeedTree::CForest::SPopulationStats _population_stats;
  bool _needs_repopulate;
  bool _is_valid;

  PT(STTerrain) _terrain;
  SpeedTree::CTerrainRender _terrain_render;
  SpeedTree::STerrainCullResults _visible_terrain;

  SpeedTree::Vec3 _light_dir;

  class ShadowInfo {
  public:
    ShadowInfo() {};

    SpeedTree::CView _light_view;
    SpeedTree::SForestCullResultsRender _light_cull;
    PN_stdfloat _shadow_split;
  };
  typedef pvector<ShadowInfo> ShadowInfos;
  ShadowInfos _shadow_infos;

  double _time_delta;
  static double _global_time_delta;

  static bool _authorized;
  static bool _done_first_init;

  static PStatCollector _cull_speedtree_pcollector;
  static PStatCollector _cull_speedtree_shadows_pcollector;
  static PStatCollector _cull_speedtree_trees_pcollector;
  static PStatCollector _cull_speedtree_terrain_pcollector;
  static PStatCollector _draw_speedtree_pcollector;
  static PStatCollector _draw_speedtree_shadows_pcollector;
  static PStatCollector _draw_speedtree_trees_pcollector;
  static PStatCollector _draw_speedtree_terrain_pcollector;
  static PStatCollector _draw_speedtree_terrain_update_pcollector;

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

INLINE std::ostream &operator << (std::ostream &out, const SpeedTreeNode::InstanceList &instances) {
  instances.output(out);
  return out;
}

#include "speedTreeNode.I"

#endif
