/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureReference.cxx
 * @author drose
 * @date 2000-11-29
 */

#include "textureReference.h"
#include "textureImage.h"
#include "paletteImage.h"
#include "sourceTextureImage.h"
#include "destTextureImage.h"
#include "texturePlacement.h"
#include "palettizer.h"
#include "eggFile.h"

#include "indent.h"
#include "eggTexture.h"
#include "eggData.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggNurbsSurface.h"
#include "eggVertexPool.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"

#include <math.h>

using std::max;
using std::min;
using std::string;

TypeHandle TextureReference::_type_handle;

/**
 *
 */
TextureReference::
TextureReference() {
  _egg_file = nullptr;
  _egg_tex = nullptr;
  _tex_mat = LMatrix3d::ident_mat();
  _inv_tex_mat = LMatrix3d::ident_mat();
  _source_texture = nullptr;
  _placement = nullptr;
  _uses_alpha = false;
  _any_uvs = false;
  _min_uv.set(0.0, 0.0);
  _max_uv.set(0.0, 0.0);
  _wrap_u = EggTexture::WM_unspecified;
  _wrap_v = EggTexture::WM_unspecified;
}

/**
 *
 */
TextureReference::
~TextureReference() {
  clear_placement();
}

/**
 * Sets up the TextureReference using information extracted from an egg file.
 */
void TextureReference::
from_egg(EggFile *egg_file, EggData *data, EggTexture *egg_tex) {
  _egg_file = egg_file;
  _egg_tex = egg_tex;
  _egg_data = data;
  _tref_name = egg_tex->get_name();

  if (_egg_tex->has_transform2d()) {
    _tex_mat = _egg_tex->get_transform2d();
    if (!_inv_tex_mat.invert_from(_tex_mat)) {
      _inv_tex_mat = LMatrix3d::ident_mat();
    }
  } else {
    _tex_mat = LMatrix3d::ident_mat();
    _inv_tex_mat = LMatrix3d::ident_mat();
  }

  Filename filename = _egg_tex->get_filename();
  Filename alpha_filename;
  if (_egg_tex->has_alpha_filename()) {
    alpha_filename = _egg_tex->get_alpha_filename();
  }
  int alpha_file_channel = _egg_tex->get_alpha_file_channel();

  _properties._format = _egg_tex->get_format();
  _properties._minfilter = _egg_tex->get_minfilter();
  _properties._magfilter = _egg_tex->get_magfilter();
  _properties._quality_level = _egg_tex->get_quality_level();
  _properties._anisotropic_degree = _egg_tex->get_anisotropic_degree();

  string name = filename.get_basename_wo_extension();
  TextureImage *texture = pal->get_texture(name);
  if (texture->get_name() != name) {
    nout << "Texture name conflict: \"" << name
         << "\" conflicts with existing texture named \""
         << texture->get_name() << "\".\n";

    // Make this a hard error; refuse to do anything else until the user fixes
    // it.  Case conflicts can be very bad, especially if CVS is involved on a
    // Windows machine.
    exit(1);
  }
  _source_texture = texture->get_source(filename, alpha_filename,
                                        alpha_file_channel);
  _source_texture->update_properties(_properties);

  _uses_alpha = false;
  EggRenderMode::AlphaMode alpha_mode = _egg_tex->get_alpha_mode();
  if (alpha_mode == EggRenderMode::AM_unspecified) {
    if (_source_texture->get_size()) {
      _uses_alpha =
        _egg_tex->has_alpha_channel(_source_texture->get_num_channels());
    }

  } else if (alpha_mode == EggRenderMode::AM_off) {
    _uses_alpha = false;

  } else {
    _uses_alpha = true;
  }

  get_uv_range(_egg_data, pal->_remap_uv);

  _wrap_u = _egg_tex->determine_wrap_u();
  _wrap_v = _egg_tex->determine_wrap_v();
}

/**
 * Sets up the pointers within the TextureReference to the same egg file
 * pointers indicated by the other TextureReference object, without changing
 * any of the other internal data stored here regarding the egg structures.
 * This is intended for use when we have already shown that the two
 * TextureReferences describe equivalent data.
 */
void TextureReference::
from_egg_quick(const TextureReference &other) {
  nassertv(_tref_name == other._tref_name);
  _egg_file = other._egg_file;
  _egg_tex = other._egg_tex;
  _egg_data = other._egg_data;
}

