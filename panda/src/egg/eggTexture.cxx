/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTexture.cxx
 * @author drose
 * @date 1999-01-18
 */

#include "eggTexture.h"
#include "eggMiscFuncs.h"
#include "lexerDefs.h"

#include "indent.h"
#include "string_utils.h"

using std::ostream;
using std::string;

TypeHandle EggTexture::_type_handle;


/**
 *
 */
EggTexture::
EggTexture(const string &tref_name, const Filename &filename)
  : EggFilenameNode(tref_name, filename)
{
  _texture_type = TT_unspecified;
  _format = F_unspecified;
  _compression_mode = CM_default;
  _wrap_mode = WM_unspecified;
  _wrap_u = WM_unspecified;
  _wrap_v = WM_unspecified;
  _wrap_w = WM_unspecified;
  _minfilter = FT_unspecified;
  _magfilter = FT_unspecified;
  _anisotropic_degree = 0;
  _env_type = ET_unspecified;
  _saved_result = false;
  _multiview = false;
  _num_views = 0;
  _tex_gen = TG_unspecified;
  _quality_level = QL_unspecified;
  _priority = 0;
  _color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _border_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _flags = 0;
  _alpha_file_channel = 0;
  _read_mipmaps = false;
  _multitexture_sort = 0;
}

/**
 *
 */
EggTexture::
EggTexture(const EggTexture &copy) {
  (*this) = copy;
}

/**
 *
 */
EggTexture &EggTexture::
operator = (const EggTexture &copy) {
  clear_multitexture();

  EggFilenameNode::operator = (copy);
  EggRenderMode::operator = (copy);
  EggTransform::operator = (copy);

  _texture_type = copy._texture_type;
  _format = copy._format;
  _compression_mode = copy._compression_mode;
  _wrap_mode = copy._wrap_mode;
  _wrap_u = copy._wrap_u;
  _wrap_v = copy._wrap_v;
  _wrap_w = copy._wrap_w;
  _minfilter = copy._minfilter;
  _magfilter = copy._magfilter;
  _anisotropic_degree = copy._anisotropic_degree;
  _env_type = copy._env_type;
  _saved_result = copy._saved_result;
  _multiview = copy._multiview;
  _num_views = copy._num_views;
  _tex_gen = copy._tex_gen;
  _quality_level = copy._quality_level;
  _stage_name = copy._stage_name;
  _priority = copy._priority;
  _color = copy._color;
  _border_color = copy._border_color;
  _uv_name = copy._uv_name;
  _rgb_scale = 1;
  _alpha_scale = 1;
  _flags = copy._flags;
  _alpha_filename = copy._alpha_filename;
  _alpha_fullpath = copy._alpha_fullpath;
  _alpha_file_channel = copy._alpha_file_channel;
  _read_mipmaps = copy._read_mipmaps;
  _multitexture_sort = 0;
  _combiner[0] = copy._combiner[0];
  _combiner[1] = copy._combiner[1];

  return *this;
}

/**
 *
 */
EggTexture::
~EggTexture() {
  clear_multitexture();
}

/**
 * Writes the texture definition to the indicated output stream in Egg format.
 */
