// Filename: multitexReducer.h
// Created by:  drose (30Nov04)
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

#ifndef MULTITEXREDUCER_H
#define MULTITEXREDUCER_H

#include "pandabase.h"
#include "texture.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "texMatrixAttrib.h"
#include "transformState.h"
#include "geomNode.h"
#include "nodePath.h"
#include "luse.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pvector.h"

class GraphicsOutput;
class PandaNode;
class RenderState;
class TransformState;

////////////////////////////////////////////////////////////////////
//       Class : MultitexReducer
// Description : This object presents an interface for generating new
//               texture images that represent the combined images
//               from one or more individual textures, reproducing 
//               certain kinds of multitexture effects without
//               depending on multitexture support in the hardware.
//
//               This also flattens out texture matrices and removes
//               extra texture coordinates from the Geoms.  It is thus
//               not a complete substitute for true multitexturing,
//               because it does not lend itself well to dynamic
//               animation of the textures once they have been
//               flattened.  It is, however, useful for "baking in" a
//               particular multitexture effect.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL MultitexReducer {
PUBLISHED:
  MultitexReducer();
  ~MultitexReducer();

  void clear();
  INLINE void scan(const NodePath &node);
  INLINE void scan(const NodePath &node, const NodePath &state_from);
  void scan(PandaNode *node, const RenderState *state, 
            const TransformState *transform);

  void set_target(TextureStage *stage);
  void set_use_geom(bool use_geom);
  void set_allow_tex_mat(bool allow_tex_mat);

  void flatten(GraphicsOutput *window);

private:
  class StageInfo {
  public:
    StageInfo(TextureStage *stage, const TextureAttrib *ta, 
              const TexMatrixAttrib *tma);

    INLINE bool operator < (const StageInfo &other) const;

    PT(TextureStage) _stage;
    PT(Texture) _tex;
    CPT(TransformState) _tex_mat;
  };

  typedef pvector<StageInfo> StageList;

  class GeomInfo {
  public:
    INLINE GeomInfo(const RenderState *state, const RenderState *geom_net_state,
                    GeomNode *geom_node, int index);

    CPT(RenderState) _state;
    CPT(RenderState) _geom_net_state;
    PT(GeomNode) _geom_node;
    int _index;
  };

  typedef pvector<GeomInfo> GeomList;

  class GeomNodeInfo {
  public:
    INLINE GeomNodeInfo(const RenderState *state, GeomNode *geom_node);
    CPT(RenderState) _state;
    PT(GeomNode) _geom_node;
  };

  typedef pvector<GeomNodeInfo> GeomNodeList;

  typedef pmap<StageList, GeomList> Stages;
  Stages _stages;
  GeomNodeList _geom_node_list;

  PT(TextureStage) _target_stage;
  bool _use_geom;
  bool _allow_tex_mat;

private:
  void scan_geom_node(GeomNode *node, const RenderState *state, 
                      const TransformState *transform);

  void record_stage_list(const StageList &stage_list, 
                         const GeomInfo &geom_info);

  size_t choose_model_stage(const StageList &stage_list) const;
  bool determine_uv_range(LTexCoord &min_uv, LTexCoord &max_uv,
                          const StageInfo &model_stage,
                          const GeomList &geom_list) const;

  void get_uv_scale(LVecBase2 &uv_scale, LVecBase2 &uv_trans,
                    const LTexCoord &min_uv, const LTexCoord &max_uv) const;

  void choose_texture_size(int &x_size, int &y_size,
                           const StageInfo &model_stage,
                           const LVecBase2 &uv_scale,
                           GraphicsOutput *window) const;

  void make_texture_layer(const NodePath &render, 
                          const StageInfo &stage_info, 
                          const GeomList &geom_list,
                          const LTexCoord &min_uv, const LTexCoord &max_uv,
                          bool force_use_geom, bool transparent_base);
  void transfer_geom(GeomNode *geom_node, const InternalName *texcoord_name,
                     const GeomList &geom_list, bool preserve_color);

  void scan_color(const GeomList &geom_list, LColor &geom_color, 
                  int &num_colors) const;
  bool scan_decal(const StageList &stage_list) const;

};

#include "multitexReducer.I"

#endif