/**
 * Called to indicate that the EggData previously passed to from_egg() is
 * about to be deallocated, and all of its pointers should be cleared.
 */
void TextureReference::
release_egg_data() {
  _egg_tex = nullptr;
  _egg_data = nullptr;
}

/**
 * After an EggData has previously been released via release_egg_data(), this
 * can be called to indicate that the egg file has been reloaded and we should
 * assign the indicated pointers.
 */
void TextureReference::
rebind_egg_data(EggData *data, EggTexture *egg_tex) {
  nassertv(_tref_name == egg_tex->get_name());
  _egg_data = data;
  _egg_tex = egg_tex;
}

/**
 * Returns the EggFile that references this texture.
 */
EggFile *TextureReference::
get_egg_file() const {
  return _egg_file;
}

/**
 * Returns the SourceTextureImage that this object refers to.
 */
SourceTextureImage *TextureReference::
get_source() const {
  return _source_texture;
}

/**
 * Returns the TextureImage that this object refers to.
 */
TextureImage *TextureReference::
get_texture() const {
  nassertr(_source_texture != nullptr, nullptr);
  return _source_texture->get_texture();
}

/**
 * Returns the name of the EggTexture entry that references this texture.
 */
const string &TextureReference::
get_tref_name() const {
  return _tref_name;
}

/**
 * Defines an ordering of TextureReference pointers in alphabetical order by
 * their tref name.
 */
bool TextureReference::
operator < (const TextureReference &other) const {
  return _tref_name < other._tref_name;
}

/**
 * Returns true if this TextureReference actually uses the texture on
 * geometry, with UV's and everything, or false otherwise.  Strictly speaking,
 * this should always return true.
 */
bool TextureReference::
has_uvs() const {
  return _any_uvs;
}

/**
 * Returns the minimum UV coordinate in use for the texture by this reference.
 */
const LTexCoordd &TextureReference::
get_min_uv() const {
  nassertr(_any_uvs, _min_uv);
  return _min_uv;
}

/**
 * Returns the maximum UV coordinate in use for the texture by this reference.
 */
const LTexCoordd &TextureReference::
get_max_uv() const {
  nassertr(_any_uvs, _max_uv);
  return _max_uv;
}

/**
 * Returns the specification for the wrapping in the U direction.
 */
EggTexture::WrapMode TextureReference::
get_wrap_u() const {
  return _wrap_u;
}

/**
 * Returns the specification for the wrapping in the V direction.
 */
EggTexture::WrapMode TextureReference::
get_wrap_v() const {
  return _wrap_v;
}

/**
 * Returns true if all essential properties of this TextureReference are the
 * same as that of the other, or false if any of them differ.  This is useful
 * when reading a new egg file and comparing its references to its previously-
 * defined references.
 */
bool TextureReference::
is_equivalent(const TextureReference &other) const {
  if (_source_texture != other._source_texture) {
    return false;
  }
  if (!_properties.egg_properties_match(other._properties)) {
    return false;
  }
  if (_uses_alpha != other._uses_alpha) {
    return false;
  }
  if (_any_uvs != other._any_uvs) {
    return false;
  }
  if (_wrap_u != other._wrap_u ||
      _wrap_v != other._wrap_v) {
    return false;
  }
  if (_any_uvs) {
    if (!_min_uv.almost_equal(other._min_uv, 0.00001)) {
      return false;
    }
    if (!_max_uv.almost_equal(other._max_uv, 0.00001)) {
      return false;
    }
  }
  if (!_tex_mat.almost_equal(other._tex_mat, 0.00001)) {
    return false;
  }

  return true;
}

/**
 * Sets the particular TexturePlacement that is appropriate for this egg file.
 * This is called by EggFile::choose_placements().
 */
void TextureReference::
set_placement(TexturePlacement *placement) {
  if (_placement != placement) {
    if (_placement != nullptr) {
      // Remove our reference from the old placement object.
      _placement->remove_egg(this);
    }
    _placement = placement;
    if (_placement != nullptr) {
      // Add our reference to the new placement object.
      _placement->add_egg(this);
    }
  }
}

/**
 * Removes any reference to a TexturePlacement.
 */
void TextureReference::
clear_placement() {
  set_placement(nullptr);
}

/**
 * Returns the particular TexturePlacement that is appropriate for this egg
 * file.  This will not be filled in until EggFile::choose_placements() has
 * been called.
 */
TexturePlacement *TextureReference::
get_placement() const {
  return _placement;
}

/**
 * Marks the egg file that shares this reference as stale.
 */