void EggTexture::
write(ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Texture>");
  enquote_string(out, get_filename(), indent_level + 2) << "\n";

  if (has_alpha_filename()) {
    indent(out, indent_level + 2)
      << "<Scalar> alpha-file { ";
    enquote_string(out, get_alpha_filename());
    out << " }\n";
  }

  if (has_alpha_file_channel()) {
    indent(out, indent_level + 2)
      << "<Scalar> alpha-file-channel { "
      << get_alpha_file_channel() << " }\n";
  }

  if (get_read_mipmaps()) {
    indent(out, indent_level + 2)
      << "<Scalar> read-mipmaps { 1 }\n";
  }

  if (get_texture_type() != TT_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> type { " << get_texture_type() << " }\n";
  }

  if (get_format() != F_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> format { " << get_format() << " }\n";
  }

  if (get_compression_mode() != CM_default) {
    indent(out, indent_level + 2)
      << "<Scalar> compression { " << get_compression_mode() << " }\n";
  }

  if (get_wrap_mode() != WM_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> wrap { " << get_wrap_mode() << " }\n";
  }

  if (get_wrap_u() != WM_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> wrapu { " << get_wrap_u() << " }\n";
  }

  if (get_wrap_v() != WM_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> wrapv { " << get_wrap_v() << " }\n";
  }

  if (get_wrap_w() != WM_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> wrapw { " << get_wrap_w() << " }\n";
  }

  if (get_minfilter() != FT_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> minfilter { " << get_minfilter() << " }\n";
  }

  if (get_magfilter() != FT_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> magfilter { " << get_magfilter() << " }\n";
  }

  if (has_anisotropic_degree()) {
    indent(out, indent_level + 2)
      << "<Scalar> anisotropic-degree { " << get_anisotropic_degree() << " }\n";
  }

  if (get_env_type() != ET_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> envtype { " << get_env_type() << " }\n";
  }

  for (int ci = 0; ci < (int)CC_num_channels; ci++) {
    CombineChannel channel = (CombineChannel)ci;
    if (get_combine_mode(channel) != CM_unspecified) {
      indent(out, indent_level + 2)
        << "<Scalar> combine-" << channel
        << " { " << get_combine_mode(channel) << " }\n";
    }
    for (int i = 0; i < (int)CI_num_indices; i++) {
      if (get_combine_source(channel, i) != CS_unspecified) {
        indent(out, indent_level + 2)
          << "<Scalar> combine-" << channel << "-source" << i
          << " { " << get_combine_source(channel, i) << " }\n";
      }
      if (get_combine_operand(channel, i) != CO_unspecified) {
        indent(out, indent_level + 2)
          << "<Scalar> combine-" << channel << "-operand" << i
          << " { " << get_combine_operand(channel, i) << " }\n";
      }
    }
  }

  if (get_saved_result()) {
    indent(out, indent_level + 2)
      << "<Scalar> saved-result { 1 }\n";
  }

  if (get_tex_gen() != TG_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> tex-gen { " << get_tex_gen() << " }\n";
  }

  if (get_quality_level() != QL_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> quality-level { " << get_quality_level() << " }\n";
  }

  if (has_stage_name()) {
    indent(out, indent_level + 2)
      << "<Scalar> stage-name { " << get_stage_name() << " }\n";
  }

  if (has_priority()) {
    indent(out, indent_level + 2)
      << "<Scalar> priority { " << get_priority() << " }\n";
  }

  if (has_color()) {
    indent(out, indent_level + 2)
      << "<Scalar> blendr { " << _color[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> blendg { " << _color[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> blendb { " << _color[2] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> blenda { " << _color[3] << " }\n";
  }

  if (has_border_color()) {
    indent(out, indent_level + 2)
      << "<Scalar> borderr { " << _border_color[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> borderg { " << _border_color[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> borderb { " << _border_color[2] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> bordera { " << _border_color[3] << " }\n";
  }

  if (has_uv_name()) {
    indent(out, indent_level + 2)
      << "<Scalar> uv-name { " << get_uv_name() << " }\n";
  }

  if (has_rgb_scale()) {
    indent(out, indent_level + 2)
      << "<Scalar> rgb-scale { " << get_rgb_scale() << " }\n";
  }

  if (has_alpha_scale()) {
    indent(out, indent_level + 2)
      << "<Scalar> alpha-scale { " << get_alpha_scale() << " }\n";
  }

  if (get_multiview()) {
    indent(out, indent_level + 2)
      << "<Scalar> multiview { 1 }\n";
  }

  if (has_num_views()) {
    indent(out, indent_level + 2)
      << "<Scalar> num-views { " << get_num_views() << " }\n";
  }

  EggRenderMode::write(out, indent_level + 2);

  if (has_transform()) {
    EggTransform::write(out, indent_level + 2, "<Transform>");
  }

  indent(out, indent_level) << "}\n";
}

/**
 * Returns true if the two textures are equivalent in all relevant properties
 * (according to eq), false otherwise.
 *
 * The Equivalence parameter, eq, should be set to the bitwise OR of the
 * following properties, according to what you consider relevant:
 *
 * EggTexture::E_basename: The basename part of the texture filename, without
 * the directory prefix *or* the filename extension.
 *
 * EggTexture::E_extension: The extension part of the texture filename.
 *
 * EggTexture::E_dirname: The directory prefix of the texture filename.
 *
 * EggTexture::E_complete_filename: The union of the above three; that is, the
 * complete filename, with directory, basename, and extension.
 *
 * EggTexture::E_transform: The texture matrix.
 *
 * EggTexture::E_attributes: All remaining texture attributes (mode, mipmap,
 * etc.) except TRef name.
 *
 * EggTexture::E_tref_name: The TRef name.
 */
bool EggTexture::
is_equivalent_to(const EggTexture &other, int eq) const {
  if ((eq & E_complete_filename) == E_complete_filename) {
    // cout << "compared by filename" << endl;
    if (get_filename() != other.get_filename()) {
      return false;
    }
  } else {
    // cout << "compared by not complete filename" << endl;
    const Filename &a = get_filename();
    const Filename &b = other.get_filename();

    if (eq & E_basename) {
      if (a.get_basename_wo_extension() != b.get_basename_wo_extension()) {
        return false;
      }
    }
    if (eq & E_extension) {
      if (a.get_extension() != b.get_extension()) {
        return false;
      }
    }
    if (eq & E_dirname) {
      if (a.get_dirname() != b.get_dirname()) {
        return false;
      }
    }
  }

  if (eq & E_transform) {
    // cout << "compared by transform" << endl;
    if (transform_is_identity() != other.transform_is_identity()) {
      return false;
    }

    if (has_transform() && other.has_transform()) {
      if (!get_transform3d().almost_equal(other.get_transform3d(), 0.0001)) {
        return false;
      }
    }
  }

  if (eq & E_attributes) {
    // cout << "compared by attributes" << endl;
    if (_texture_type != other._texture_type ||
        _format != other._format ||
        _compression_mode != other._compression_mode ||
        _wrap_mode != other._wrap_mode ||
        _wrap_u != other._wrap_u ||
        _wrap_v != other._wrap_v ||
        _wrap_w != other._wrap_w ||
        _minfilter != other._minfilter ||
        _magfilter != other._magfilter ||
        _env_type != other._env_type) {
      return false;
    }
    if (EggRenderMode::operator != (other)) {
      return false;
    }
  }

  if (eq & E_tref_name) {
    // cout << "compared by tref_name" << endl;
    if (get_name() != other.get_name()) {
      return false;
    }
  }

  return true;
}

/**
 * An ordering operator to compare two textures for sorting order.  This
 * imposes an arbitrary ordering useful to identify unique textures, according
 * to the indicated Equivalence factor.  See is_equivalent_to().
 */
bool EggTexture::
sorts_less_than(const EggTexture &other, int eq) const {
  if ((eq & E_complete_filename) == E_complete_filename) {
    if (get_filename() != other.get_filename()) {
      return get_filename() < other.get_filename();
    }
  } else {
    const Filename &a = get_filename();
    const Filename &b = other.get_filename();

    if (eq & E_basename) {
      if (a.get_basename_wo_extension() != b.get_basename_wo_extension()) {
        return a.get_basename_wo_extension() < b.get_basename_wo_extension();
      }
    }
    if (eq & E_extension) {
      if (a.get_extension() != b.get_extension()) {
        return a.get_extension() < b.get_extension();
      }
    }
    if (eq & E_dirname) {
      if (a.get_dirname() != b.get_dirname()) {
        return a.get_dirname() < b.get_dirname();
      }
    }
  }

  if (eq & E_transform) {
    bool is_identity = transform_is_identity();
    bool other_is_identity = other.transform_is_identity();
    if (is_identity != other_is_identity) {
      return (int)is_identity < (int)other_is_identity;
    }

    if (has_transform() && other.has_transform()) {
      int compare = get_transform3d().compare_to(other.get_transform3d());
      if (compare != 0) {
        return compare < 0;
      }
    }
  }

  if (eq & E_attributes) {
    if (_texture_type != other._texture_type) {
      return (int)_texture_type < (int)other._texture_type;
    }
    if (_format != other._format) {
      return (int)_format < (int)other._format;
    }
    if (_compression_mode != other._compression_mode) {
      return (int)_compression_mode < (int)other._compression_mode;
    }
    if (_wrap_mode != other._wrap_mode) {
      return (int)_wrap_mode < (int)other._wrap_mode;
    }
    if (_wrap_u != other._wrap_u) {
      return (int)_wrap_u < (int)other._wrap_u;
    }
    if (_wrap_v != other._wrap_v) {
      return (int)_wrap_v < (int)other._wrap_v;
    }
    if (_wrap_w != other._wrap_w) {
      return (int)_wrap_w < (int)other._wrap_w;
    }
    if (_minfilter != other._minfilter) {
      return (int)_minfilter < (int)other._minfilter;
    }
    if (_magfilter != other._magfilter) {
      return (int)_magfilter < (int)other._magfilter;
    }
    if (_anisotropic_degree != other._anisotropic_degree) {
      return _anisotropic_degree < other._anisotropic_degree;
    }
    if (_env_type != other._env_type) {
      return (int)_env_type < (int)other._env_type;
    }
    if (EggRenderMode::operator != (other)) {
      return EggRenderMode::operator < (other);
    }
  }

  if (eq & E_tref_name) {
    if (get_name() != other.get_name()) {
      return get_name() < other.get_name();
    }
  }

  return false;
}

/**
 * Given the number of color components (channels) in the image file as
 * actually read from the disk, return true if this texture seems to have an
 * alpha channel or not.  This depends on the EggTexture's format as well as
 * the number of channels.
 */
bool EggTexture::
has_alpha_channel(int num_components) const {
  switch (_format) {
  case F_red:
  case F_green:
  case F_blue:
  case F_luminance:
  case F_rgb:
  case F_rgb12:
  case F_rgb8:
  case F_rgb5:
  case F_rgb332:
    // These formats never use alpha, regardless of the number of components
    // we have.
    return false;

  case F_alpha:
    // This format always uses alpha.
    return true;

  case F_luminance_alpha:
  case F_luminance_alphamask:
  case F_rgba:
  case F_rgbm:
  case F_rgba12:
  case F_rgba8:
  case F_rgba4:
  case F_rgba5:
  case F_unspecified:
    // These formats use alpha if the image had alpha.
    return (num_components == 2 || num_components == 4);
  }

  return false;
}

/**
 * Returns true if this texture's environment type or combine mode allows the
 * texture to have an effect on the polygon's alpha values, false otherwise.
 */
bool EggTexture::
affects_polygon_alpha() const {
  switch (_env_type) {
  case ET_modulate:
  case ET_replace:
    return true;

  case ET_decal:
  case ET_blend:
  case ET_add:
  case ET_blend_color_scale:
  case ET_modulate_glow:
  case ET_modulate_gloss:
  case ET_normal:
  case ET_normal_height:
  case ET_glow:
  case ET_gloss:
  case ET_height:
  case ET_normal_gloss:
    return false;

  case ET_selector:
    return true;

  case ET_unspecified:
    break;
  }

  switch (_combiner[CC_alpha]._mode) {
  case CM_replace:
  case CM_modulate:
  case CM_add_signed:
  case CM_subtract:
    return true;

  case CM_interpolate:
  case CM_add:
  case CM_dot3_rgb:
  case CM_dot3_rgba:
    return false;

  case CM_unspecified:
    break;
  }

  // A completely unspecified texture environment implies "modulate", which
  // does affect alpha.
  return true;
}


/**
 * Resets the multitexture flags set by multitexture_over().  After this call,
 * get_multitexture() will return false, and get_multitexture_sort() will
 * return 0.
 */
void EggTexture::
clear_multitexture() {
  _multitexture_sort = 0;

  // Now empty out the _over_textures and _under_textures sets.  This requires
  // a bit of care so we don't end up in mutual recursion or iterating through
  // self-modifying structures.  To avoid this, we empty the sets first, and
  // then walk through their original contents.
  MultiTextures orig_over_textures, orig_under_textures;
  orig_over_textures.swap(_over_textures);
  orig_under_textures.swap(_under_textures);

  MultiTextures::iterator mti;
  for (mti = orig_over_textures.begin();
       mti != orig_over_textures.end();
       ++mti) {
    EggTexture *other = (*mti);
    other->_under_textures.erase(this);
  }
  for (mti = orig_under_textures.begin();
       mti != orig_under_textures.end();
       ++mti) {
    EggTexture *other = (*mti);
    other->_over_textures.erase(this);
  }
}

/**
 * Indicates that this texture should be layered on top of the other texture.
 * This will guarantee that this->get_multitexture_sort() >
 * other->get_multitexture_sort(), at least until clear_multitexture() is
 * called on either one.
 *
 * The return value is true if successful, or false if there is a failure
 * because the other texture was already layered on top of this one (or there
 * is a three- or more-way cycle).
 */
bool EggTexture::
multitexture_over(EggTexture *other) {
  if (get_multitexture_sort() <= other->get_multitexture_sort()) {
    MultiTextures cycle_detector;
    if (!r_min_multitexture_sort(other->get_multitexture_sort() + 1,
                                 cycle_detector)) {
      // Found a cycle right off the bat!
      return false;
    }
  }

  if (_over_textures.insert(other).second) {
    bool inserted_under = other->_under_textures.insert(this).second;
    nassertr(inserted_under, false);
  }
  nassertr(get_multitexture_sort() > other->get_multitexture_sort(), false);

  return true;
}

/**
 * Returns the Texture_ype value associated with the given string
 * representation, or TT_unspecified if the string does not match any known
 * TextureType value.
 */
EggTexture::TextureType EggTexture::
string_texture_type(const string &string) {
  if (cmp_nocase_uh(string, "1d") == 0 ||
      cmp_nocase_uh(string, "1dtexture") == 0 ||
      cmp_nocase_uh(string, "1d_texture") == 0) {
    return TT_1d_texture;

  } else if (cmp_nocase_uh(string, "2d") == 0 ||
             cmp_nocase_uh(string, "2dtexture") == 0 ||
             cmp_nocase_uh(string, "2d_texture") == 0) {
    return TT_2d_texture;

  } else if (cmp_nocase_uh(string, "3d") == 0 ||
             cmp_nocase_uh(string, "3dtexture") == 0 ||
             cmp_nocase_uh(string, "3d_texture") == 0) {
    return TT_3d_texture;

  } else if (cmp_nocase_uh(string, "cube") == 0 ||
             cmp_nocase_uh(string, "cubemap") == 0 ||
             cmp_nocase_uh(string, "cube_map") == 0) {
    return TT_cube_map;

  } else {
    return TT_unspecified;
  }
}

/**
 * Returns the Format value associated with the given string representation,
 * or F_unspecified if the string does not match any known Format value.
 */
EggTexture::Format EggTexture::
string_format(const string &string) {
  if (cmp_nocase_uh(string, "rgba") == 0) {
    return F_rgba;
  } else if (cmp_nocase_uh(string, "rgbm") == 0) {
    return F_rgbm;
  } else if (cmp_nocase_uh(string, "rgba12") == 0) {
    return F_rgba12;
  } else if (cmp_nocase_uh(string, "rgba8") == 0) {
    return F_rgba8;
  } else if (cmp_nocase_uh(string, "rgba4") == 0) {
    return F_rgba4;

  } else if (cmp_nocase_uh(string, "rgb") == 0) {
    return F_rgb;
  } else if (cmp_nocase_uh(string, "rgb12") == 0) {
    return F_rgb12;
  } else if (cmp_nocase_uh(string, "rgb8") == 0) {
    return F_rgb8;
  } else if (cmp_nocase_uh(string, "rgb5") == 0) {
    return F_rgb5;
  } else if (cmp_nocase_uh(string, "rgba5") == 0) {
    return F_rgba5;
  } else if (cmp_nocase_uh(string, "rgb332") == 0) {
    return F_rgb332;
  } else if (cmp_nocase_uh(string, "red") == 0) {
    return F_red;
  } else if (cmp_nocase_uh(string, "green") == 0) {
    return F_green;
  } else if (cmp_nocase_uh(string, "blue") == 0) {
    return F_blue;
  } else if (cmp_nocase_uh(string, "alpha") == 0) {
    return F_alpha;
  } else if (cmp_nocase_uh(string, "luminance") == 0) {
    return F_luminance;
  } else if (cmp_nocase_uh(string, "luminance_alpha") == 0) {
    return F_luminance_alpha;
  } else if (cmp_nocase_uh(string, "luminance_alphamask") == 0) {
    return F_luminance_alphamask;
  } else {
    return F_unspecified;
  }
}

/**
 * Returns the CompressionMode value associated with the given string
 * representation, or CM_default if the string does not match any known
 * CompressionMode value.
 */
EggTexture::CompressionMode EggTexture::
string_compression_mode(const string &string) {
  if (cmp_nocase_uh(string, "off") == 0) {
    return CM_off;
  } else if (cmp_nocase_uh(string, "on") == 0) {
    return CM_on;
  } else if (cmp_nocase_uh(string, "fxt1") == 0) {
    return CM_fxt1;
  } else if (cmp_nocase_uh(string, "dxt1") == 0) {
    return CM_dxt1;
  } else if (cmp_nocase_uh(string, "dxt2") == 0) {
    return CM_dxt2;
  } else if (cmp_nocase_uh(string, "dxt3") == 0) {
    return CM_dxt3;
  } else if (cmp_nocase_uh(string, "dxt4") == 0) {
    return CM_dxt4;
  } else if (cmp_nocase_uh(string, "dxt5") == 0) {
    return CM_dxt5;
  } else {
    return CM_default;
  }
}

/**
 * Returns the WrapMode value associated with the given string representation,
 * or WM_unspecified if the string does not match any known WrapMode value.
 */
EggTexture::WrapMode EggTexture::
string_wrap_mode(const string &string) {
  if (cmp_nocase_uh(string, "repeat") == 0) {
    return WM_repeat;
  } else if (cmp_nocase_uh(string, "clamp") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror") == 0) {
    return WM_mirror;
  } else if (cmp_nocase_uh(string, "mirror_once") == 0) {
    return WM_mirror_once;
  } else if (cmp_nocase_uh(string, "border_color") == 0) {
    return WM_border_color;
  } else {
    return WM_unspecified;
  }
}

/**
 * Returns the FilterType value associated with the given string
 * representation, or FT_unspecified if the string does not match any known
 * FilterType value.
 */
EggTexture::FilterType EggTexture::
string_filter_type(const string &string) {
  // Old egg filter types.
  if (cmp_nocase_uh(string, "point") == 0) {
    return FT_nearest;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "bilinear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "trilinear") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap_point") == 0) {
    return FT_nearest_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "mipmap_linear") == 0) {
    return FT_nearest_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap_bilinear") == 0) {
    return FT_linear_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "mipmap_trilinear") == 0) {
    return FT_linear_mipmap_linear;

    // Current egg filter types, that match those in Texture.
  } else if (cmp_nocase_uh(string, "nearest") == 0) {
    return FT_nearest;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_nearest") == 0) {
    return FT_nearest_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "linear_mipmap_nearest") == 0) {
    return FT_linear_mipmap_nearest;
  } else if (cmp_nocase_uh(string, "nearest_mipmap_linear") == 0) {
    return FT_nearest_mipmap_linear;
  } else if (cmp_nocase_uh(string, "linear_mipmap_linear") == 0) {
    return FT_linear_mipmap_linear;

  } else {
    return FT_unspecified;
  }
}

