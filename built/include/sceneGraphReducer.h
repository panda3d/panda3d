/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphReducer.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef SCENEGRAPHREDUCER_H
#define SCENEGRAPHREDUCER_H

#include "pandabase.h"
#include "transformState.h"
#include "renderAttrib.h"
#include "renderState.h"
#include "accumulatedAttribs.h"
#include "geomTransformer.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "typedObject.h"
#include "pointerTo.h"
#include "graphicsStateGuardianBase.h"

class PandaNode;

/**
 * An interface for simplifying ("flattening") scene graphs by eliminating
 * unneeded nodes and collapsing out unneeded state changes and transforms.
 *
 * This class is designed so that it may be inherited from and specialized, if
 * needed, to fine-tune the flattening behavior, but normally the default
 * behavior is sufficient.
 */
class EXPCL_PANDA_PGRAPH SceneGraphReducer {
PUBLISHED:
  INLINE explicit SceneGraphReducer(GraphicsStateGuardianBase *gsg = nullptr);
  INLINE ~SceneGraphReducer();

  enum AttribTypes {
    TT_transform           = 0x001,
    TT_color               = 0x002,
    TT_color_scale         = 0x004,
    TT_tex_matrix          = 0x008,
    TT_clip_plane          = 0x010,
    TT_cull_face           = 0x020,
    TT_apply_texture_color = 0x040,
    TT_other               = 0x080,
  };

  enum CombineSiblings {
    CS_geom_node       = 0x001,
    CS_within_radius   = 0x002,
    CS_other           = 0x004,
    CS_recurse         = 0x008,
  };

  enum CollectVertexData {
    // If set, two GeomVertexDatas with different names will not be collected
    // together.
    CVD_name           = 0x001,

    // If set, a ModelNode begins a subgraph of nodes whose GeomVertexDatas
    // will not be collected with nodes outside the subgraph.
    CVD_model          = 0x002,

    // If set, a non-identity transform begins a subgraph of nodes whose
    // GeomVertexDatas will not be collected with nodes outside the subgraph.
    CVD_transform      = 0x004,

    // If set, GeomVertexDatas with any usage_hint other than UH_static will
    // not be collected with any other Geoms in a different GeomNode.
    // However, two different dynamic Geoms within the same node might still
    // be collected together.
    CVD_avoid_dynamic  = 0x008,

    // If set, only those GeomVertexDatas within the same node might be
    // collected together.
    CVD_one_node_only  = 0x010,

    // If set, two GeomVertexDatas with different formats will not be
    // collected together.  If not set, GeomVertexDatas of different formats
    // may be combined by expanding all GeomVertexDatas to the union of all
    // defined columns.
    CVD_format         = 0x020,

    // If set, two GeomVertexDatas with different usage hints (for instance,
    // UH_static vs.  UH_dynamic) will not be collected together.
    CVD_usage_hint     = 0x040,

    // If set, GeomVertexDatas with unanimated vertices will not be combined
    // with GeomVertexDatas with animated vertices.
    CVD_animation_type = 0x080,
  };

  enum MakeNonindexed {
    // If set, only composite primitives such as tristrips and trifans will be
    // made nonindexed; simple primitives such as triangles will be left
    // indexed.
    MN_composite_only  = 0x001,

    // If set any GeomVertexData with any animation indication will not be
    // adjusted, whether the animation is to be performed on the CPU or on the
    // graphics pipe.
    MN_avoid_animated  = 0x002,

    // If set, any GeomVertexData or Geom with a usage_hint other than
    // UH_static will not be made nonindexed.
    MN_avoid_dynamic   = 0x004,
  };

  void set_gsg(GraphicsStateGuardianBase *gsg);
  void clear_gsg();
  INLINE GraphicsStateGuardianBase *get_gsg() const;

  INLINE void set_combine_radius(PN_stdfloat combine_radius);
  INLINE PN_stdfloat get_combine_radius() const;

  INLINE void apply_attribs(PandaNode *node, int attrib_types = ~(TT_clip_plane | TT_cull_face | TT_apply_texture_color));
  INLINE void apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
                            int attrib_types, GeomTransformer &transformer);

  int flatten(PandaNode *root, int combine_siblings_bits);

  int remove_column(PandaNode *root, const InternalName *column);

  int make_compatible_state(PandaNode *root);

  INLINE int make_compatible_format(PandaNode *root, int collect_bits = ~0);
  void decompose(PandaNode *root);

  INLINE int collect_vertex_data(PandaNode *root, int collect_bits = ~0);
  INLINE int make_nonindexed(PandaNode *root, int nonindexed_bits = ~0);
  void unify(PandaNode *root, bool preserve_order);
  void remove_unused_vertices(PandaNode *root);

  INLINE void premunge(PandaNode *root, const RenderState *initial_state);
  bool check_live_flatten(PandaNode *node);

protected:
  void r_apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
                       int attrib_types, GeomTransformer &transformer);

  int r_flatten(PandaNode *grandparent_node, PandaNode *parent_node,
                int combine_siblings_bits);
  int flatten_siblings(PandaNode *parent_node,
                       int combine_siblings_bits);

  bool consider_child(PandaNode *grandparent_node,
                      PandaNode *parent_node, PandaNode *child_node);
  bool consider_siblings(PandaNode *parent_node, PandaNode *child1,
                         PandaNode *child2);

  bool do_flatten_child(PandaNode *grandparent_node,
                        PandaNode *parent_node, PandaNode *child_node);

  PandaNode *do_flatten_siblings(PandaNode *parent_node,
                                 PandaNode *child1, PandaNode *child2);

  PT(PandaNode) collapse_nodes(PandaNode *node1, PandaNode *node2,
                               bool siblings);
  void choose_name(PandaNode *preserve, PandaNode *source1,
                   PandaNode *source2);

  int r_remove_column(PandaNode *node, const InternalName *column,
                      GeomTransformer &transformer);

  int r_make_compatible_state(PandaNode *node, GeomTransformer &transformer);

  int r_collect_vertex_data(PandaNode *node, int collect_bits,
                            GeomTransformer &transformer, bool format_only);
  int r_make_nonindexed(PandaNode *node, int collect_bits);
  void r_unify(PandaNode *node, int max_indices, bool preserve_order);
  void r_register_vertices(PandaNode *node, GeomTransformer &transformer);
  void r_decompose(PandaNode *node);

  void r_premunge(PandaNode *node, const RenderState *state);

private:
  PT(GraphicsStateGuardianBase) _gsg;
  PN_stdfloat _combine_radius;
  GeomTransformer _transformer;

  static PStatCollector _flatten_collector;
  static PStatCollector _apply_collector;
  static PStatCollector _remove_column_collector;
  static PStatCollector _compatible_state_collector;
  static PStatCollector _collect_collector;
  static PStatCollector _make_nonindexed_collector;
  static PStatCollector _unify_collector;
  static PStatCollector _remove_unused_collector;
  static PStatCollector _premunge_collector;
};

#include "sceneGraphReducer.I"

#endif