void TextureReference::
mark_egg_stale() {
  if (_egg_file != nullptr) {
    _egg_file->mark_stale();
  }
}

/**
 * Updates the egg file with all the relevant information to reference the
 * texture in its new home, wherever that might be.
 */
void TextureReference::
update_egg() {
  if (_egg_tex == nullptr) {
    // Not much we can do if we don't have an actual egg file to reference.
    return;
  }

  if (_placement == nullptr) {
    // Nor if we don't have an actual placement yet.  This is possible if the
    // egg was assigned to the "null" group, and the texture hasn't been re-
    // assigned yet.
    return;
  }

  TextureImage *texture = get_texture();
  if (texture != nullptr) {
    // Make sure the alpha mode is set according to what the texture image
    // wants.
    if (texture->has_num_channels() &&
        !_egg_tex->has_alpha_channel(texture->get_num_channels())) {
      // The egg file doesn't want to use the alpha on the texture; leave it
      // unspecified so the egg loader can figure out whether to enable alpha
      // or not based on the object color.
      _egg_tex->set_alpha_mode(EggRenderMode::AM_unspecified);

    } else {
      // The egg file does want alpha, so get the alpha mode from the texture.
      EggRenderMode::AlphaMode am = texture->get_alpha_mode();
      if (am != EggRenderMode::AM_unspecified) {
        _egg_tex->set_alpha_mode(am);
      }
    }

    // Also make sure the wrap mode is set properly.
    if (texture->get_txa_wrap_u() != EggTexture::WM_unspecified) {
      _egg_tex->set_wrap_u(texture->get_txa_wrap_u());
    }
    if (texture->get_txa_wrap_v() != EggTexture::WM_unspecified) {
      _egg_tex->set_wrap_v(texture->get_txa_wrap_v());
    }
  }

  // We check for an OmitReason of OR_none, rather than asking is_placed(),
  // because in this case we don't want to consider an OR_solitary texture as
  // having been placed.
  if (_placement->get_omit_reason() == OR_unknown) {
    // The texture doesn't even exist.  We can't update the egg to point to
    // any meaningful path; just leave it pointing to the source texture's
    // basename.  Maybe it will be found along the texture path later.
    Filename orig_filename = _egg_tex->get_filename();
    texture->update_egg_tex(_egg_tex);
    _egg_tex->set_filename(orig_filename.get_basename());
    return;
  }
  if (_placement->get_omit_reason() != OR_none) {
    // The texture exists but is not on a palette.  This is the easy case; we
    // simply have to update the texture reference to the new texture
    // location.
    DestTextureImage *dest = _placement->get_dest();
    nassertv(dest != nullptr);
    dest->update_egg_tex(_egg_tex);
    return;
  }

  // The texture *does* appear on a palette.  This means we need to not only
  // update the texture reference, but also adjust the UV's.  In most cases,
  // we can do this by simply applying a texture matrix to the reference.
  PaletteImage *image = _placement->get_image();
  nassertv(image != nullptr);

  image->update_egg_tex(_egg_tex);

  // Palette images never wrap, so the wrap mode doesn't matter.  We let this
  // default to unspecified, which means the images will wrap by default,
  // which is the fastest mode for tinydisplay anyway.
  _egg_tex->set_wrap_mode(EggTexture::WM_unspecified);
  _egg_tex->set_wrap_u(EggTexture::WM_unspecified);
  _egg_tex->set_wrap_v(EggTexture::WM_unspecified);

  LMatrix3d new_tex_mat;
  _placement->compute_tex_matrix(new_tex_mat);

  // Compose the new texture matrix with whatever matrix was already there, if
  // any.
  _egg_tex->set_transform2d(_tex_mat * new_tex_mat);

  // Finally, go back and actually adjust the UV's to match what we claimed
  // they could be.
  if (_egg_tex->get_tex_gen() == EggTexture::TG_unspecified) {
    update_uv_range(_egg_data, pal->_remap_uv);
  }
}

/**
 * Applies the texture properties as read from the egg file to the source
 * image's properties.  This updates the source image with the now-known
 * properties indicated with in the tref block of the egg file.
 */
void TextureReference::
apply_properties_to_source() {
  nassertv(_source_texture != nullptr);
  _source_texture->update_properties(_properties);
}

/**
 *
 */
void TextureReference::
output(std::ostream &out) const {
  out << *_source_texture;
}

/**
 *
 */
