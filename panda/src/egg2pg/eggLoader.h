// Filename: eggLoader.h
// Created by:  drose (26Feb02)
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

#ifndef EGGLOADER_H
#define EGGLOADER_H

#include "pandabase.h"

#include "deferredNodeProperty.h"
#include "eggData.h"
#include "eggTexture.h"
#include "pt_EggTexture.h"
#include "eggGroup.h"
#include "eggMaterial.h"
#include "pt_EggMaterial.h"
#include "texture.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "builder.h"
#include "lmatrix.h"
#include "indirectCompareTo.h"
#include "textureAttrib.h"
#include "eggTransform3d.h"

class EggNode;
class EggBin;
class EggTable;
class EggNurbsCurve;
class EggNurbsSurface;
class EggPrimitive;
class EggPolygon;
class EggMaterial;
class ComputedVerticesMaker;
class RenderRelation;
class CollisionSolid;
class CollisionNode;
class CollisionPlane;
class CollisionPolygon;

///////////////////////////////////////////////////////////////////
//       Class : EggLoader
// Description : Converts an egg data structure, possibly read from an
//               egg file but not necessarily, into a scene graph
//               suitable for rendering.
//
//               This class isn't exported from this package.
////////////////////////////////////////////////////////////////////
class EggLoader {
public:
  EggLoader();
  EggLoader(const EggData &data);

  void build_graph();
  void reparent_decals();

  void make_nonindexed_primitive(EggPrimitive *egg_prim, PandaNode *parent,
                                 const LMatrix4d *transform = NULL);

  void make_indexed_primitive(EggPrimitive *egg_prim, PandaNode *parent,
                              const LMatrix4d *transform,
                              ComputedVerticesMaker &_comp_verts_maker);

private:
  class TextureDef {
  public:
    CPT(RenderAttrib) _texture;
    CPT(RenderAttrib) _apply;
  };

  void make_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                        const LMatrix4d &mat);
  void make_old_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                            const LMatrix4d &mat);
  void make_nurbs_surface(EggNurbsSurface *egg_surface, PandaNode *parent,
                          const LMatrix4d &mat);

  void load_textures();
  bool load_texture(TextureDef &def, const EggTexture *egg_tex);
  void apply_texture_attributes(Texture *tex, const EggTexture *egg_tex);
  CPT(RenderAttrib) get_texture_apply_attributes(const EggTexture *egg_tex);

  CPT(RenderAttrib) get_material_attrib(const EggMaterial *egg_mat,
                                        bool bface);

  void setup_bucket(BuilderBucket &bucket, PandaNode *parent,
                    EggPrimitive *egg_prim);

  PandaNode *make_node(EggNode *egg_node, PandaNode *parent);
  PandaNode *make_node(EggPrimitive *egg_prim, PandaNode *parent);
  PandaNode *make_node(EggBin *egg_bin, PandaNode *parent);
  PandaNode *make_node(EggGroup *egg_group, PandaNode *parent);
  PandaNode *create_group_arc(EggGroup *egg_group, PandaNode *parent,
                                   PandaNode *node);
  PandaNode *make_node(EggTable *egg_table, PandaNode *parent);
  PandaNode *make_node(EggGroupNode *egg_group, PandaNode *parent);

  void make_collision_solids(EggGroup *start_group, EggGroup *egg_group,
                             CollisionNode *cnode);
  void make_collision_plane(EggGroup *egg_group, CollisionNode *cnode,
                            EggGroup::CollideFlags flags);
  void make_collision_polygon(EggGroup *egg_group, CollisionNode *cnode,
                              EggGroup::CollideFlags flags);
  void make_collision_polyset(EggGroup *egg_group, CollisionNode *cnode,
                              EggGroup::CollideFlags flags);
  void make_collision_sphere(EggGroup *egg_group, CollisionNode *cnode,
                             EggGroup::CollideFlags flags);
  void make_collision_tube(EggGroup *egg_group, CollisionNode *cnode,
                           EggGroup::CollideFlags flags);
  void apply_collision_flags(CollisionSolid *solid,
                             EggGroup::CollideFlags flags);
  EggGroup *find_collision_geometry(EggGroup *egg_group);
  CollisionPlane *create_collision_plane(EggPolygon *egg_poly,
                                         EggGroup *parent_group);
  void create_collision_polygons(CollisionNode *cnode, EggPolygon *egg_poly,
                                 EggGroup *parent_group,
                                 EggGroup::CollideFlags flags);

  void apply_deferred_nodes(PandaNode *node, const DeferredNodeProperty &prop);
  bool expand_object_types(EggGroup *egg_group, const pset<string> &expanded,
                           const pvector<string> &expanded_history);
  bool do_expand_object_type(EggGroup *egg_group, const pset<string> &expanded,
                             const pvector<string> &expanded_history,
                             const string &object_type);

  CPT(TransformState) make_transform(const EggTransform3d *egg_transform);

  Builder _builder;

  typedef pmap<PT_EggTexture, TextureDef> Textures;
  Textures _textures;

  typedef pmap<CPT_EggMaterial, CPT(RenderAttrib) > Materials;
  Materials _materials;
  Materials _materials_bface;

  typedef pset<PandaNode *> Decals;
  Decals _decals;

  DeferredNodes _deferred_nodes;

public:
  PT(PandaNode) _root;
  EggData _data;
  bool _error;
};


#endif
