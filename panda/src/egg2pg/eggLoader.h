/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggLoader.h
 * @author drose
 * @date 2002-02-26
 */

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
#include "eggVertexPool.h"
#include "texture.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "lmatrix.h"
#include "indirectCompareTo.h"
#include "textureAttrib.h"
#include "textureStage.h"
#include "texGenAttrib.h"
#include "colorBlendAttrib.h"
#include "eggTransform.h"
#include "geomVertexData.h"
#include "geomPrimitive.h"
#include "bamCacheRecord.h"

class EggNode;
class EggBin;
class EggTable;
class EggNurbsCurve;
class EggNurbsSurface;
class EggPrimitive;
class EggPolygon;
class EggMaterial;
class RenderRelation;
class CollisionSolid;
class CollisionNode;
class CollisionPlane;
class CollisionPolygon;
class PortalNode;
class OccluderNode;
class PolylightNode;
class EggRenderState;
class CharacterMaker;


/**
 * Converts an egg data structure, possibly read from an egg file but not
 * necessarily, into a scene graph suitable for rendering.
 *
 * This class isn't exported from this package.
 */
class EXPCL_PANDA_EGG2PG EggLoader {
public:
  EggLoader();
  EggLoader(const EggData *data);

  void build_graph();
  void reparent_decals();
  void start_sequences();

  void make_polyset(EggBin *egg_bin, PandaNode *parent,
                    const LMatrix4d *transform, bool is_dynamic,
                    CharacterMaker *character_maker);

  CPT(TransformState) make_transform(const EggTransform *egg_transform);

private:
  class TextureDef {
  public:
    CPT(RenderAttrib) _texture;
    PT(TextureStage) _stage;
    const EggTexture *_egg_tex;
  };

  // This structure is used internally in setup_bucket().
  typedef pvector<const TextureDef *> TexMatTextures;
  typedef pmap<LMatrix3d, TexMatTextures> TexMatTransforms;
  typedef pmap<CPT(InternalName), TexMatTransforms> TexMats;

  // This structure is returned by setup_bucket().
  typedef pmap<CPT(InternalName), const EggTexture *> BakeInUVs;

  // This is used by make_primitive().
  class PrimitiveUnifier {
  public:
    INLINE PrimitiveUnifier(const GeomPrimitive *prim);
    INLINE bool operator < (const PrimitiveUnifier &other) const;

    TypeHandle _type;
    GeomPrimitive::ShadeModel _shade_model;
  };
  typedef pmap<PrimitiveUnifier, PT(GeomPrimitive) > UniquePrimitives;
  typedef pvector< PT(GeomPrimitive) > Primitives;

  void show_normals(EggVertexPool *vertex_pool, GeomNode *geom_node);