void TextureReference::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_texture()->get_name();

  if (_uses_alpha) {
    out << " (uses alpha)";
  }

  if (_any_uvs) {
    // Compute the fraction of the image that is covered by the UV's minmax
    // rectangle.
    LTexCoordd box = _max_uv - _min_uv;
    double area = box[0] * box[1];

    out << " coverage " << area;
  }

  if (_wrap_u != EggTexture::WM_unspecified ||
      _wrap_v != EggTexture::WM_unspecified) {
    if (_wrap_u != _wrap_v) {
      out << " (" << _wrap_u << ", " << _wrap_v << ")";
    } else {
      out << " " << _wrap_u;
    }
  }

  if (_properties._format != EggTexture::F_unspecified) {
    out << " " << _properties._format;
  }

  switch (_properties._minfilter) {
      case EggTexture::FT_nearest_mipmap_nearest:
      case EggTexture::FT_linear_mipmap_nearest:
      case EggTexture::FT_nearest_mipmap_linear:
      case EggTexture::FT_linear_mipmap_linear:
        out << " mipmap";
        break;

      default:
        break;
  }

  if(_properties._anisotropic_degree>1) {
        out << " aniso " << _properties._anisotropic_degree;
  }

  out << "\n";
}


/**
 * Checks the geometry in the egg file to see what range of UV's are requested
 * for this particular texture reference.
 *
 * If pal->_remap_uv is not RU_never, this will also attempt to remap the UV's
 * found so that the midpoint lies in the unit square (0,0) - (1,1), in the
 * hopes of maximizing overlap of UV coordinates between different polygons.
 * However, the hypothetical translations are not actually applied to the egg
 * file at this point (because we might decide not to place the texture in a
 * palette); they will actually be applied when update_uv_range(), below, is
 * called later.
 *
 * The return value is true if the search should continue, or false if it
 * should abort prematurely.
 */
bool TextureReference::
get_uv_range(EggGroupNode *group, Palettizer::RemapUV remap) {
  if (group->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group;
    DCAST_INTO_R(egg_group, group, false);

    if (egg_group->get_dart_type() != EggGroup::DT_none) {
      // If it's a character, we might change the kind of remapping we do.
      remap = pal->_remap_char_uv;
    }
  }

  bool group_any_uvs = false;
  LTexCoordd group_min_uv, group_max_uv;

  EggGroupNode::iterator ci;
  for (ci = group->begin(); ci != group->end(); ci++) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggNurbsSurface::get_class_type())) {
      EggNurbsSurface *nurbs = DCAST(EggNurbsSurface, child);
      if (nurbs->has_texture(_egg_tex)) {
        // Here's a NURBS surface that references the texture.  Unlike other
        // kinds of geometries, NURBS don't store UV's; they're implicit in
        // the surface.  NURBS UV's will always run in the range (0, 0) - (1,
        // 1).  However, we do need to apply the texture matrix.

        // We also don't count the NURBS surfaces in with the group's UV's,
        // because we can't adjust the UV's on a NURBS, so counting them up
        // would be misleading (the reason we count up the group UV's is so we
        // can consider adjusting them later).  Instead, we just accumulate
        // the NURBS UV's directly into our total.
        collect_nominal_uv_range();
      }

    } else if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *geom = DCAST(EggPrimitive, child);
      if (geom->has_texture(_egg_tex)) {
        // Here's a piece of geometry that references this texture.  Walk
        // through its vertices and get its UV's.

        if (_egg_tex->get_tex_gen() != EggTexture::TG_unspecified) {
          // If the texture has a TexGen mode, we don't check the UV range on
          // the model, since that doesn't matter.  Instead, we assume the
          // texture is used in the range (0, 0) - (1, 1), which will be true
          // for a sphere map, although the effective range is a little less
          // clear for the TG_world_position and similar modes.
          collect_nominal_uv_range();

          // In fact, now we can return, having found at least one model that
          // references the texture; there's no need to search further.
          return false;

        } else {
          LTexCoordd geom_min_uv, geom_max_uv;

          if (get_geom_uvs(geom, geom_min_uv, geom_max_uv)) {
            if (remap == Palettizer::RU_poly) {
              LVector2d trans = translate_uv(geom_min_uv, geom_max_uv);
              geom_min_uv += trans;
              geom_max_uv += trans;
            }
            collect_uv(group_any_uvs, group_min_uv, group_max_uv,
                       geom_min_uv, geom_max_uv);
          }
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *cg = DCAST(EggGroupNode, child);
      if (!get_uv_range(cg, remap)) {
        return false;
      }
    }
  }

  if (group_any_uvs) {
    if (remap == Palettizer::RU_group) {
      LVector2d trans = translate_uv(group_min_uv, group_max_uv);
      group_min_uv += trans;
      group_max_uv += trans;
    }
    collect_uv(_any_uvs, _min_uv, _max_uv, group_min_uv, group_max_uv);
  }

  return true;
}

