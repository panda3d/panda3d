// Filename: eggLoader.cxx
// Created by:  drose (21Jan99)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "eggLoader.h"
#include "animBundleMaker.h"
#include "characterMaker.h"
#include "computedVerticesMaker.h"
#include "eggBinner.h"
#include "config_egg2sg.h"
#include "deferredArcProperty.h"

#include <string_utils.h>
#include <nodeRelation.h>
#include <renderRelation.h>
#include <geomNode.h>
#include <modelRoot.h>
#include <modelNode.h>
#include <LODNode.h>
#include <sequenceNode.h>
#include <eggGroup.h>
#include <eggTable.h>
#include <eggPrimitive.h>
#include <eggPolygon.h>
#include <eggPoint.h>
#include <eggTextureCollection.h>
#include <eggBin.h>
#include <builderBucket.h>
#include <builderPrim.h>
#include <builderVertex.h>
#include <texturePool.h>
#include <textureTransition.h>
#include <transformTransition.h>
#include <transparencyTransition.h>
#include <cullFaceTransition.h>
#include <billboardTransition.h>
#include <decalTransition.h>
#include <directRenderTransition.h>
#include <pruneTransition.h>
#include <animBundleNode.h>
#include <character.h>
#include <notify.h>
#include <luse.h>
#include <collisionNode.h>
#include <collisionPolygon.h>
#include <collisionPlane.h>
#include <collisionSphere.h>
#include <dftraverser.h>

#include <ctype.h>
#include <algorithm>

// This class is used in make_node(EggBin *) to sort LOD instances in
// order by switching distance.
class LODInstance {
public:
  LODInstance(EggNode *egg_node, RenderRelation *arc);
  bool operator < (const LODInstance &other) const {
    return _d->_switch_in < other._d->_switch_in;
  }

  RenderRelation *_arc;
  const EggSwitchConditionDistance *_d;
};

LODInstance::
LODInstance(EggNode *egg_node, RenderRelation *arc) {
  assert(arc != NULL);
  _arc = arc;

  // We expect this egg node to be an EggGroup with an LOD
  // specification.  That's what the EggBinner collected together,
  // after all.
  EggGroup *egg_group = DCAST(EggGroup, egg_node);
  assert(egg_group->has_lod());
  const EggSwitchCondition &sw = egg_group->get_lod();
  
  // For now, this is the only kind of switch condition there is.
  _d = DCAST(EggSwitchConditionDistance, &sw);
}

 
////////////////////////////////////////////////////////////////////
//     Function: EggLoader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggLoader::
EggLoader() {
  // We need to enforce whatever coordinate system the user asked for.
  _data.set_coordinate_system(egg_coordinate_system);
}
 
