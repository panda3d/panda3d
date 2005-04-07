// Filename: eggRenderState.cxx
// Created by:  drose (12Mar05)
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

#include "eggRenderState.h"
#include "eggRenderMode.h"
#include "eggLine.h"
#include "eggPoint.h"
#include "textureAttrib.h"
#include "renderAttrib.h"
#include "eggTexture.h"
#include "texGenAttrib.h"
#include "internalName.h"
#include "eggCurve.h"
#include "eggSurface.h"
#include "cullBinAttrib.h"
#include "cullFaceAttrib.h"
#include "shadeModelAttrib.h"
#include "transparencyAttrib.h"
#include "depthWriteAttrib.h"
#include "depthTestAttrib.h"
#include "texMatrixAttrib.h"
#include "renderModeAttrib.h"
#include "material.h"
#include "materialAttrib.h"
#include "materialPool.h"
#include "config_gobj.h"


////////////////////////////////////////////////////////////////////
//     Function: EggRenderState::fill_state
//       Access: Public
//  Description: Sets up the state as appropriate for the indicated
//               primitive.
////////////////////////////////////////////////////////////////////
void EggRenderState::
fill_state(EggPrimitive *egg_prim) {
  // The various EggRenderMode properties can be defined directly at
  // the primitive, at a group above the primitive, or an a texture
  // applied to the primitive.  The EggNode::determine_*() functions
  // can find the right pointer to the level at which this is actually
  // defined for a given primitive.
  EggRenderMode::AlphaMode am = EggRenderMode::AM_unspecified;
  EggRenderMode::DepthWriteMode dwm = EggRenderMode::DWM_unspecified;
  EggRenderMode::DepthTestMode dtm = EggRenderMode::DTM_unspecified;
  EggRenderMode::VisibilityMode vm = EggRenderMode::VM_unspecified;
  bool implicit_alpha = false;
  bool has_draw_order = false;
  int draw_order = 0;
  bool has_bin = false;
  string bin;

  EggRenderMode *render_mode;
  render_mode = egg_prim->determine_alpha_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    am = render_mode->get_alpha_mode();
  }
  render_mode = egg_prim->determine_depth_write_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    dwm = render_mode->get_depth_write_mode();
  }
  render_mode = egg_prim->determine_depth_test_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    dtm = render_mode->get_depth_test_mode();
  }
  render_mode = egg_prim->determine_visibility_mode();
  if (render_mode != (EggRenderMode *)NULL) {
    vm = render_mode->get_visibility_mode();
  }
  render_mode = egg_prim->determine_draw_order();
  if (render_mode != (EggRenderMode *)NULL) {
    has_draw_order = true;
    draw_order = render_mode->get_draw_order();
  }
  render_mode = egg_prim->determine_bin();
  if (render_mode != (EggRenderMode *)NULL) {
    has_bin = true;
    bin = render_mode->get_bin();
  }

  add_attrib(TextureAttrib::make_off());
  int num_textures = egg_prim->get_num_textures();
  CPT(RenderAttrib) texture_attrib = NULL;
  CPT(RenderAttrib) tex_gen_attrib = NULL;
  CPT(RenderAttrib) tex_mat_attrib = NULL;
  TexMats tex_mats;

  for (int i = 0; i < num_textures; i++) {
    PT_EggTexture egg_tex = egg_prim->get_texture(i);

    const TextureDef &def = _loader._textures[egg_tex];
    if (def._texture != (const RenderAttrib *)NULL) {
      if (texture_attrib == (RenderAttrib *)NULL) {
        texture_attrib = def._texture;
      } else {
        texture_attrib = texture_attrib->compose(def._texture);
      }

      // If neither the primitive nor the texture specified an alpha
      // mode, assume it should be alpha'ed if the texture has an
      // alpha channel (unless the texture environment type is one
      // that doesn't apply its alpha to the result).
      if (am == EggRenderMode::AM_unspecified) {
        const TextureAttrib *tex_attrib = DCAST(TextureAttrib, def._texture);
        Texture *tex = tex_attrib->get_texture();
        nassertv(tex != (Texture *)NULL);
        int num_components = tex->get_num_components();
        if (egg_tex->has_alpha_channel(num_components)) {
          switch (egg_tex->get_env_type()) {
          case EggTexture::ET_decal:
          case EggTexture::ET_add:
            break;

          default:
            implicit_alpha = true;
          }
        }
      }

      // Check for a texgen attrib.
      bool has_tex_gen = false;
      if (egg_tex->get_tex_gen() != EggTexture::TG_unspecified) {
        has_tex_gen = true;
        if (tex_gen_attrib == (const RenderAttrib *)NULL) {
          tex_gen_attrib = TexGenAttrib::make();
        }
        tex_gen_attrib = DCAST(TexGenAttrib, tex_gen_attrib)->
          add_stage(def._stage, get_tex_gen(egg_tex));
      }

      // Record the texture's associated texture matrix, so we can see
      // if we can safely bake it into the UV's.  (We need to get the
      // complete list of textures that share this same set of UV's
      // per each unique texture matrix.  Whew!)
      CPT(InternalName) uv_name;
      if (egg_tex->has_uv_name() && egg_tex->get_uv_name() != string("default")) {
        uv_name = InternalName::get_texcoord_name(egg_tex->get_uv_name());
      } else {
        uv_name = InternalName::get_texcoord();
      }

      if (has_tex_gen) {
        // If the texture has a texgen mode, we will always apply its
        // texture transform, never bake it in.  In fact, we don't
        // even care about its UV's in this case, since we won't be
        // using them.
        tex_mat_attrib = apply_tex_mat(tex_mat_attrib, def._stage, egg_tex);

      } else {
        // Otherwise, we need to record that there is at least one
        // texture on this particular UV name and with this particular
        // texture matrix.  If there are no other textures, or if all
        // of the other textures use the same texture matrix, then
        // tex_mats[uv_name].size() will remain 1 (which tells us we
        // can bake in the texture matrix to the UV's).  On the other
        // hand, if there is another texture on the same uv name but
        // with a different transform, it will increase
        // tex_mats[uv_name].size() to at least 2, indicating we can't
        // bake in the texture matrix.
        tex_mats[uv_name][egg_tex->get_transform()].push_back(&def);
      }
    }
  }

  // These parametric primitive types can't have their UV's baked in,
  // so if we have one of these we always need to apply the texture
  // matrix as a separate attribute, regardless of how many textures
  // share the particular UV set.
  bool needs_tex_mat = (egg_prim->is_of_type(EggCurve::get_class_type()) ||
                        egg_prim->is_of_type(EggSurface::get_class_type()));

  // Now that we've visited all of the textures in the above loop, we
  // can go back and see how many of them share the same UV name and
  // texture matrix.
  TexMats::const_iterator tmi;
  for (tmi = tex_mats.begin(); tmi != tex_mats.end(); ++tmi) {
    const InternalName *uv_name = (*tmi).first;
    const TexMatTransforms &tmt = (*tmi).second;

    if (tmt.size() == 1 && !needs_tex_mat) {
      // Only one unique transform sharing this set of UV's.  We can
      // bake in the transform!
      const TexMatTextures &tmtex = (*tmt.begin()).second;

      // The first EggTexture on the list is sufficient, since we know
      // they all have the same transform.
      nassertv(!tmtex.empty());
      TexMatTextures::const_iterator tmtexi = tmtex.begin();
      const EggTexture *egg_tex = (*tmtexi)->_egg_tex;
      if (egg_tex->has_transform()) {
        // If there's no transform, it's an identity matrix; don't
        // bother recording it.  Of course, it would do no harm to
        // record it if we felt like it.
        _bake_in_uvs[uv_name] = egg_tex;
      }

    } else {
      // Multiple transforms on this UV set, or a geometry type that
      // doesn't support baking in UV's.  We have to apply the
      // texture matrix to each stage.
      TexMatTransforms::const_iterator tmti;
      for (tmti = tmt.begin(); tmti != tmt.end(); ++tmti) {
        const TexMatTextures &tmtex = (*tmti).second;
        TexMatTextures::const_iterator tmtexi;
        for (tmtexi = tmtex.begin(); tmtexi != tmtex.end(); ++tmtexi) {
          const EggTexture *egg_tex = (*tmtexi)->_egg_tex;
          TextureStage *stage = (*tmtexi)->_stage;
          
          tex_mat_attrib = apply_tex_mat(tex_mat_attrib, stage, egg_tex);
        }
      }
    }
  }

  if (texture_attrib != (RenderAttrib *)NULL) {
    add_attrib(texture_attrib);
  }

  if (tex_gen_attrib != (RenderAttrib *)NULL) {
    add_attrib(tex_gen_attrib);
  }

  if (tex_mat_attrib != (RenderAttrib *)NULL) {
    add_attrib(tex_mat_attrib);
  }

  if (egg_prim->has_material()) {
    CPT(RenderAttrib) mt =
      get_material_attrib(egg_prim->get_material(),
                          egg_prim->get_bface_flag());
    add_attrib(mt);
  }


  // Also check the color of the primitive to see if we should assume
  // alpha based on the alpha values specified in the egg file.
  if (am == EggRenderMode::AM_unspecified) {
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
      am = EggRenderMode::AM_on;
    }
  }

  if (am == EggRenderMode::AM_on && 
      egg_alpha_mode != EggRenderMode::AM_unspecified) {
    // Alpha type "on" means to get the default transparency type.
    am = egg_alpha_mode;
  }

  switch (am) {
  case EggRenderMode::AM_on:
  case EggRenderMode::AM_blend:
    add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    break;

  case EggRenderMode::AM_blend_no_occlude:
    add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
    break;

  case EggRenderMode::AM_ms:
    add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_multisample));
    break;

  case EggRenderMode::AM_ms_mask:
    add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_multisample_mask));
    break;

  case EggRenderMode::AM_binary:
    add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_binary));
    break;

  case EggRenderMode::AM_dual:
    add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_dual));
    break;

  default:
    break;
  }

  switch (dwm) {
  case EggRenderMode::DWM_on:
    add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_on));
    break;

  case EggRenderMode::DWM_off:
    add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off));
    break;

  default:
    break;
  }

  switch (dtm) {
  case EggRenderMode::DTM_on:
    add_attrib(DepthTestAttrib::make(DepthTestAttrib::M_less));
    break;

  case EggRenderMode::DTM_off:
    add_attrib(DepthTestAttrib::make(DepthTestAttrib::M_none));
    break;

  default:
    break;
  }

  switch (vm) {
  case EggRenderMode::VM_hidden:
    _hidden = true;
    break;

  case EggRenderMode::VM_normal:
  default:
    break;
  }

  _flat_shaded = 
    (egg_prim->get_connected_shading() == EggPrimitive::S_per_face);

  if (use_qpgeom) {
    if (_flat_shaded) {
      add_attrib(ShadeModelAttrib::make(ShadeModelAttrib::M_flat));
    }
  }

  if (egg_prim->is_of_type(EggLine::get_class_type())) {
    _primitive_type = qpGeomPrimitive::PT_lines;
    EggLine *egg_line = DCAST(EggLine, egg_prim);
    if (egg_line->get_thick() != 1.0) {
      add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_unchanged, 
                                        egg_line->get_thick()));
    }
  } else if (egg_prim->is_of_type(EggPoint::get_class_type())) {
    _primitive_type = qpGeomPrimitive::PT_points;
    EggPoint *egg_point = DCAST(EggPoint, egg_prim);
    if (egg_point->get_thick() != 1.0 || egg_point->get_perspective()) {
      add_attrib(RenderModeAttrib::make(RenderModeAttrib::M_unchanged, 
                                        egg_point->get_thick(),
                                        egg_point->get_perspective()));
    }
  } else {
    _primitive_type = qpGeomPrimitive::PT_polygons;
  }

  if (has_bin) {
    add_attrib(CullBinAttrib::make(bin, draw_order));

  } else if (has_draw_order) {
    add_attrib(CullBinAttrib::make("fixed", draw_order));
  }
 

  if (egg_prim->get_bface_flag()) {
    // The primitive is marked with backface culling disabled--we want
    // to see both sides.
    add_attrib(CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderState::get_material_attrib
//       Access: Private
//  Description: Returns a RenderAttrib suitable for enabling the
//               material indicated by the given EggMaterial, and with
//               the indicated backface flag.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) EggRenderState::
get_material_attrib(const EggMaterial *egg_mat, bool bface) {
  Materials &materials = 
    bface ? _loader._materials_bface : _loader._materials;

  // First, check whether we've seen this material before.
  Materials::const_iterator mi;
  mi = materials.find(egg_mat);
  if (mi != materials.end()) {
    return (*mi).second;
  }

  // Ok, this is the first time we've seen this particular
  // EggMaterial.  Create a new Material that matches it.
  PT(Material) mat = new Material;
  if (egg_mat->has_diff()) {
    mat->set_diffuse(egg_mat->get_diff());
    // By default, ambient is the same as diffuse, if diffuse is
    // specified but ambient is not.
    mat->set_ambient(egg_mat->get_diff());
  }
  if (egg_mat->has_amb()) {
    mat->set_ambient(egg_mat->get_amb());
  }
  if (egg_mat->has_emit()) {
    mat->set_emission(egg_mat->get_emit());
  }
  if (egg_mat->has_spec()) {
    mat->set_specular(egg_mat->get_spec());
  }
  if (egg_mat->has_shininess()) {
    mat->set_shininess(egg_mat->get_shininess());
  }
  if (egg_mat->has_local()) {
    mat->set_local(egg_mat->get_local());
  }

  mat->set_twoside(bface);

  // Now get a global Material pointer, shared with other models.
  const Material *shared_mat = MaterialPool::get_material(mat);

  // And create a MaterialAttrib for this Material.
  CPT(RenderAttrib) mt = MaterialAttrib::make(shared_mat);
  materials.insert(Materials::value_type(egg_mat, mt));

  return mt;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderState::get_tex_gen
//       Access: Private, Static
//  Description: Extracts the tex_gen from the given egg texture,
//               and returns its corresponding TexGenAttrib mode.
////////////////////////////////////////////////////////////////////
TexGenAttrib::Mode EggRenderState::
get_tex_gen(const EggTexture *egg_tex) {
  switch (egg_tex->get_tex_gen()) {
  case EggTexture::TG_unspecified:
    return TexGenAttrib::M_off;

  case EggTexture::TG_eye_sphere_map:
    return TexGenAttrib::M_eye_sphere_map;

  case EggTexture::TG_world_cube_map:
    return TexGenAttrib::M_world_cube_map;

  case EggTexture::TG_eye_cube_map:
    return TexGenAttrib::M_eye_cube_map;

  case EggTexture::TG_world_normal:
    return TexGenAttrib::M_world_normal;

  case EggTexture::TG_eye_normal:
    return TexGenAttrib::M_eye_normal;

  case EggTexture::TG_world_position:
    return TexGenAttrib::M_world_position;

  case EggTexture::TG_object_position:
    return TexGenAttrib::M_object_position;

  case EggTexture::TG_eye_position:
    return TexGenAttrib::M_eye_position;

  case EggTexture::TG_point_sprite:
    return TexGenAttrib::M_point_sprite;
  };

  return TexGenAttrib::M_off;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderState::apply_tex_mat
//       Access: Private, Static
//  Description: Applies the texture matrix from the indicated egg
//               texture to the given TexMatrixAttrib, and returns the
//               new attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) EggRenderState::
apply_tex_mat(CPT(RenderAttrib) tex_mat_attrib, 
              TextureStage *stage, const EggTexture *egg_tex) {
  if (egg_tex->has_transform()) {
    const LMatrix3d &tex_mat = egg_tex->get_transform();
    LMatrix4f mat4(tex_mat(0, 0), tex_mat(0, 1), 0.0f, tex_mat(0, 2),
                   tex_mat(1, 0), tex_mat(1, 1), 0.0f, tex_mat(1, 2),
                   0.0f, 0.0f, 1.0f, 0.0f,
                   tex_mat(2, 0), tex_mat(2, 1), 0.0f, tex_mat(2, 2));
    CPT(TransformState) transform;

    LVecBase3f scale, shear, hpr, translate;
    if (decompose_matrix(mat4, scale, shear, hpr, translate)) {
      // If the texture matrix can be represented componentwise, do
      // so.
      transform = TransformState::make_pos_hpr_scale_shear
        (translate, hpr, scale, shear);

    } else {
      // Otherwise, make a matrix transform.
      transform = TransformState::make_mat(mat4);
    }
  
    if (tex_mat_attrib == (const RenderAttrib *)NULL) {
      tex_mat_attrib = TexMatrixAttrib::make();
    }
    tex_mat_attrib = DCAST(TexMatrixAttrib, tex_mat_attrib)->
      add_stage(stage, transform);
  }
    
  return tex_mat_attrib;
}