/**
 * Returns the EnvType value associated with the given string representation,
 * or ET_unspecified if the string does not match any known EnvType value.
 */
EggTexture::EnvType EggTexture::
string_env_type(const string &string) {
  if (cmp_nocase_uh(string, "modulate") == 0) {
    return ET_modulate;

  } else if (cmp_nocase_uh(string, "decal") == 0) {
    return ET_decal;

  } else if (cmp_nocase_uh(string, "blend") == 0) {
    return ET_blend;

  } else if (cmp_nocase_uh(string, "replace") == 0) {
    return ET_replace;

  } else if (cmp_nocase_uh(string, "add") == 0) {
    return ET_add;

  } else if (cmp_nocase_uh(string, "blend_color_scale") == 0) {
    return ET_blend_color_scale;

  } else if (cmp_nocase_uh(string, "modulate_glow") == 0) {
    return ET_modulate_glow;

  } else if (cmp_nocase_uh(string, "modulate_gloss") == 0) {
    return ET_modulate_gloss;

  } else if (cmp_nocase_uh(string, "normal") == 0) {
    return ET_normal;

  } else if (cmp_nocase_uh(string, "normal_height") == 0) {
    return ET_normal_height;

  } else if (cmp_nocase_uh(string, "glow") == 0) {
    return ET_glow;

  } else if (cmp_nocase_uh(string, "gloss") == 0) {
    return ET_gloss;

  } else if (cmp_nocase_uh(string, "height") == 0) {
    return ET_height;

  } else if (cmp_nocase_uh(string, "selector") == 0) {
    return ET_selector;

  } else if (cmp_nocase_uh(string, "normal_gloss") == 0) {
    return ET_normal_gloss;

  } else {
    return ET_unspecified;
  }
}

