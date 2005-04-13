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
    CVD_name           = 0x001,
    CVD_model          = 0x002,
  };

  INLINE void apply_attribs(PandaNode *node, int attrib_types = ~0);
  INLINE void apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
                            int attrib_types, GeomTransformer &transformer);

  int flatten(PandaNode *root, int combine_siblings_bits);

  INLINE int collect_vertex_data(PandaNode *root, int collect_bits = ~0);

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

private:
  GeomTransformer _transformer;
};

#include "sceneGraphReducer.I"

#endif
