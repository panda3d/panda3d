// Filename: eggTexture.cxx
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eggTexture.h"
#include "eggMiscFuncs.h"
#include "lexerDefs.h"

#include <indent.h>
#include <string_utils.h>

TypeHandle EggTexture::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggTexture::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTexture::
EggTexture(const string &tref_name, const string &filename)
  : EggFilenameNode(tref_name, filename)
{
  _format = F_unspecified;
  _wrap_mode = WM_unspecified;
  _wrap_u = WM_unspecified;
  _wrap_v = WM_unspecified;
  _minfilter = FT_unspecified;
  _magfilter = FT_unspecified;
  _anisotropic_degree = 0;
  _env_type = ET_unspecified;
  _flags = 0;
  _transform = LMatrix3d::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTexture::
EggTexture(const EggTexture &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTexture &EggTexture::
operator = (const EggTexture &copy) {
  EggFilenameNode::operator = (copy);
  EggRenderMode::operator = (copy);

  _format = copy._format;
  _wrap_mode = copy._wrap_mode;
  _wrap_u = copy._wrap_u;
  _wrap_v = copy._wrap_v;
  _minfilter = copy._minfilter;
  _magfilter = copy._magfilter;
  _anisotropic_degree = copy._anisotropic_degree;
  _env_type = copy._env_type;
  _flags = copy._flags;
  _transform = copy._transform;
  _alpha_filename = copy._alpha_filename;
  _alpha_fullpath = copy._alpha_fullpath;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::write
//       Access: Public, Virtual
//  Description: Writes the texture definition to the indicated output
//               stream in Egg format.
////////////////////////////////////////////////////////////////////
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

  if (get_format() != F_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> format { " << get_format() << " }\n";
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

  EggRenderMode::write(out, indent_level + 2);

  if (has_transform()) {
    write_transform(out, _transform, indent_level + 2);
  }

  indent(out, indent_level) << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::is_equivalent_to
//       Access: Public
//  Description: Returns true if the two textures are equivalent in
//               all relevant properties (according to eq), false
//               otherwise.
//
//               The Equivalence parameter, eq, should be set to the
//               bitwise OR of the following properties, according to
//               what you consider relevant:
//
//               EggTexture::E_basename:
//                 The basename part of the texture filename, without
//                 the directory prefix *or* the filename extension.
//
//               EggTexture::E_extension:
//                 The extension part of the texture filename.
//
//               EggTexture::E_dirname:
//                 The directory prefix of the texture filename.
//
//               EggTexture::E_complete_filename:
//                 The union of the above three; that is, the complete
//                 filename, with directory, basename, and extension.
//
//               EggTexture::E_transform:
//                 The texture matrix.
//
//               EggTexture::E_attributes:
//                 All remaining texture attributes (mode, mipmap,
//                 etc.) except TRef name.
//
//               EggTexture::E_tref_name:
//                 The TRef name.
////////////////////////////////////////////////////////////////////
bool EggTexture::
is_equivalent_to(const EggTexture &other, int eq) const {
  if ((eq & E_complete_filename) == E_complete_filename) {
    if (get_filename() != other.get_filename()) {
      return false;
    }
  } else {
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
    if (transform_is_identity() != other.transform_is_identity()) {
      return false;
    }

    if (has_transform() && other.has_transform()) {
      if (!_transform.almost_equal(other._transform, 0.0001)) {
        return false;
      }
    }
  }

  if (eq & E_attributes) {
    if (_format != other._format ||
        _wrap_mode != other._wrap_mode ||
        _wrap_u != other._wrap_u ||
        _wrap_v != other._wrap_v ||
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
    if (get_name() != other.get_name()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::sorts_less_than
//       Access: Public
//  Description: An ordering operator to compare two textures for
//               sorting order.  This imposes an arbitrary ordering
//               useful to identify unique textures, according to the
//               indicated Equivalence factor.  See
//               is_equivalent_to().
////////////////////////////////////////////////////////////////////
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
      int compare = _transform.compare_to(other._transform);
      if (compare != 0) {
    return compare < 0;
      }
    }
  }

  if (eq & E_attributes) {
    if (_format != other._format) {
      return (int)_format < (int)other._format;
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

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::has_alpha_channel
//       Access: Public
//  Description: Given the number of color components (channels) in
//               the image file as actually read from the disk, return
//               true if this texture seems to have an alpha channel
//               or not.  This depends on the EggTexture's format as
//               well as the number of channels.
////////////////////////////////////////////////////////////////////
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
    // These formats never use alpha, regardless of the number of
    // components we have.
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

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::string_format
//       Access: Public
//  Description: Returns the Format value associated with the given
//               string representation, or F_unspecified if the string
//               does not match any known Format value.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::string_wrap_mode
//       Access: Public
//  Description: Returns the WrapMode value associated with the given
//               string representation, or WM_unspecified if the string
//               does not match any known WrapMode value.
////////////////////////////////////////////////////////////////////
EggTexture::WrapMode EggTexture::
string_wrap_mode(const string &string) {
  if (cmp_nocase_uh(string, "repeat") == 0) {
    return WM_repeat;
  } else if (cmp_nocase_uh(string, "clamp") == 0) {
    return WM_clamp;
  } else {
    return WM_unspecified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::string_filter_type
//       Access: Public
//  Description: Returns the FilterType value associated with the given
//               string representation, or FT_unspecified if the string
//               does not match any known FilterType value.
////////////////////////////////////////////////////////////////////
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
    return FT_linear;
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

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::string_env_type
//       Access: Public
//  Description: Returns the EnvType value associated with the given
//               string representation, or ET_unspecified if the string
//               does not match any known EnvType value.
////////////////////////////////////////////////////////////////////
EggTexture::EnvType EggTexture::
string_env_type(const string &string) {
  if (cmp_nocase_uh(string, "modulate") == 0) {
    return ET_modulate;
  } else if (cmp_nocase_uh(string, "decal") == 0) {
    return ET_decal;
  } else {
    return ET_unspecified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggTexture::egg_start_parse_body
//       Access: Protected, Virtual
//  Description: This function is called within parse_egg().  It
//               should call the appropriate function on the lexer to
//               initialize the parser into the state associated with
//               this object.  If the object cannot be parsed into
//               directly, it should return false.
////////////////////////////////////////////////////////////////////
bool EggTexture::
egg_start_parse_body() {
  egg_start_texture_body();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Format output operator
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: WrapMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggTexture::WrapMode mode) {
  switch (mode) {
  case EggTexture::WM_unspecified:
    return out << "unspecified";
  case EggTexture::WM_repeat:
    return out << "repeat";
  case EggTexture::WM_clamp:
    return out << "clamp";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

////////////////////////////////////////////////////////////////////
//     Function: FilterType output operator
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EnvType output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggTexture::EnvType type) {
  switch (type) {
  case EggTexture::ET_unspecified:
    return out << "unspecified";
  case EggTexture::ET_modulate:
    return out << "modulate";
  case EggTexture::ET_decal:
    return out << "decal";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}