/**
 * Returns the CombineMode value associated with the given string
 * representation, or CM_unspecified if the string does not match any known
 * CombineMode value.
 */
EggTexture::CombineMode EggTexture::
string_combine_mode(const string &string) {
  if (cmp_nocase_uh(string, "replace") == 0) {
    return CM_replace;

  } else if (cmp_nocase_uh(string, "modulate") == 0) {
    return CM_modulate;

  } else if (cmp_nocase_uh(string, "add") == 0) {
    return CM_add;

  } else if (cmp_nocase_uh(string, "add_signed") == 0) {
    return CM_add_signed;

  } else if (cmp_nocase_uh(string, "interpolate") == 0) {
    return CM_interpolate;

  } else if (cmp_nocase_uh(string, "subtract") == 0) {
    return CM_subtract;

  } else if (cmp_nocase_uh(string, "dot3_rgb") == 0) {
    return CM_dot3_rgb;

  } else if (cmp_nocase_uh(string, "dot3_rgba") == 0) {
    return CM_dot3_rgba;

  } else {
    return CM_unspecified;
  }
}

/**
 * Returns the CombineSource value associated with the given string
 * representation, or CS_unspecified if the string does not match any known
 * CombineSource value.
 */
EggTexture::CombineSource EggTexture::
string_combine_source(const string &string) {
  if (cmp_nocase_uh(string, "texture") == 0) {
    return CS_texture;

  } else if (cmp_nocase_uh(string, "constant") == 0) {
    return CS_constant;

  } else if (cmp_nocase_uh(string, "primary_color") == 0) {
    return CS_primary_color;

  } else if (cmp_nocase_uh(string, "previous") == 0) {
    return CS_previous;

  } else if (cmp_nocase_uh(string, "constant_color_scale") == 0) {
    return CS_constant_color_scale;

  } else if (cmp_nocase_uh(string, "last_saved_result") == 0) {
    return CS_last_saved_result;

  } else {
    return CS_unspecified;
  }
}

