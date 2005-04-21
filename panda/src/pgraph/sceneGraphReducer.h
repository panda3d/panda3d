// Filename: sceneGraphReducer.h
// Created by:  drose (14Mar02)
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

#ifndef SCENEGRAPHREDUCER_H
#define SCENEGRAPHREDUCER_H

#include "pandabase.h"
#include "transformState.h"
#include "renderAttrib.h"
#include "renderState.h"
#include "accumulatedAttribs.h"
#include "geomTransformer.h"

#include "typedObject.h"
#include "pointerTo.h"

class PandaNode;

///////////////////////////////////////////////////////////////////
//       Class : SceneGraphReducer
// Description : An interface for simplifying ("flattening") scene
//               graphs by eliminating unneeded nodes and collapsing
//               out unneeded state changes and transforms.
//
//               This class is designed so that it may be inherited
//               from and specialized, if needed, to fine-tune the
//               flattening behavior, but normally the default
//               behavior is sufficient.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SceneGraphReducer {
PUBLISHED:
  INLINE SceneGraphReducer();
  INLINE ~SceneGraphReducer();

  enum AttribTypes {
    TT_transform       = 0x001,
    TT_color           = 0x002,
    TT_color_scale     = 0x004,
    TT_tex_matrix      = 0x008,
    TT_other           = 0x010,
  };

  enum CombineSiblings {
    CS_other           = 0x001,
    CS_geom_node       = 0x002,
    CS_recurse         = 0x004,
  };

  enum CollectVertexData {
    // If set, two GeomVertexDatas with different names will not be
    // collected together.
    CVD_name           = 0x001,

    // If set, a ModelNode begins a subgraph of nodes whose
    // GeomVertexDatas will not be collected with nodes outside the
    // subgraph.
    CVD_model          = 0x002,

    // If set, a non-identity transform begins a subgraph of nodes
    // whose GeomVertexDatas will not be collected with nodes outside
    // the subgraph.
    CVD_transform      = 0x004,

    // If set, GeomVertexDatas with any usage_hint other than
    // UH_static will not be collected with any other Geoms in a
    // different GeomNode.  However, two different dynamic Geoms
    // within the same node might still be collected together.
    CVD_avoid_dynamic  = 0x008,

    // If set, only those GeomVertexDatas within the same node might
    // be collected together.
    CVD_one_node_only  = 0x010,
  };

  enum MakeNonindexed {
    // If set, only composite primitives such as tristrips and trifans
    // will be made nonindexed; simple primitives such as triangles
    // will be left indexed.
    MN_composite_only  = 0x001,

    // If set any GeomVertexData with any animation indication will
    // not be adjusted, whether the animation is to be performed on
    // the CPU or on the graphics pipe.
    MN_avoid_animated  = 0x002,

    // If set, any GeomVertexData or Geom with a usage_hint other than
    // UH_static will not be made nonindexed.
    MN_avoid_dynamic   = 0x004,
  };

  INLINE void set_usage_hint(qpGeom::UsageHint usage_hint);
  INLINE qpGeom::UsageHint get_usage_hint() const;

  INLINE void apply_attribs(PandaNode *node, int attrib_types = ~0);
  INLINE void apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
                            int attrib_types, GeomTransformer &transformer);

  int flatten(PandaNode *root, int combine_siblings_bits);

  INLINE int collect_vertex_data(PandaNode *root, int collect_bits = ~0);
  INLINE int make_nonindexed(PandaNode *root, int nonindexed_bits = ~0);
  INLINE void unify(PandaNode *root);

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

  int r_collect_vertex_data(PandaNode *node, int collect_bits,
                            GeomTransformer &transformer);
  int r_make_nonindexed(PandaNode *node, int collect_bits);
  void r_unify(PandaNode *node);

private:
  GeomTransformer _transformer;
};

#include "sceneGraphReducer.I"

#endif
