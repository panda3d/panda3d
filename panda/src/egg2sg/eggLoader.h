// Filename: eggLoader.h
// Created by:  drose (21Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGLOADER_H
#define EGGLOADER_H

#include <pandabase.h>

#include "deferredArcTraverser.h"

#include <eggData.h>
#include <eggTexture.h>
#include <pt_EggTexture.h>
#include <eggGroup.h>
#include <eggMaterial.h>
#include <pt_EggMaterial.h>
#include <texture.h>
#include <namedNode.h>
#include <pt_NamedNode.h>
#include <pointerTo.h>
#include <builder.h>
#include <lmatrix.h>
#include <indirectCompareTo.h>
#include <textureTransition.h>
#include <textureApplyTransition.h>
#include <materialTransition.h>

class EggNode;
class EggBin;
class EggTable;
class EggNurbsCurve;
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
// 	 Class : EggLoader
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
  void reset_directs();

  void make_nonindexed_primitive(EggPrimitive *egg_prim, NamedNode *parent,
				 const LMatrix4d *transform = NULL);

  void make_indexed_primitive(EggPrimitive *egg_prim, NamedNode *parent,
			      const LMatrix4d *transform,
			      ComputedVerticesMaker &_comp_verts_maker);

private:
  class TextureDef {
  public:
    PT(TextureTransition) _texture;
    PT(TextureApplyTransition) _apply;
  };

  void load_textures();
  bool load_texture(TextureDef &def, const EggTexture *egg_tex);
  void apply_texture_attributes(Texture *tex, const EggTexture *egg_tex);
  void apply_texture_apply_attributes(TextureApplyTransition *apply, 
				      const EggTexture *egg_tex);

  MaterialTransition *get_material_transition(const EggMaterial *egg_mat, 
					      bool bface);

  void setup_bucket(BuilderBucket &bucket, NamedNode *parent,
		    EggPrimitive *egg_prim);

  RenderRelation *make_node(EggNode *egg_node, NamedNode *parent);
  RenderRelation *make_node(EggNurbsCurve *egg_curve, NamedNode *parent);
  RenderRelation *make_node(EggPrimitive *egg_prim, NamedNode *parent);
  RenderRelation *make_node(EggBin *egg_bin, NamedNode *parent);
  RenderRelation *make_node(EggGroup *egg_group, NamedNode *parent);
  RenderRelation *create_group_arc(EggGroup *egg_group, NamedNode *parent,
				   NamedNode *node);
  RenderRelation *make_node(EggTable *egg_table, NamedNode *parent);
  RenderRelation *make_node(EggGroupNode *egg_group, NamedNode *parent);

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
  void apply_collision_flags(CollisionSolid *solid, 
			     EggGroup::CollideFlags flags);
  EggGroup *find_collision_geometry(EggGroup *egg_group);
  CollisionPlane *create_collision_plane(EggPolygon *egg_poly,
					 EggGroup *parent_group);
  void create_collision_polygons(CollisionNode *cnode, EggPolygon *egg_poly, 
				 EggGroup *parent_group,
				 EggGroup::CollideFlags flags);

  void apply_deferred_arcs(Node *root);


  Builder _builder;

  typedef set<PT(TextureApplyTransition), IndirectCompareTo<NodeTransition> > TextureApplies;
  TextureApplies _texture_applies;

  typedef map<PT_EggTexture, TextureDef> Textures;
  Textures _textures;

  typedef map<CPT_EggMaterial, PT(MaterialTransition) > Materials;
  Materials _materials;
  Materials _materials_bface;

  typedef set<RenderRelation *> Decals;
  Decals _decals;

  typedef set<RenderRelation *> Directs;
  Directs _directs;

  DeferredArcs _deferred_arcs;

public: 
  PT_NamedNode _root;
  EggData _data;
  bool _error;
};


#endif