/**
 * Returns the CombineOperand value associated with the given string
 * representation, or CO_unspecified if the string does not match any known
 * CombineOperand value.
 */
EggTexture::CombineOperand EggTexture::
string_combine_operand(const string &string) {
  if (cmp_nocase_uh(string, "src_color") == 0) {
    return CO_src_color;

  } else if (cmp_nocase_uh(string, "one_minus_src_color") == 0) {
    return CO_one_minus_src_color;

  } else if (cmp_nocase_uh(string, "src_alpha") == 0) {
    return CO_src_alpha;

  } else if (cmp_nocase_uh(string, "one_minus_src_alpha") == 0) {
    return CO_one_minus_src_alpha;

  } else {
    return CO_unspecified;
  }
}

/**
 * Returns the TexGen value associated with the given string representation,
 * or ET_unspecified if the string does not match any known TexGen value.
 */
EggTexture::TexGen EggTexture::
string_tex_gen(const string &string) {
  if (cmp_nocase_uh(string, "unspecified") == 0) {
    return TG_unspecified;

  } else if (cmp_nocase_uh(string, "sphere_map") == 0 ||
             cmp_nocase_uh(string, "eye_sphere_map") == 0) {
    return TG_eye_sphere_map;

  } else if (cmp_nocase_uh(string, "world_cube_map") == 0) {
    return TG_world_cube_map;

  } else if (cmp_nocase_uh(string, "cube_map") == 0 ||
             cmp_nocase_uh(string, "eye_cube_map") == 0) {
    return TG_eye_cube_map;

  } else if (cmp_nocase_uh(string, "world_normal") == 0) {
    return TG_world_normal;

  } else if (cmp_nocase_uh(string, "eye_normal") == 0) {
    return TG_eye_normal;

  } else if (cmp_nocase_uh(string, "world_position") == 0) {
    return TG_world_position;

  } else if (cmp_nocase_uh(string, "eye_position") == 0) {
    return TG_eye_position;

  } else if (cmp_nocase_uh(string, "point_sprite") == 0) {
    return TG_point_sprite;

  } else {
    return TG_unspecified;
  }
}