/**
 * Actually applies the UV translates that were assumed in the previous call
 * to get_uv_range().
 */
void TextureReference::
update_uv_range(EggGroupNode *group, Palettizer::RemapUV remap) {
  if (group->is_of_type(EggGroup::get_class_type())) {
    EggGroup *egg_group;
    DCAST_INTO_V(egg_group, group);

    if (egg_group->get_dart_type() != EggGroup::DT_none) {
      // If it's a character, we might change the kind of remapping we do.
      remap = pal->_remap_char_uv;
    }
  }

  bool group_any_uvs = false;
  LTexCoordd group_min_uv, group_max_uv;

  EggGroupNode::iterator ci;
  for (ci = group->begin(); ci != group->end(); ci++) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggNurbsSurface::get_class_type())) {
      // We do nothing at this point for a Nurbs.  Nothing we can do about
      // these things.

    } else if (child->is_of_type(EggPrimitive::get_class_type())) {
      if (remap != Palettizer::RU_never) {
        EggPrimitive *geom = DCAST(EggPrimitive, child);
        if (geom->has_texture(_egg_tex)) {
          LTexCoordd geom_min_uv, geom_max_uv;

          if (get_geom_uvs(geom, geom_min_uv, geom_max_uv)) {
            if (remap == Palettizer::RU_poly) {
              LVector2d trans = translate_uv(geom_min_uv, geom_max_uv);
              trans = trans * _inv_tex_mat;
              if (!trans.almost_equal(LVector2d::zero())) {
                translate_geom_uvs(geom, trans);
              }
            } else {
              collect_uv(group_any_uvs, group_min_uv, group_max_uv,
                         geom_min_uv, geom_max_uv);
            }
          }
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *cg = DCAST(EggGroupNode, child);
      update_uv_range(cg, remap);
    }
  }

  if (group_any_uvs && remap == Palettizer::RU_group) {
    LVector2d trans = translate_uv(group_min_uv, group_max_uv);
    trans = trans * _inv_tex_mat;
    if (!trans.almost_equal(LVector2d::zero())) {
      for (ci = group->begin(); ci != group->end(); ci++) {
        EggNode *child = (*ci);
        if (child->is_of_type(EggPrimitive::get_class_type())) {
          EggPrimitive *geom = DCAST(EggPrimitive, child);
          if (geom->has_texture(_egg_tex)) {
            translate_geom_uvs(geom, trans);
          }
        }
      }
    }
  }
}

/**
 * Determines the minimum and maximum UV range for a particular primitive.
 * Returns true if it has any UV's, false otherwise.
 */
bool TextureReference::
get_geom_uvs(EggPrimitive *geom,
             LTexCoordd &geom_min_uv, LTexCoordd &geom_max_uv) {
  string uv_name = _egg_tex->get_uv_name();
  bool geom_any_uvs = false;

  EggPrimitive::iterator pi;
  for (pi = geom->begin(); pi != geom->end(); ++pi) {
    EggVertex *vtx = (*pi);
    if (vtx->has_uv(uv_name)) {
      LTexCoordd uv = vtx->get_uv(uv_name) * _tex_mat;
      collect_uv(geom_any_uvs, geom_min_uv, geom_max_uv, uv, uv);
    }
  }

  return geom_any_uvs;
}

/**
 * Applies the indicated translation to each UV in the primitive.
 */
void TextureReference::
translate_geom_uvs(EggPrimitive *geom, const LTexCoordd &trans) const {
  string uv_name = _egg_tex->get_uv_name();

  EggPrimitive::iterator pi;
  for (pi = geom->begin(); pi != geom->end(); ++pi) {
    EggVertex *vtx = (*pi);
    if (vtx->has_uv(uv_name)) {
      EggVertex vtx_copy(*vtx);
      vtx_copy.set_uv(uv_name, vtx_copy.get_uv(uv_name) + trans);
      EggVertex *new_vtx = vtx->get_pool()->create_unique_vertex(vtx_copy);

      if (new_vtx->gref_size() != vtx->gref_size()) {
        new_vtx->copy_grefs_from(*vtx);
      }

      geom->replace(pi, new_vtx);
    }
  }
}