  void make_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                        const LMatrix4d &mat);
  void make_old_nurbs_curve(EggNurbsCurve *egg_curve, PandaNode *parent,
                            const LMatrix4d &mat);
  void make_nurbs_surface(EggNurbsSurface *egg_surface, PandaNode *parent,
                          const LMatrix4d &mat);

  void load_textures();
  bool load_texture(TextureDef &def, EggTexture *egg_tex);
  void apply_texture_attributes(Texture *tex, const EggTexture *egg_tex);
  Texture::CompressionMode convert_compression_mode(EggTexture::CompressionMode compression_mode) const;
  SamplerState::WrapMode convert_wrap_mode(EggTexture::WrapMode wrap_mode) const;
  PT(TextureStage) make_texture_stage(const EggTexture *egg_tex);

  void separate_switches(EggNode *egg_node);
  void emulate_bface(EggNode *egg_node);

  PandaNode *make_node(EggNode *egg_node, PandaNode *parent);
  PandaNode *make_node(EggBin *egg_bin, PandaNode *parent);
  PandaNode *make_polyset(EggBin *egg_bin, PandaNode *parent);
  PandaNode *make_lod(EggBin *egg_bin, PandaNode *parent);
  PandaNode *make_node(EggGroup *egg_group, PandaNode *parent);
  PandaNode *create_group_arc(EggGroup *egg_group, PandaNode *parent,
                                   PandaNode *node);
  PandaNode *make_node(EggTable *egg_table, PandaNode *parent);
  PandaNode *make_node(EggGroupNode *egg_group, PandaNode *parent);

  void check_for_polysets(EggGroup *egg_group, bool &all_polysets,
                          bool &any_hidden);
  PT(GeomVertexData) make_vertex_data
  (const EggRenderState *render_state, EggVertexPool *vertex_pool,
   EggNode *primitive_home, const LMatrix4d &transform, TransformBlendTable *blend_table,
   bool is_dynamic, CharacterMaker *character_maker, bool ignore_color);
  PT(TransformBlendTable) make_blend_table
  (EggVertexPool *vertex_bool, EggNode *primitive_home,
   CharacterMaker *character_maker);
  void record_morph
  (GeomVertexArrayFormat *array_format,
   CharacterMaker *character_maker, const std::string &morph_name,
   InternalName *column_name, int num_components);

  void make_primitive(const EggRenderState *render_state,
                      EggPrimitive *egg_prim,
                      UniquePrimitives &unique_primitives,
                      Primitives &primitives,
                      bool has_overall_color, const LColor &overall_color);

  void set_portal_polygon(EggGroup *egg_group, PortalNode *pnode);
  void set_occluder_polygon(EggGroup *egg_group, OccluderNode *pnode);
  PT(EggPolygon) find_first_polygon(EggGroup *egg_group);

  bool make_sphere(EggGroup *start_group, EggGroup::CollideFlags flags,
                   LPoint3 &center, PN_stdfloat &radius, LColor &color);

  bool make_box(EggGroup *start_group, EggGroup::CollideFlags flags,
                const LMatrix4 &xform, LPoint3 &min_p, LPoint3 &max_p);
  bool make_box(EggGroup *start_group, EggGroup::CollideFlags flags,
                LPoint3 &min_p, LPoint3 &max_p, LColor &color);

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
  void make_collision_box(EggGroup *egg_group, CollisionNode *cnode,
                          EggGroup::CollideFlags flags);
  void make_collision_inv_sphere(EggGroup *egg_group, CollisionNode *cnode,
                                 EggGroup::CollideFlags flags);
  void make_collision_capsule(EggGroup *egg_group, CollisionNode *cnode,
                              EggGroup::CollideFlags flags);
  void make_collision_floor_mesh(EggGroup *egg_group, CollisionNode *cnode,
                           EggGroup::CollideFlags flags);
  void apply_collision_flags(CollisionSolid *solid,
                             EggGroup::CollideFlags flags);
  EggGroup *find_collision_geometry(EggGroup *egg_group,
                                    EggGroup::CollideFlags flags);
  CollisionPlane *create_collision_plane(EggPolygon *egg_poly,
                                         EggGroup *parent_group);
  void create_collision_polygons(CollisionNode *cnode, EggPolygon *egg_poly,
                                 EggGroup *parent_group,
                                 EggGroup::CollideFlags flags);

  void create_collision_floor_mesh(CollisionNode *cnode,
                                 EggGroup *parent_group,
                                 EggGroup::CollideFlags flags);

  void apply_deferred_nodes(PandaNode *node, const DeferredNodeProperty &prop);
  bool expand_all_object_types(EggNode *egg_node);
  bool expand_object_types(EggGroup *egg_group, const pset<std::string> &expanded,
                           const pvector<std::string> &expanded_history);
  bool do_expand_object_type(EggGroup *egg_group, const pset<std::string> &expanded,
                             const pvector<std::string> &expanded_history,
                             const std::string &object_type);

  static TextureStage::CombineMode
  get_combine_mode(const EggTexture *egg_tex,
                   EggTexture::CombineChannel channel);

  static TextureStage::CombineSource
  get_combine_source(const EggTexture *egg_tex,
                     EggTexture::CombineChannel channel, int n);

  static TextureStage::CombineOperand
  get_combine_operand(const EggTexture *egg_tex,
                      EggTexture::CombineChannel channel, int n);

  static ColorBlendAttrib::Mode
  get_color_blend_mode(EggGroup::BlendMode mode);

  static ColorBlendAttrib::Operand
  get_color_blend_operand(EggGroup::BlendOperand operand);

  typedef pmap<PT_EggTexture, TextureDef> Textures;
  Textures _textures;

  typedef pmap<CPT_EggMaterial, CPT(RenderAttrib) > Materials;
  Materials _materials;
  Materials _materials_bface;

  typedef pmap<PT(EggGroup), PT(PandaNode) > Groups;
  Groups _groups;

  typedef pset<PandaNode *> ExtraNodes;
  ExtraNodes _decals;
  ExtraNodes _sequences;

  class VertexPoolTransform {
  public:
    bool operator < (const VertexPoolTransform &other) const;
    PT(EggVertexPool) _vertex_pool;
    BakeInUVs _bake_in_uvs;
    LMatrix4d _transform;
  };
  typedef pmap<VertexPoolTransform, PT(GeomVertexData) > VertexPoolData;
  VertexPoolData _vertex_pool_data;

  typedef pmap<LMatrix4, CPT(TransformState) > TransformStates;
  TransformStates _transform_states;

  DeferredNodes _deferred_nodes;

public:
  PT(PandaNode) _root;
  PT(EggData) _data;
  PT(BamCacheRecord) _record;
  bool _error;

  CharacterMaker * _dynamic_override_char_maker;
  bool _dynamic_override;


  friend class EggRenderState;
  friend class PandaNode;
};

#include "eggLoader.I"

#endif