/**
 * Returns the TexGen value associated with the given string representation,
 * or ET_unspecified if the string does not match any known TexGen value.
 */
EggTexture::QualityLevel EggTexture::
string_quality_level(const string &string) {
  if (cmp_nocase_uh(string, "unspecified") == 0) {
    return QL_unspecified;

  } else if (cmp_nocase_uh(string, "default") == 0) {
    return QL_default;

  } else if (cmp_nocase_uh(string, "fastest") == 0) {
    return QL_fastest;

  } else if (cmp_nocase_uh(string, "normal") == 0) {
    return QL_normal;

  } else if (cmp_nocase_uh(string, "best") == 0) {
    return QL_best;

  } else {
    return QL_unspecified;
  }
}

/**
 * Returns this object cross-cast to an EggTransform pointer, if it inherits
 * from EggTransform, or NULL if it does not.
 */
EggTransform *EggTexture::
as_transform() {
  return this;
}

/**
 * This function is called within parse_egg().  It should call the appropriate
 * function on the lexer to initialize the parser into the state associated
 * with this object.  If the object cannot be parsed into directly, it should
 * return false.
 */
bool EggTexture::
egg_start_parse_body() {
  egg_start_texture_body();
  return true;
}

/**
 * Ensures that our multitexture_sort is at least the indicated value.
 */