/**
 * Updates _any_uvs, _min_uv, and _max_uv with the range (0, 0) - (1, 1),
 * adjusted by the texture matrix.
 */
void TextureReference::
collect_nominal_uv_range() {
  static const int num_nurbs_uvs = 4;
  static LTexCoordd nurbs_uvs[num_nurbs_uvs] = {
    LTexCoordd(0.0, 0.0),
    LTexCoordd(0.0, 1.0),
    LTexCoordd(1.0, 1.0),
    LTexCoordd(1.0, 0.0)
  };

  for (int i = 0; i < num_nurbs_uvs; i++) {
    LTexCoordd uv = nurbs_uvs[i] * _tex_mat;
    collect_uv(_any_uvs, _min_uv, _max_uv, uv, uv);
  }
}

/**
 * Updates any_uvs, min_uv, and max_uv with the indicated min and max UV's
 * already determined.
 */
void TextureReference::
collect_uv(bool &any_uvs, LTexCoordd &min_uv, LTexCoordd &max_uv,
           const LTexCoordd &got_min_uv, const LTexCoordd &got_max_uv) {
  if (any_uvs) {
    min_uv.set(min(min_uv[0], got_min_uv[0]),
               min(min_uv[1], got_min_uv[1]));
    max_uv.set(max(max_uv[0], got_max_uv[0]),
               max(max_uv[1], got_max_uv[1]));
  } else {
    // The first UV.
    min_uv = got_min_uv;
    max_uv = got_max_uv;
    any_uvs = true;
  }
}

/**
 * Returns the needed adjustment to translate the given bounding box so that
 * its center lies in the unit square (0,0) - (1,1).
 */
LVector2d TextureReference::
translate_uv(const LTexCoordd &min_uv, const LTexCoordd &max_uv) {
  LTexCoordd center = (min_uv + max_uv) / 2;
  return LVector2d(-floor(center[0]), -floor(center[1]));
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void TextureReference::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_TextureReference);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void TextureReference::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  writer->write_pointer(datagram, _egg_file);

  // We don't write _egg_tex or _egg_data; that's specific to the session.

  datagram.add_string(_tref_name);

  _tex_mat.write_datagram(datagram);
  _inv_tex_mat.write_datagram(datagram);

  writer->write_pointer(datagram, _source_texture);
  writer->write_pointer(datagram, _placement);

  datagram.add_bool(_uses_alpha);
  datagram.add_bool(_any_uvs);
  datagram.add_float64(_min_uv[0]);
  datagram.add_float64(_min_uv[1]);
  datagram.add_float64(_max_uv[0]);
  datagram.add_float64(_max_uv[1]);
  datagram.add_int32((int)_wrap_u);
  datagram.add_int32((int)_wrap_v);
  _properties.write_datagram(writer, datagram);
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int TextureReference::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  if (p_list[pi] != nullptr) {
    DCAST_INTO_R(_egg_file, p_list[pi], pi);
  }
  pi++;

  if (p_list[pi] != nullptr) {
    DCAST_INTO_R(_source_texture, p_list[pi], pi);
  }
  pi++;

  if (p_list[pi] != nullptr) {
    DCAST_INTO_R(_placement, p_list[pi], pi);
  }
  pi++;

  pi += _properties.complete_pointers(p_list + pi, manager);

  return pi;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable* TextureReference::
make_TextureReference(const FactoryParams &params) {
  TextureReference *me = new TextureReference;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void TextureReference::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  manager->read_pointer(scan);  // _egg_file

  if (Palettizer::_read_pi_version >= 11) {
    _tref_name = scan.get_string();
  }

  _tex_mat.read_datagram(scan);
  _inv_tex_mat.read_datagram(scan);

  manager->read_pointer(scan);  // _source_texture
  manager->read_pointer(scan);  // _placement

  _uses_alpha = scan.get_bool();
  _any_uvs = scan.get_bool();
  _min_uv[0] = scan.get_float64();
  _min_uv[1] = scan.get_float64();
  _max_uv[0] = scan.get_float64();
  _max_uv[1] = scan.get_float64();
  _wrap_u = (EggTexture::WrapMode)scan.get_int32();
  _wrap_v = (EggTexture::WrapMode)scan.get_int32();
  _properties.fillin(scan, manager);
}