////////////////////////////////////////////////////////////////////
//     Function: EggLoader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggLoader::
EggLoader(const EggData &data) :
  _data(data)
{
}

 
////////////////////////////////////////////////////////////////////
//     Function: EggLoader::build_graph
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
build_graph() {
  _deferred_arcs.clear();

  // First, bin up the LOD nodes.
  EggBinner binner;
  binner.make_bins(&_data);

  // Then load up all of the textures.
  load_textures();

  // Now build up the scene graph.
  _root = new ModelRoot;
  _root->set_name(_data.get_egg_filename().get_basename());
  make_node(&_data, _root);
  _builder.build();

  reset_directs();
  reparent_decals();

  apply_deferred_arcs(_root);
}

 
////////////////////////////////////////////////////////////////////
//     Function: EggLoader::reparent_decals
//       Access: Public
//  Description: For each node representing a decal base geometry
//               (i.e. a node corresponding to an EggGroup with the
//               decal flag set), move all of its nested geometry
//               directly below the GeomNode representing the group.
////////////////////////////////////////////////////////////////////
void EggLoader::
reparent_decals() {
  Decals::const_iterator di;
  for (di = _decals.begin(); di != _decals.end(); ++di) {
    RenderRelation *arc = (*di);
    nassertv(arc != (RenderRelation *)NULL);
    NamedNode *node = DCAST(NamedNode, arc->get_child());
    nassertv(node != (NamedNode *)NULL);

    // First, search for the GeomNode.
    GeomNode *geom = NULL;
    int num_children = 
      node->get_num_children(RenderRelation::get_class_type());
    for (int i = 0; i < num_children; i++) {
      NodeRelation *child_arc = 
	node->get_child(RenderRelation::get_class_type(), i);
      nassertv(child_arc != (NodeRelation *)NULL);
      Node *child = child_arc->get_child();
      nassertv(child != (Node *)NULL);

      if (child->is_of_type(GeomNode::get_class_type())) {
	if (geom != (GeomNode *)NULL) {
	  // Oops, too many GeomNodes.
	  egg2sg_cat.warning()
	    << "Decal onto " << node->get_name() 
	    << " uses base geometry with multiple states.\n";
	  break;
	}
	DCAST_INTO_V(geom, child);
      }
    }

    if (geom == (GeomNode *)NULL) {
      // No children were GeomNodes.
      egg2sg_cat.warning()
	<< "Ignoring decal onto " << node->get_name()
	<< "; no geometry within group.\n";
    } else {
      // Now reparent all of the non-GeomNodes to this node.  We have
      // to be careful so we don't get lost as we self-modify this
      // list.
      int i = 0;
      while (i < num_children) {
	NodeRelation *child_arc = 
	  node->get_child(RenderRelation::get_class_type(), i);
	nassertv(child_arc != (NodeRelation *)NULL);
	Node *child = child_arc->get_child();
	nassertv(child != (Node *)NULL);

	if (child->is_of_type(GeomNode::get_class_type())) {
	  i++;
	} else {
	  child_arc->change_parent(geom);
	  num_children--;
	}
      }
    }
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: EggLoader::reset_directs
//       Access: Public
//  Description: This applies to all of the nodes marked with the
//               "render" flag, i.e. direct rendering of a subgraph,
//               in depth-first order, as opposed to state-sorting
//               within the subgraph.  For each such node, it moves
//               all the transitions from the first GeomNode under
//               that node up to the node itself, just so we'll be
//               able to state-sort at least the tops of the
//               subgraphs.
////////////////////////////////////////////////////////////////////
void EggLoader::
reset_directs() {
  Directs::const_iterator di;
  for (di = _directs.begin(); di != _directs.end(); ++di) {
    RenderRelation *arc = (*di);
    nassertv(arc != (RenderRelation *)NULL);
    NamedNode *node = DCAST(NamedNode, arc->get_child());
    nassertv(node != (NamedNode *)NULL);

    // First, search for the first GeomNode.
    GeomNode *geom = NULL;
    NodeRelation *child_arc = NULL;

    int num_children = 
      node->get_num_children(RenderRelation::get_class_type());
    for (int i = 0; i < num_children && geom == (GeomNode *)NULL; i++) {
      child_arc = node->get_child(RenderRelation::get_class_type(), i);
      nassertv(child_arc != (NodeRelation *)NULL);
      Node *child = child_arc->get_child();
      nassertv(child != (Node *)NULL);

      if (child->is_of_type(GeomNode::get_class_type())) {
	DCAST_INTO_V(geom, child);
      }
    }

    if (geom != (GeomNode *)NULL) {
      // Now copy all of the GeomNode's transitions up to its parent.
      nassertv(child_arc != (NodeRelation *)NULL);
      arc->copy_transitions_from(child_arc);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_nonindexed_primitive
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
make_nonindexed_primitive(EggPrimitive *egg_prim, NamedNode *parent,
			  const LMatrix4d *transform) {
  BuilderBucket bucket;
  setup_bucket(bucket, parent, egg_prim);

  LMatrix4d mat = egg_prim->get_vertex_to_node();

  if (transform != NULL) {
    mat = (*transform) * mat;
  } 

  BuilderPrim bprim;
  bprim.set_type(BPT_poly);
  if (egg_prim->is_of_type(EggPoint::get_class_type())) {
    bprim.set_type(BPT_point);
  }
  
  if (egg_prim->has_normal()) {
    bprim.set_normal(LCAST(float, egg_prim->get_normal() * mat));
  }
  if (egg_prim->has_color() && !egg_false_color) {
    bprim.set_color(LCAST(float, egg_prim->get_color()));
  }

  bool has_vert_color = true;
  EggPrimitive::const_iterator vi;
  for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
    EggVertex *egg_vert = *vi;
    BuilderVertex bvert(LCAST(float, egg_vert->get_pos3() * mat));
    
    if (egg_vert->has_normal()) {
      bvert.set_normal(LCAST(float, egg_vert->get_normal() * mat));
    }
    if (egg_vert->has_color() && !egg_false_color) {
      bvert.set_color(LCAST(float, egg_vert->get_color()));
    } else {
      // If any vertex doesn't have a color, we can't use any of the
      // vertex colors.
      has_vert_color = false;
    }
    if (egg_vert->has_uv()) {
      TexCoordd uv = egg_vert->get_uv();
      if (egg_prim->has_texture() && 
	  egg_prim->get_texture()->has_transform()) {
	// If we have a texture matrix, apply it.
	uv = uv * egg_prim->get_texture()->get_transform();
      }
      bvert.set_texcoord(LCAST(float, uv));
    }
    
    bprim.add_vertex(bvert);
  }

  // Finally, if the primitive didn't have a color, and it didn't have
  // vertex color, make it white.
  if (!egg_prim->has_color() && !has_vert_color && !egg_false_color) {
    bprim.set_color(Colorf(1.0, 1.0, 1.0, 1.0));
  }
  
  _builder.add_prim(bucket, bprim);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_indexed_primitive
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
make_indexed_primitive(EggPrimitive *egg_prim, NamedNode *parent,
		       const LMatrix4d *transform,
		       ComputedVerticesMaker &_comp_verts_maker) {
  BuilderBucket bucket;
  setup_bucket(bucket, parent, egg_prim);

  bucket.set_coords(_comp_verts_maker._coords);
  bucket.set_normals(_comp_verts_maker._norms);
  bucket.set_texcoords(_comp_verts_maker._texcoords);
  bucket.set_colors(_comp_verts_maker._colors);

  LMatrix4d mat = egg_prim->get_vertex_to_node();

  if (transform != NULL) {
    mat = (*transform) * mat;
  } 

  BuilderPrimI bprim;
  bprim.set_type(BPT_poly);
  if (egg_prim->is_of_type(EggPoint::get_class_type())) {
    bprim.set_type(BPT_point);
  }

  if (egg_prim->has_normal()) {
    // Define the transform space of the polygon normal.  This will be
    // the average of all the vertex transform spaces.
    _comp_verts_maker.begin_new_space();
    EggPrimitive::const_iterator vi;
    for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
      EggVertex *egg_vert = *vi;
      EggVertex::GroupRef::const_iterator gri;
      for (gri = egg_vert->gref_begin(); gri != egg_vert->gref_end(); ++gri) {
	EggGroup *egg_joint = (*gri);
	double membership = egg_joint->get_vertex_membership(egg_vert);
	_comp_verts_maker.add_joint(egg_joint, membership);
      }
    }
    _comp_verts_maker.mark_space();

    int nindex =
      _comp_verts_maker.add_normal(egg_prim->get_normal(),
				   egg_prim->_dnormals, mat);

    bprim.set_normal(nindex);
  }

  if (egg_prim->has_color() && !egg_false_color) {
    int cindex =
      _comp_verts_maker.add_color(egg_prim->get_color(),
				  egg_prim->_drgbas);
    bprim.set_color(cindex);
  }

  bool has_vert_color = true;
  EggPrimitive::const_iterator vi;
  for (vi = egg_prim->begin(); vi != egg_prim->end(); ++vi) {
    EggVertex *egg_vert = *vi;

    // Set up the ComputedVerticesMaker for the coordinate space of
    // the vertex.
    _comp_verts_maker.begin_new_space();
    if (egg_vert->gref_size() == 0) {
      // This vertex belongs where the primitive is.
      EggGroupNode *egg_joint = egg_prim->get_parent();
      _comp_verts_maker.add_joint(egg_joint, 1.0);

    } else {
      // This vertex belongs in the joint or joints that reference it.
      EggVertex::GroupRef::const_iterator gri;
      for (gri = egg_vert->gref_begin(); gri != egg_vert->gref_end(); ++gri) {
	EggGroup *egg_joint = (*gri);
	double membership = egg_joint->get_vertex_membership(egg_vert);
	_comp_verts_maker.add_joint(egg_joint, membership);
      }
    }
    _comp_verts_maker.mark_space();

    int vindex =
      _comp_verts_maker.add_vertex(egg_vert->get_pos3(), 
				   egg_vert->_dxyzs, mat);
    BuilderVertexI bvert(vindex);

    if (egg_vert->has_normal()) {
      int nindex =
	_comp_verts_maker.add_normal(egg_vert->get_normal(),
				     egg_vert->_dnormals,
				     mat);
      bvert.set_normal(nindex);
    }

    if (egg_vert->has_color() && !egg_false_color) {
      int cindex =
	_comp_verts_maker.add_color(egg_vert->get_color(),
				    egg_vert->_drgbas);
      bvert.set_color(cindex);
    } else {
      // If any vertex doesn't have a color, we can't use any of the
      // vertex colors.
      has_vert_color = false;
    }

    if (egg_vert->has_uv()) {
      TexCoordd uv = egg_vert->get_uv();
      LMatrix3d mat;

      if (egg_prim->has_texture() && 
	  egg_prim->get_texture()->has_transform()) {
	// If we have a texture matrix, apply it.
	mat = egg_prim->get_texture()->get_transform();
      } else {
	mat = LMatrix3d::ident_mat();
      }

      int tindex =
	_comp_verts_maker.add_texcoord(uv, egg_vert->_duvs, mat);
      bvert.set_texcoord(tindex);
    }

    bprim.add_vertex(bvert);
  }

  // Finally, if the primitive didn't have a color, and it didn't have
  // vertex color, make it white.
  if (!egg_prim->has_color() && !has_vert_color && !egg_false_color) {
    int cindex =
      _comp_verts_maker.add_color(Colorf(1.0, 1.0, 1.0, 1.0),
				  EggMorphColorList());
    bprim.set_color(cindex);
  }
  
  _builder.add_prim(bucket, bprim);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::load_textures
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
load_textures() {
  // First, collect all the textures that are referenced.
  EggTextureCollection tc;
  tc.find_used_textures(&_data);

  // Collapse the textures down by filename only.  Should we also
  // differentiate by attributes?  Maybe.
  EggTextureCollection::TextureReplacement replace;
  tc.collapse_equivalent_textures(EggTexture::E_complete_filename,
				  replace);

  EggTextureCollection::iterator ti;
  for (ti = tc.begin(); ti != tc.end(); ++ti) {
    PT(EggTexture) egg_tex = (*ti);

    TextureDef def;
    if (load_texture(def, egg_tex)) {
      // Now associate the pointers, so we'll be able to look up the
      // Texture pointer given an EggTexture pointer, later.
      _textures[egg_tex] = def;
    }
  }

  // Finally, associate all of the removed texture references back to
  // the same pointers as the others.
  EggTextureCollection::TextureReplacement::const_iterator ri;
  for (ri = replace.begin(); ri != replace.end(); ++ri) {
    PT(EggTexture) orig = (*ri).first;
    PT(EggTexture) repl = (*ri).second;

    _textures[orig] = _textures[repl];
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::load_texture
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggLoader::
load_texture(TextureDef &def, const EggTexture *egg_tex) {
  Texture *tex;
  if (egg_tex->has_alpha_file()) {
    tex = TexturePool::load_texture(egg_tex->get_filename(),
				    egg_tex->get_alpha_file());
  } else {
    tex = TexturePool::load_texture(egg_tex->get_filename());
  }
  if (tex == (Texture *)NULL) {
    return false;
  }

  if (egg_keep_texture_pathnames) {
    tex->set_name(egg_tex->get_filename());
    if (egg_tex->has_alpha_file()) {
      tex->set_alpha_name(egg_tex->get_alpha_file());
    } else {
      tex->clear_alpha_name();
    }
  }

  PT(TextureApplyTransition) apply = 
    new TextureApplyTransition(TextureApplyProperty::M_modulate);

  apply_texture_attributes(tex, egg_tex);
  apply_texture_apply_attributes(apply, egg_tex);

  def._texture = new TextureTransition(tex);
  def._apply = *(_texture_applies.insert(apply).first);

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_texture_attributes
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
apply_texture_attributes(Texture *tex, const EggTexture *egg_tex) {
  tex->set_name(egg_tex->get_filename().get_fullpath());

  switch (egg_tex->determine_wrap_u()) {
  case EggTexture::WM_repeat:
    tex->set_wrapu(Texture::WM_repeat);
    break;

  case EggTexture::WM_clamp:
    if (egg_ignore_clamp) {
      egg2sg_cat.warning()
	<< "Ignoring clamp request\n";
      tex->set_wrapu(Texture::WM_repeat);
    } else {
      tex->set_wrapu(Texture::WM_clamp);
    }
    break;

  case EggTexture::WM_unspecified:
    break;

  default:
    egg2sg_cat.warning()
      << "Unexpected texture wrap flag: "
      << (int)egg_tex->determine_wrap_u() << "\n";
  }

  switch (egg_tex->determine_wrap_v()) {
  case EggTexture::WM_repeat:
    tex->set_wrapv(Texture::WM_repeat);
    break;

  case EggTexture::WM_clamp:
    if (egg_ignore_clamp) {
      egg2sg_cat.warning()
	<< "Ignoring clamp request\n";
      tex->set_wrapv(Texture::WM_repeat);
    } else {
      tex->set_wrapv(Texture::WM_clamp);
    }
    break;

  case EggTexture::WM_unspecified:
    break;

  default:
    egg2sg_cat.warning()
      << "Unexpected texture wrap flag: "
      << (int)egg_tex->determine_wrap_v() << "\n";
  }

  switch (egg_tex->get_minfilter()) {
  case EggTexture::FT_nearest:
    tex->set_minfilter(Texture::FT_nearest);
    break;
    
  case EggTexture::FT_linear:
    if (egg_ignore_filters) {
      egg2sg_cat.warning()
	<< "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_linear);
    }
    break;
    
  case EggTexture::FT_nearest_mipmap_nearest:
    if (egg_ignore_filters) {
      egg2sg_cat.warning()
	<< "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2sg_cat.warning()
	<< "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_nearest_mipmap_nearest);
    }
    break;
    
  case EggTexture::FT_linear_mipmap_nearest:
    if (egg_ignore_filters) {
      egg2sg_cat.warning()
	<< "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2sg_cat.warning()
	<< "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_linear);
    } else {
      tex->set_minfilter(Texture::FT_linear_mipmap_nearest);
    }
    break;
    
  case EggTexture::FT_nearest_mipmap_linear:
    if (egg_ignore_filters) {
      egg2sg_cat.warning()
	<< "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2sg_cat.warning()
	<< "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_nearest_mipmap_linear);
    }
    break;
    
  case EggTexture::FT_linear_mipmap_linear:
    if (egg_ignore_filters) {
      egg2sg_cat.warning()
	<< "Ignoring minfilter request\n";
      tex->set_minfilter(Texture::FT_nearest);
    } else if (egg_ignore_mipmaps) {
      egg2sg_cat.warning()
	<< "Ignoring mipmap request\n";
      tex->set_minfilter(Texture::FT_linear);
    } else {
      tex->set_minfilter(Texture::FT_linear_mipmap_linear);
    }
    break;

  case EggTexture::FT_unspecified:
    // Default is bilinear, unless egg_ignore_filters is specified.
    if (egg_ignore_filters) {
      tex->set_minfilter(Texture::FT_nearest);
    } else {
      tex->set_minfilter(Texture::FT_linear);
    }
  }

  switch (egg_tex->get_magfilter()) {
  case EggTexture::FT_nearest:
  case EggTexture::FT_nearest_mipmap_nearest:
  case EggTexture::FT_nearest_mipmap_linear:
    tex->set_magfilter(Texture::FT_nearest);
    break;
    
  case EggTexture::FT_linear:
  case EggTexture::FT_linear_mipmap_nearest:
  case EggTexture::FT_linear_mipmap_linear:
    if (egg_ignore_filters) {
      egg2sg_cat.warning()
	<< "Ignoring magfilter request\n";
      tex->set_magfilter(Texture::FT_nearest);
    } else {
      tex->set_magfilter(Texture::FT_linear);
    }
    break;

  case EggTexture::FT_unspecified:
    // Default is bilinear, unless egg_ignore_filters is specified.
    if (egg_ignore_filters) {
      tex->set_magfilter(Texture::FT_nearest);
    } else {
      tex->set_magfilter(Texture::FT_linear);
    }
  }

  if (egg_tex->has_anisotropic_degree()) {
    tex->set_anisotropic_degree(egg_tex->get_anisotropic_degree());
  }

  if (tex->_pbuffer->get_num_components() == 1) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_red:
      tex->_pbuffer->set_format(PixelBuffer::F_red);
      break;
    case EggTexture::F_green:
      tex->_pbuffer->set_format(PixelBuffer::F_green);
      break;
    case EggTexture::F_blue:
      tex->_pbuffer->set_format(PixelBuffer::F_blue);
      break;
    case EggTexture::F_alpha:
      tex->_pbuffer->set_format(PixelBuffer::F_alpha);
      break;
    case EggTexture::F_luminance:
      tex->_pbuffer->set_format(PixelBuffer::F_luminance);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2sg_cat.error()
	<< "Ignoring inappropriate format " << egg_tex->get_format()
	<< " for 1-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->_pbuffer->get_num_components() == 2) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_luminance_alpha:
      tex->_pbuffer->set_format(PixelBuffer::F_luminance_alpha);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2sg_cat.error()
	<< "Ignoring inappropriate format " << egg_tex->get_format()
	<< " for 2-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->_pbuffer->get_num_components() == 3) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_rgb:
      tex->_pbuffer->set_format(PixelBuffer::F_rgb);
      break;
    case EggTexture::F_rgb12:
      if (tex->_pbuffer->get_component_width() >= 2) {
	// Only do this if the component width supports it.
	tex->_pbuffer->set_format(PixelBuffer::F_rgb12);
      } else {
	egg2sg_cat.error()
	  << "Ignoring inappropriate format " << egg_tex->get_format()
	  << " for 8-bit texture " << egg_tex->get_name() << "\n";
      }
      break;
    case EggTexture::F_rgb8:
    case EggTexture::F_rgba8:  
      // We'll quietly accept RGBA8 for a 3-component texture, since
      // flt2egg generates these for 3-component as well as for
      // 4-component textures.
      tex->_pbuffer->set_format(PixelBuffer::F_rgb8);
      break;
    case EggTexture::F_rgb5:
      tex->_pbuffer->set_format(PixelBuffer::F_rgb5);
      break;
    case EggTexture::F_rgba5:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba5);
      break;
    case EggTexture::F_rgb332:
      tex->_pbuffer->set_format(PixelBuffer::F_rgb332);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2sg_cat.error()
	<< "Ignoring inappropriate format " << egg_tex->get_format()
	<< " for 3-component texture " << egg_tex->get_name() << "\n";
    }

  } else if (tex->_pbuffer->get_num_components() == 4) {
    switch (egg_tex->get_format()) {
    case EggTexture::F_rgba:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba);
      break;
    case EggTexture::F_rgba12:
      if (tex->_pbuffer->get_component_width() >= 2) {
	// Only do this if the component width supports it.
	tex->_pbuffer->set_format(PixelBuffer::F_rgba12);
      } else {
	egg2sg_cat.error()
	  << "Ignoring inappropriate format " << egg_tex->get_format()
	  << " for 8-bit texture " << egg_tex->get_name() << "\n";
      }
      break;
    case EggTexture::F_rgba8:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba8);
      break;
    case EggTexture::F_rgba4:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba4);
      break;
    case EggTexture::F_rgba5:
      tex->_pbuffer->set_format(PixelBuffer::F_rgba5);
      break;

    case EggTexture::F_unspecified:
      break;

    default:
      egg2sg_cat.error()
	<< "Ignoring inappropriate format " << egg_tex->get_format()
	<< " for 4-component texture " << egg_tex->get_name() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_texture_apply_attributes
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
apply_texture_apply_attributes(TextureApplyTransition *apply,
			       const EggTexture *egg_tex) {
  if (egg_always_decal_textures) {
    apply->set_mode(TextureApplyProperty::M_decal);

  } else {
    switch (egg_tex->get_env_type()) {
    case EggTexture::ET_modulate:
      apply->set_mode(TextureApplyProperty::M_modulate);
      break;
      
    case EggTexture::ET_decal:
      apply->set_mode(TextureApplyProperty::M_decal);
      break;

    case EggTexture::ET_unspecified:
      break;

    default:
      egg2sg_cat.warning()
	<< "Invalid texture environment "
	   << (int)egg_tex->get_env_type() << "\n";
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::setup_bucket
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void EggLoader::
setup_bucket(BuilderBucket &bucket, NamedNode *parent, 
	     EggPrimitive *egg_prim) {
  bucket._node = parent;
  bucket._mesh = egg_mesh;
  bucket._retesselate_coplanar = egg_retesselate_coplanar;
  bucket._unroll_fans = egg_unroll_fans;
  bucket._show_tstrips = egg_show_tstrips;
  bucket._show_qsheets = egg_show_qsheets;
  bucket._show_quads = egg_show_quads;
  bucket._show_normals = egg_show_normals;
  bucket._normal_scale = egg_normal_scale;
  bucket._subdivide_polys = egg_subdivide_polys;
  bucket._consider_fans = egg_consider_fans;
  bucket._max_tfan_angle = egg_max_tfan_angle;
  bucket._min_tfan_tris = egg_min_tfan_tris;
  bucket._coplanar_threshold = egg_coplanar_threshold;

  // If a primitive has a name that does not begin with a digit, it
  // should be used to group primitives together--i.e. each primitive
  // with the same name gets placed into the same GeomNode.  However,
  // if a prim's name begins with a digit, just ignore it.
  if (egg_prim->has_name() && !isdigit(egg_prim->get_name()[0])) {
    bucket.set_name(egg_prim->get_name());
  }

  // Assign the appropriate properties to the bucket.
  EggAlphaMode::AlphaMode am = egg_prim->get_alpha_mode();
  bool implicit_alpha = false;

  bucket._trans.set_transition(new TextureTransition(TextureTransition::off()));
  if (egg_prim->has_texture()) {
    PT(EggTexture) egg_tex = egg_prim->get_texture();
    // If the primitive didn't specify an alpha mode, allow the
    // texture to specify one.
    if (am == EggAlphaMode::AM_unspecified) {
      am = egg_tex->get_alpha_mode();
    }
    
    const TextureDef &def = _textures[egg_tex];
    if (def._texture != (TextureTransition *)NULL) {
      bucket._trans.set_transition(def._texture);
      bucket._trans.set_transition(def._apply);

      // If neither the primitive nor the texture specified an alpha
      // mode, assume it should be alpha'ed if the texture has an
      // alpha channel.
      if (am == EggAlphaMode::AM_unspecified) {
	Texture *tex = def._texture->get_texture();
	nassertv(tex != (Texture *)NULL);
	int num_components = tex->_pbuffer->get_num_components();
	if (egg_tex->has_alpha_channel(num_components)) {
	  implicit_alpha = true;
	}
      }
    }
  }

  // Also check the color of the primitive to see if we should assume
  // alpha based on the alpha values specified in the egg file.
  if (am == EggAlphaMode::AM_unspecified) {
    if (egg_prim->has_color()) {
      if (egg_prim->get_color()[3] != 1.0) {
	implicit_alpha = true;
      }
    }
    EggPrimitive::const_iterator vi;
    for (vi = egg_prim->begin(); 
	 !implicit_alpha && vi != egg_prim->end();
	 ++vi) {
      if ((*vi)->has_color()) {
	if ((*vi)->get_color()[3] != 1.0) {
	  implicit_alpha = true;
	}
      }
    }
    
    if (implicit_alpha) {
      am = EggAlphaMode::AM_on;
    }
  }

  switch (am) {
  case EggAlphaMode::AM_on:
  case EggAlphaMode::AM_blend:
    bucket._trans.set_transition(new TransparencyTransition(TransparencyProperty::M_alpha));
    break;

  case EggAlphaMode::AM_blend_no_occlude:
    bucket._trans.set_transition(new TransparencyTransition(TransparencyProperty::M_alpha_sorted));
    break;

  case EggAlphaMode::AM_ms:
    bucket._trans.set_transition(new TransparencyTransition(TransparencyProperty::M_multisample));
    break;

  case EggAlphaMode::AM_ms_mask:
    bucket._trans.set_transition(new TransparencyTransition(TransparencyProperty::M_multisample_mask));
    break;

  default:
    //    bucket._trans.set_transition(new TransparencyTransition(TransparencyProperty::M_none));
    break;
  }

  if (egg_prim->get_bface_flag()) {
    // The primitive is marked with backface culling disabled--we want
    // to see both sides.
    bucket._trans.set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none));

    /*
      Not sure if we need to enforce this between children.
  } else {
    bucket._trans.set_transition(new CullFaceTransition(CullFaceProperty::M_cull_clockwise));
    */
  }
}



////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
make_node(EggNode *egg_node, NamedNode *parent) {
  if (egg_node->is_of_type(EggPrimitive::get_class_type())) {
    return make_node(DCAST(EggPrimitive, egg_node), parent);
  } else if (egg_node->is_of_type(EggBin::get_class_type())) {
    return make_node(DCAST(EggBin, egg_node), parent);
  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    return make_node(DCAST(EggGroup, egg_node), parent);
  } else if (egg_node->is_of_type(EggTable::get_class_type())) {
    return make_node(DCAST(EggTable, egg_node), parent); 
  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    return make_node(DCAST(EggGroupNode, egg_node), parent);
  }

  return (RenderRelation *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggPrimitive)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
make_node(EggPrimitive *egg_prim, NamedNode *parent) {
  assert(parent != NULL);
  assert(!parent->is_of_type(GeomNode::get_class_type()));

  egg_prim->cleanup();
  make_nonindexed_primitive(egg_prim, parent);
  return (RenderRelation *)NULL;
}
  
////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggBin)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
make_node(EggBin *egg_bin, NamedNode *parent) {
  // Presently, an EggBin can only mean an LOD node (i.e. a parent of
  // one or more EggGroups with LOD specifications).  Later it might
  // mean other things as well.

  assert((EggBinner::BinNumber)egg_bin->get_bin_number() == EggBinner::BN_lod);
  LODNode *lod_node = new LODNode;
  lod_node->set_name(egg_bin->get_name());

  vector<LODInstance> instances;

  EggGroup::const_iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    RenderRelation *arc = make_node(*ci, lod_node);
    assert(arc != (RenderRelation *)NULL);
    LODInstance instance(*ci, arc);
    instances.push_back(instance);
  }

  // Now that we've created all of our children, put them in the
  // proper order and tell the LOD node about them.
  sort(instances.begin(), instances.end());

  if (!instances.empty()) {
    // Set up the LOD node's center.  All of the children should have
    // the same center, because that's how we binned them.
    lod_node->_lod._center = LCAST(float, instances[0]._d->_center);
  }

  for (size_t i = 0; i < instances.size(); i++) {
    // Put the children in the proper order within the scene graph.
    const LODInstance &instance = instances[i];

    // All of the children should have the same center, because that's
    // how we binned them.
    assert(lod_node->_lod._center.almost_equal
	   (LCAST(float, instance._d->_center), 0.01));

    instance._arc->set_sort(i);

    // Tell the LOD node about this child's switching distances.
    lod_node->add_switch(instance._d->_switch_in, instance._d->_switch_out);
  }

  return create_group_arc(egg_bin, parent, lod_node);
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggGroup)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
make_node(EggGroup *egg_group, NamedNode *parent) {
  NamedNode *node = NULL;

  if (egg_group->has_objecttype()) {
    // We'll allow recursive expansion of ObjectType strings--but we
    // don't want to get caught in a cycle.  Keep track of the strings
    // we've expanded so far.
    set<string> expanded;
    vector<string> expanded_history;

    while (egg_group->has_objecttype()) {
      string objecttype = egg_group->get_objecttype();
      if (!expanded.insert(objecttype).second) {
	egg2sg_cat.error()
	  << "Cycle in ObjectType expansions:\n";
	copy(expanded_history.begin(), expanded_history.end(),
	     ostream_iterator<string>(egg2sg_cat.error(false), " -> "));
	egg2sg_cat.error(false) << objecttype << "\n";
	break;
      }
      expanded_history.push_back(objecttype);

      // Now clear the group's ObjectType flag.  We'll only loop back
      // here again if we end up setting this during the ObjectType
      // expansion; e.g. the expansion string itself contains an
      // <ObjectType> reference.
      egg_group->clear_objecttype();

      // Now try to find the egg syntax that the given objecttype is
      // shorthand for.  First, look in the config file.

      string egg_syntax =
	config_egg2sg.GetString("egg-object-type-" + objecttype, "none");
      
      if (egg_syntax == "none") {
	// It wasn't defined in a config file.  Maybe it's built in?
	
	if (cmp_nocase_uh(objecttype, "barrier") == 0) {
	  egg_syntax = "<Collide> { Polyset descend }";
	  
	} else if (cmp_nocase_uh(objecttype, "solidpoly") == 0) {
	  egg_syntax = "<Collide> { Polyset descend solid }";
	  
	} else if (cmp_nocase_uh(objecttype, "turnstile") == 0) {
	  egg_syntax = "<Collide> { Polyset descend turnstile }";
	  
	} else if (cmp_nocase_uh(objecttype, "sphere") == 0) {
	  egg_syntax = "<Collide> { Sphere descend }";
	  
	} else if (cmp_nocase_uh(objecttype, "trigger") == 0) {
	  egg_syntax = "<Collide> { Polyset descend intangible }";
	  
	} else if (cmp_nocase_uh(objecttype, "trigger_sphere") == 0) {
	  egg_syntax = "<Collide> { Sphere descend intangible }";
	  
	} else if (cmp_nocase_uh(objecttype, "eye_trigger") == 0) {
	  egg_syntax = "<Collide> { Polyset descend intangible center }";
	  
	} else if (cmp_nocase_uh(objecttype, "bubble") == 0) {
	  egg_syntax = "<Collide> { Sphere keep descend }";
	  
	} else if (cmp_nocase_uh(objecttype, "ghost") == 0) {
	  egg_syntax = "<Scalar> collide-mask { 0 }";
	  
	} else if (cmp_nocase_uh(objecttype, "backstage") == 0) {
	  // Ignore "backstage" geometry.
	  return NULL;
	  
	} else {
	  egg2sg_cat.warning()
	    << "Ignoring unknown ObjectType " << objecttype << "\n";
	  break;
	}
      }

      if (!egg_syntax.empty()) {
	if (!egg_group->parse_egg(egg_syntax)) {
	  egg2sg_cat.error()
	    << "Error while parsing definition for ObjectType " 
	    << objecttype << "\n";
	}
      }
    }
  }

  if (egg_group->get_dart_type() != EggGroup::DT_none) {
    // A group with the <Dart> flag set means to create a character.
    CharacterMaker char_maker(egg_group, *this);
    node = char_maker.make_node();

  } else if (egg_group->get_cs_type() != EggGroup::CST_none &&
	     egg_group->get_cs_type() != EggGroup::CST_geode) {
    // A collision group: create collision geometry.
    node = new CollisionNode;
    node->set_name(egg_group->get_name());
    
    make_collision_solids(egg_group, egg_group, (CollisionNode *)node);
    if ((egg_group->get_collide_flags() & EggGroup::CF_keep) != 0) {
      // If we also specified to keep the geometry, continue the
      // traversal.
      EggGroup::const_iterator ci;
      for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
	make_node(*ci, parent);
      }
    }

    RenderRelation *arc = create_group_arc(egg_group, parent, node);

    if (!egg_show_collision_solids) {
      arc->set_transition(new PruneTransition());
    }
    return arc;

  } else if (egg_group->get_switch_flag() && 
	     egg_group->get_switch_fps() != 0.0) {
    // Create a sequence node.
    node = new SequenceNode(1.0 / egg_group->get_switch_fps());
    node->set_name(egg_group->get_name());
    
    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }

  } else if (egg_group->get_model_flag()) {
    // A model flag; create a model node.
    node = new ModelNode;
    node->set_name(egg_group->get_name());
    
    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }

  } else {
    // A normal group; just create a normal node, and traverse.
    node = new NamedNode;
    node->set_name(egg_group->get_name());
    
    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      make_node(*ci, node);
    }
  }

  if (node == (NamedNode *)NULL) {
    return NULL;
  }
  return create_group_arc(egg_group, parent, node);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::create_group_arc
//       Access: Private
//  Description: Creates the arc parenting a new group to the scene
//               graph, and applies any relevant transitions to the
//               arc according to the EggGroup node that inspired the
//               group.
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
create_group_arc(EggGroup *egg_group, NamedNode *parent, NamedNode *node) {
  RenderRelation *arc = new RenderRelation(parent, node);
  
  // If the group had a transform, apply it to the arc.
  if (egg_group->has_transform()) {
    LMatrix4f matf = LCAST(float, egg_group->get_transform());
    arc->set_transition(new TransformTransition(matf));
  }

  // If the group has a billboard flag, apply that.
  switch (egg_group->get_billboard_type()) {
  case EggGroup::BT_point_camera_relative:
    arc->set_transition(new BillboardTransition(BillboardTransition::point_eye()));
    break;

  case EggGroup::BT_point_world_relative:
    arc->set_transition(new BillboardTransition(BillboardTransition::point_world()));
    break;

  case EggGroup::BT_axis:
    arc->set_transition(new BillboardTransition(BillboardTransition::axis()));
    break;

  case EggGroup::BT_none:
    break;
  }

  if (egg_group->get_decal_flag()) {
    if (egg_ignore_decals) {
      egg2sg_cat.warning() 
	<< "Ignoring decal flag on " << egg_group->get_name() << "\n";
    }

    // If the group has the "decal" flag set, it means that all of the
    // descendant groups will be decaled onto the geometry within
    // this group.  This means we'll need to reparent things a bit
    // afterward.
    _decals.insert(arc);

    // We'll also set up the DecalTransition now.
    arc->set_transition(new DecalTransition);

  }

  if (egg_group->get_direct_flag()) {
    // If the group has the "direct" flag set, it means that
    // everything at this node and below should be rendered in direct
    // mode, i.e. in depth-first tree order, without state-sorting.

    arc->set_transition(new DirectRenderTransition);

    // We'll also want to set up the transitions on this arc to
    // reflect the geometry at the top of the tree below this node, so
    // we get good state-sorting behavior.  We'll have to do this
    // later.
    _directs.insert(arc);
  }

  // If the group specified some property that should propagate down
  // to the leaves, we have to remember this arc and apply the
  // property later, after we've created the actual geometry.
  DeferredArcProperty def;
  if (egg_group->has_collide_mask()) {
    def._from_collide_mask = egg_group->get_collide_mask();
    def._into_collide_mask = egg_group->get_collide_mask();
    def._flags |= 
      DeferredArcProperty::F_has_from_collide_mask |
      DeferredArcProperty::F_has_into_collide_mask;
  }
  if (egg_group->has_from_collide_mask()) {
    def._from_collide_mask = egg_group->get_from_collide_mask();
    def._flags |= DeferredArcProperty::F_has_from_collide_mask;
  }
  if (egg_group->has_into_collide_mask()) {
    def._into_collide_mask = egg_group->get_into_collide_mask();
    def._flags |= DeferredArcProperty::F_has_into_collide_mask;
  }

  if (def._flags != 0) {
    _deferred_arcs[arc] = def;
  }
  
  return arc;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggTable)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
make_node(EggTable *egg_table, NamedNode *parent) {
  if (egg_table->get_table_type() != EggTable::TT_bundle) {
    // We only do anything with bundles.  Isolated tables are treated
    // as ordinary groups.
    return make_node(DCAST(EggGroupNode, egg_table), parent);
  }

  // It's an actual bundle, so make an AnimBundle from it and its
  // descendants.
  AnimBundleMaker bundle_maker(egg_table);
  AnimBundleNode *node = bundle_maker.make_node();
  return new RenderRelation(parent, node);
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_node (EggGroupNode)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
RenderRelation *EggLoader::
make_node(EggGroupNode *egg_group, NamedNode *parent) {
  NamedNode *node = new NamedNode;
  node->set_name(egg_group->get_name());

  EggGroupNode::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    make_node(*ci, node);
  }

  return new RenderRelation(parent, node);
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_solids
//       Access: Private
//  Description: Creates CollisionSolids corresponding to the
//               collision geometry indicated at the given node and
//               below.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_solids(EggGroup *start_group, EggGroup *egg_group, 
		      CollisionNode *cnode) {
  if (egg_group->get_cs_type() != EggGroup::CST_none) {
    start_group = egg_group;
  }

  switch (start_group->get_cs_type()) {
  case EggGroup::CST_none:
  case EggGroup::CST_geode:
    // No collision flags; do nothing.  Don't even traverse further.
    return;

  case EggGroup::CST_inverse_sphere:
    // These aren't presently supported.
    egg2sg_cat.warning() 
      << "Not presently supported: <Collide> { " 
      << egg_group->get_cs_type() << " }\n";
    break;

  case EggGroup::CST_plane:
    make_collision_plane(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_polygon:
    make_collision_polygon(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_polyset:
    make_collision_polyset(egg_group, cnode, start_group->get_collide_flags());
    break;

  case EggGroup::CST_sphere:
    make_collision_sphere(egg_group, cnode, start_group->get_collide_flags());
    break;
  }

  if ((start_group->get_collide_flags() & EggGroup::CF_descend) != 0) {
    // Now pick up everything below.
    EggGroup::const_iterator ci;
    for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggGroup::get_class_type())) {
	make_collision_solids(start_group, DCAST(EggGroup, *ci), cnode);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_plane
//       Access: Private
//  Description: Creates a single CollisionPlane corresponding
//               to the first polygon associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_plane(EggGroup *egg_group, CollisionNode *cnode,
		     EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
	CollisionPlane *csplane =
	  create_collision_plane(DCAST(EggPolygon, *ci));
	if (csplane != (CollisionPlane *)NULL) {
	  apply_collision_flags(csplane, flags);
	  cnode->add_solid(csplane);
	  return;
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_polygon
//       Access: Private
//  Description: Creates a single CollisionPolygon corresponding
//               to the first polygon associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_polygon(EggGroup *egg_group, CollisionNode *cnode,
		       EggGroup::CollideFlags flags) {

  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
	CollisionPolygon *cspoly =
	  create_collision_polygon(DCAST(EggPolygon, *ci));
	if (cspoly != (CollisionPolygon *)NULL) {
	  apply_collision_flags(cspoly, flags);
	  cnode->add_solid(cspoly);
	  return;
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_polyset
//       Access: Private
//  Description: Creates a series of CollisionPolygons corresponding
//               to the polygons associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_polyset(EggGroup *egg_group, CollisionNode *cnode,
		       EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
	CollisionPolygon *cspoly =
	  create_collision_polygon(DCAST(EggPolygon, *ci));
	if (cspoly != (CollisionPolygon *)NULL) {
	  apply_collision_flags(cspoly, flags);
	  cnode->add_solid(cspoly);
	}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::make_collision_sphere
//       Access: Private
//  Description: Creates a single CollisionSphere corresponding
//               to the polygons associated with this group.
////////////////////////////////////////////////////////////////////
void EggLoader::
make_collision_sphere(EggGroup *egg_group, CollisionNode *cnode,
		      EggGroup::CollideFlags flags) {
  EggGroup *geom_group = find_collision_geometry(egg_group);
  if (geom_group != (EggGroup *)NULL) {
    // Collect all of the vertices.
    set<EggVertex *> vertices;

    EggGroup::const_iterator ci;
    for (ci = geom_group->begin(); ci != geom_group->end(); ++ci) {
      if ((*ci)->is_of_type(EggPrimitive::get_class_type())) {
	EggPrimitive *prim = DCAST(EggPrimitive, *ci);
	EggPrimitive::const_iterator pi;
	for (pi = prim->begin(); pi != prim->end(); ++pi) {
	  vertices.insert(*pi);
	}
      }
    }

    // Now average together all of the vertices to get a center.
    int num_vertices = 0;
    LPoint3d center(0.0, 0.0, 0.0);
    set<EggVertex *>::const_iterator vi;

    for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
      EggVertex *vtx = (*vi);
      if (vtx->get_num_dimensions() == 3) {
	center += vtx->get_pos3();
	num_vertices++;

      } else if (vtx->get_num_dimensions() == 4) {
	LPoint4d p4 = vtx->get_pos4();
	if (p4[3] != 0.0) {
	  center += LPoint3d(p4[0], p4[1], p4[2]) / p4[3];
	  num_vertices++;
	}
      }
    }

    if (num_vertices > 0) {
      center /= (double)num_vertices;
      
      // And the furthest vertex determines the radius.
      double radius2 = 0.0;
      for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
	EggVertex *vtx = (*vi);
	if (vtx->get_num_dimensions() == 3) {
	  LVector3d v = vtx->get_pos3() - center;
	  radius2 = max(radius2, v.length_squared());
	  
	} else if (vtx->get_num_dimensions() == 4) {
	  LPoint4d p = vtx->get_pos4();
	  if (p[3] != 0.0) {
	    LVector3d v = LPoint3d(p[0], p[1], p[2]) / p[3] - center;
	    radius2 = max(radius2, v.length_squared());
	  }
	}
      }

      float radius = sqrtf(radius2);
      CollisionSphere *cssphere = 
	new CollisionSphere(LCAST(float, center), radius);
      apply_collision_flags(cssphere, flags);
      cnode->add_solid(cssphere);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_collision_flags
//       Access: Private
//  Description: Does funny stuff to the CollisionSolid as
//               appropriate, based on the settings of the given
//               CollideFlags.
////////////////////////////////////////////////////////////////////
void EggLoader:: 
apply_collision_flags(CollisionSolid *solid, 
		      EggGroup::CollideFlags flags) {
  if ((flags & EggGroup::CF_intangible) != 0) {
    solid->set_tangible(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::find_collision_geometry
//       Access: Private
//  Description: Looks for the node, at or below the indicated node,
//               that contains the associated collision geometry.
////////////////////////////////////////////////////////////////////
EggGroup *EggLoader::
find_collision_geometry(EggGroup *egg_group) {
  if ((egg_group->get_collide_flags() & EggGroup::CF_descend) != 0) {
    // If we have the "descend" instruction, we'll get to it when we
    // get to it.  Don't worry about it now.
    return egg_group;
  }

  // Does this group have any polygons?
  EggGroup::const_iterator ci;
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggPolygon::get_class_type())) {
      // Yes!  Use this group.
      return egg_group;
    }
  }

  // Well, the group had no polygons; look for a child group that has
  // the same collision type.
  for (ci = egg_group->begin(); ci != egg_group->end(); ++ci) {
    if ((*ci)->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, *ci);
      if (child_group->get_cs_type() == egg_group->get_cs_type()) {
	return child_group;
      }
    }
  }

  // We got nothing.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::create_collision_plane
//       Access: Private
//  Description: Creates a single CollisionPlane from the indicated
//               EggPolygon.
////////////////////////////////////////////////////////////////////
CollisionPlane *EggLoader::
create_collision_plane(EggPolygon *egg_poly) {
  egg_poly->cleanup();

  vector<Vertexf> vertices;
  if (!egg_poly->empty()) {
    EggPolygon::const_iterator vi;
    vi = egg_poly->begin();

    Vertexd vert = (*vi)->get_pos3();
    vertices.push_back(LCAST(float, vert));
    
    Vertexd last_vert = vert;
    ++vi;
    while (vi != egg_poly->end()) {
      vert = (*vi)->get_pos3();
      if (!vert.almost_equal(last_vert)) {
	vertices.push_back(LCAST(float, vert));
      }

      last_vert = vert;
      ++vi;
    }
  }

  if (vertices.size() < 3) {
    return NULL;
  }
  Planef plane(vertices[0], vertices[1], vertices[2]);
  return new CollisionPlane(plane);
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::create_collision_polygon
//       Access: Private
//  Description: Creates a single CollisionPolygon from the indicated
//               EggPolygon.
////////////////////////////////////////////////////////////////////
CollisionPolygon *EggLoader::
create_collision_polygon(EggPolygon *egg_poly) {
  egg_poly->cleanup();

  vector<Vertexf> vertices;
  if (!egg_poly->empty()) {
    EggPolygon::const_iterator vi;
    vi = egg_poly->begin();

    Vertexd vert = (*vi)->get_pos3();
    vertices.push_back(LCAST(float, vert));
    
    Vertexd last_vert = vert;
    ++vi;
    while (vi != egg_poly->end()) {
      vert = (*vi)->get_pos3();
      if (!vert.almost_equal(last_vert)) {
	vertices.push_back(LCAST(float, vert));
      }

      last_vert = vert;
      ++vi;
    }
  }

  if (vertices.size() < 3) {
    return NULL;
  }
  return new CollisionPolygon(vertices.begin(), vertices.end());
}

////////////////////////////////////////////////////////////////////
//     Function: EggLoader::apply_deferred_arcs
//       Access: Private
//  Description: Walks back over the tree and applies the
//               DeferredArcProperties that were saved up along the
//               way.
////////////////////////////////////////////////////////////////////
void EggLoader::
apply_deferred_arcs(Node *root) {
  DeferredArcTraverser trav(_deferred_arcs);

  df_traverse(root, trav, NullAttributeWrapper(), DeferredArcProperty(),
	      RenderRelation::get_class_type());
}