bool EggTexture::
r_min_multitexture_sort(int sort, EggTexture::MultiTextures &cycle_detector) {
  if (_multitexture_sort >= sort) {
    // No problem.
    return true;
  }

  if (!cycle_detector.insert(this).second) {
    // Oops, we just hit a cycle!
    return false;
  }

  _multitexture_sort = sort;

  // Now we also have to increment all of the textures that we are under.
  bool no_cycles = true;

  MultiTextures::iterator mti;
  for (mti = _under_textures.begin();
       mti != _under_textures.end();
       ++mti) {
    EggTexture *other = (*mti);
    if (!other->r_min_multitexture_sort(sort + 1, cycle_detector)) {
      // Oops, found a cycle!
      no_cycles = false;
    }
  }

  return no_cycles;
}


/**
 *
 */
ostream &operator << (ostream &out, EggTexture::TextureType texture_type) {
  switch (texture_type) {
  case EggTexture::TT_unspecified:
    return out << "unspecified";

  case EggTexture::TT_1d_texture:
    return out << "1d";

  case EggTexture::TT_2d_texture:
    return out << "2d";

  case EggTexture::TT_3d_texture:
    return out << "3d";

  case EggTexture::TT_cube_map:
    return out << "cube-map";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}


/**
 *
 */
ostream &operator << (ostream &out, EggTexture::Format format) {
  switch (format) {
  case EggTexture::F_unspecified:
    return out << "unspecified";

  case EggTexture::F_rgba:
    return out << "rgba";
  case EggTexture::F_rgbm:
    return out << "rgbm";
  case EggTexture::F_rgba12:
    return out << "rgba12";
  case EggTexture::F_rgba8:
    return out << "rgba8";
  case EggTexture::F_rgba4:
    return out << "rgba4";

  case EggTexture::F_rgb:
    return out << "rgb";
  case EggTexture::F_rgb12:
    return out << "rgb12";
  case EggTexture::F_rgb8:
    return out << "rgb8";
  case EggTexture::F_rgb5:
    return out << "rgb5";
  case EggTexture::F_rgba5:
    return out << "rgba5";
  case EggTexture::F_rgb332:
    return out << "rgb332";

  case EggTexture::F_red:
    return out << "red";
  case EggTexture::F_green:
    return out << "green";
  case EggTexture::F_blue:
    return out << "blue";
  case EggTexture::F_alpha:
    return out << "alpha";
  case EggTexture::F_luminance:
    return out << "luminance";
  case EggTexture::F_luminance_alpha:
    return out << "luminance_alpha";
  case EggTexture::F_luminance_alphamask:
    return out << "luminance_alphamask";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggTexture::CompressionMode mode) {
  switch (mode) {
  case EggTexture::CM_default:
    return out << "default";
  case EggTexture::CM_off:
    return out << "off";
  case EggTexture::CM_on:
    return out << "on";
  case EggTexture::CM_fxt1:
    return out << "fxt1";
  case EggTexture::CM_dxt1:
    return out << "dxt1";
  case EggTexture::CM_dxt2:
    return out << "dxt2";
  case EggTexture::CM_dxt3:
    return out << "dxt3";
  case EggTexture::CM_dxt4:
    return out << "dxt4";
  case EggTexture::CM_dxt5:
    return out << "dxt5";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggTexture::WrapMode mode) {
  switch (mode) {
  case EggTexture::WM_unspecified:
    return out << "unspecified";
  case EggTexture::WM_repeat:
    return out << "repeat";
  case EggTexture::WM_clamp:
    return out << "clamp";
  case EggTexture::WM_mirror:
    return out << "mirror";
  case EggTexture::WM_mirror_once:
    return out << "mirror_once";
  case EggTexture::WM_border_color:
    return out << "border_color";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggTexture::FilterType type) {
  switch (type) {
  case EggTexture::FT_unspecified:
    return out << "unspecified";

  case EggTexture::FT_nearest:
    return out << "nearest";
  case EggTexture::FT_linear:
    return out << "linear";

  case EggTexture::FT_nearest_mipmap_nearest:
    return out << "nearest_mipmap_nearest";
  case EggTexture::FT_linear_mipmap_nearest:
    return out << "linear_mipmap_nearest";
  case EggTexture::FT_nearest_mipmap_linear:
    return out << "nearest_mipmap_linear";
  case EggTexture::FT_linear_mipmap_linear:
    return out << "linear_mipmap_linear";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggTexture::EnvType type) {
  switch (type) {
  case EggTexture::ET_unspecified:
    return out << "unspecified";

  case EggTexture::ET_modulate:
    return out << "modulate";

  case EggTexture::ET_decal:
    return out << "decal";

  case EggTexture::ET_blend:
    return out << "blend";

  case EggTexture::ET_replace:
    return out << "replace";

  case EggTexture::ET_add:
    return out << "add";

  case EggTexture::ET_blend_color_scale:
    return out << "blend_color_scale";

  case EggTexture::ET_modulate_glow:
    return out << "modulate_glow";

  case EggTexture::ET_modulate_gloss:
    return out << "modulate_gloss";

  case EggTexture::ET_normal:
    return out << "normal";

  case EggTexture::ET_normal_height:
    return out << "normal_height";

  case EggTexture::ET_glow:
    return out << "glow";

  case EggTexture::ET_gloss:
    return out << "gloss";

  case EggTexture::ET_height:
    return out << "height";

  case EggTexture::ET_selector:
    return out << "selector";

  case EggTexture::ET_normal_gloss:
    return out << "normal_gloss";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

ostream &
operator << (ostream &out, EggTexture::CombineMode cm) {
  switch (cm) {
  case EggTexture::CM_unspecified:
    return out << "unspecified";

  case EggTexture::CM_replace:
    return out << "replace";

  case EggTexture::CM_modulate:
    return out << "modulate";

  case EggTexture::CM_add:
    return out << "add";

  case EggTexture::CM_add_signed:
    return out << "add_signed";

  case EggTexture::CM_interpolate:
    return out << "interpolate";

  case EggTexture::CM_subtract:
    return out << "subtract";

  case EggTexture::CM_dot3_rgb:
    return out << "dot3_rgb";

  case EggTexture::CM_dot3_rgba:
    return out << "dot3_rgba";
  }

  return out << "**invalid CombineMode(" << (int)cm << ")**";
}

ostream &
operator << (ostream &out, EggTexture::CombineChannel cm) {
  switch (cm) {
  case EggTexture::CC_rgb:
    return out << "rgb";

  case EggTexture::CC_alpha:
    return out << "alpha";

  case EggTexture::CC_num_channels:
    // This case is here just to prevent a compiler warning.  Fall out of the
    // switch and return the error message.
    break;
  }

  return out << "**invalid CombineChannel(" << (int)cm << ")**";
}

ostream &
operator << (ostream &out, EggTexture::CombineSource cs) {
  switch (cs) {
  case EggTexture::CS_unspecified:
    return out << "unspecified";

  case EggTexture::CS_texture:
    return out << "texture";

  case EggTexture::CS_constant:
    return out << "constant";

  case EggTexture::CS_primary_color:
    return out << "primary_color";

  case EggTexture::CS_previous:
    return out << "previous";

  case EggTexture::CS_constant_color_scale:
    return out << "constant_color_scale";

  case EggTexture::CS_last_saved_result:
    return out << "last_saved_result";
  }

  return out << "**invalid CombineSource(" << (int)cs << ")**";
}

ostream &
operator << (ostream &out, EggTexture::CombineOperand co) {
  switch (co) {
  case EggTexture::CO_unspecified:
    return out << "unspecified";

  case EggTexture::CO_src_color:
    return out << "src_color";

  case EggTexture::CO_one_minus_src_color:
    return out << "one_minus_src_color";

  case EggTexture::CO_src_alpha:
    return out << "src_alpha";

  case EggTexture::CO_one_minus_src_alpha:
    return out << "one_minus_src_alpha";
  }

  return out << "**invalid CombineOperand(" << (int)co << ")**";
}

ostream &
operator << (ostream &out, EggTexture::TexGen tex_gen) {
  switch (tex_gen) {
  case EggTexture::TG_unspecified:
    return out << "unspecified";

  case EggTexture::TG_eye_sphere_map:
    return out << "eye_sphere_map";

  case EggTexture::TG_world_cube_map:
    return out << "world_cube_map";

  case EggTexture::TG_eye_cube_map:
    return out << "eye_cube_map";

  case EggTexture::TG_world_normal:
    return out << "world_normal";

  case EggTexture::TG_eye_normal:
    return out << "eye_normal";

  case EggTexture::TG_world_position:
    return out << "world_position";

  case EggTexture::TG_eye_position:
    return out << "eye_position";

  case EggTexture::TG_point_sprite:
    return out << "point_sprite";
  }

  return out << "**invalid TexGen(" << (int)tex_gen << ")**";
}

ostream &
operator << (ostream &out, EggTexture::QualityLevel quality_level) {
  switch (quality_level) {
  case EggTexture::QL_unspecified:
    return out << "unspecified";
  case EggTexture::QL_default:
    return out << "default";
  case EggTexture::QL_fastest:
    return out << "fastest";
  case EggTexture::QL_normal:
    return out << "normal";
  case EggTexture::QL_best:
    return out << "best";
  }

  return out << "**invalid QualityLevel(" << (int)quality_level << ")**";
}
