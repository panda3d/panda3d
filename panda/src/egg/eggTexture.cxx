// Filename: eggTexture.cxx
// Created by:  drose (18Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggTexture.h"
#include "eggMiscFuncs.h"

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
  _magfilteralpha = FT_unspecified;
  _magfiltercolor = FT_unspecified;
  _env_type = ET_unspecified;
  _has_transform = false;
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
  EggAlphaMode::operator = (copy);
  
  _format = copy._format;
  _wrap_mode = copy._wrap_mode;
  _wrap_u = copy._wrap_u;
  _wrap_v = copy._wrap_v;
  _minfilter = copy._minfilter;
  _magfilter = copy._magfilter;
  _magfilteralpha = copy._magfilteralpha;
  _magfiltercolor = copy._magfiltercolor;
  _env_type = copy._env_type;
  _has_transform = copy._has_transform;
  _transform = copy._transform;

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
  enquote_string(out, get_fullpath(), indent_level + 2) << "\n";

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

  if (get_magfilteralpha() != FT_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> magfilteralpha { " << get_magfilteralpha() << " }\n";
  }

  if (get_magfiltercolor() != FT_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> magfiltercolor { " << get_magfiltercolor() << " }\n";
  }

  if (get_env_type() != ET_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> envtype { " << get_env_type() << " }\n";
  }

  EggAlphaMode::write(out, indent_level + 2);

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
    if (get_fullpath() != other.get_fullpath()) {
      return false;
    }
  } else {
    if (eq & E_basename) {
      if (get_basename_wo_extension() != other.get_basename_wo_extension()) {
	return false;
      }
    }
    if (eq & E_extension) {
      if (get_extension() != other.get_extension()) {
	return false;
      }
    }
    if (eq & E_dirname) {
      if (get_dirname() != other.get_dirname()) {
	return false;
      }
    }
  }

  if (eq & E_transform) {
    if (transform_is_identity() != other.transform_is_identity()) {
      return false;
    }
    
    if (_has_transform && other._has_transform) {
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
	_magfilteralpha != other._magfilteralpha ||
	_magfiltercolor != other._magfiltercolor ||
	_env_type != other._env_type) {
      return false;
    }
    if (EggAlphaMode::operator != (other)) {
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
    if (get_fullpath() != other.get_fullpath()) {
      return get_fullpath() < other.get_fullpath();
    }
  } else {
    if (eq & E_basename) {
      if (get_basename_wo_extension() != other.get_basename_wo_extension()) {
	return get_basename_wo_extension() < other.get_basename_wo_extension();
      }
    }
    if (eq & E_extension) {
      if (get_extension() != other.get_extension()) {
	return get_extension() < other.get_extension();
      }
    }
    if (eq & E_dirname) {
      if (get_dirname() != other.get_dirname()) {
	return get_dirname() < other.get_dirname();
      }
    }
  }

  if (eq & E_transform) {
    bool is_identity = transform_is_identity();
    bool other_is_identity = other.transform_is_identity();
    if (is_identity != other_is_identity) {
      return (int)is_identity < (int)other_is_identity;
    }

    if (_has_transform && other._has_transform) {
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
    if (_magfilteralpha != other._magfilteralpha) {
      return (int)_magfilteralpha < (int)other._magfilteralpha;
    }
    if (_magfiltercolor != other._magfiltercolor) {
      return (int)_magfiltercolor < (int)other._magfiltercolor;
    }
    if (_env_type != other._env_type) {
      return (int)_env_type < (int)other._env_type;
    }

    if (EggAlphaMode::operator != (other)) {
      return EggAlphaMode::operator < (other);
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

  } else if (cmp_nocase_uh(string, "luminance_alpha") == 0) {
    return F_luminance_alpha;

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
  if (cmp_nocase_uh(string, "point") == 0) {
    return FT_point;
  } else if (cmp_nocase_uh(string, "linear") == 0) {
    return FT_linear;
  } else if (cmp_nocase_uh(string, "bilinear") == 0) {
    return FT_bilinear;
  } else if (cmp_nocase_uh(string, "trilinear") == 0) {
    return FT_trilinear;
  } else if (cmp_nocase_uh(string, "mipmap_point") == 0) {
    return FT_mipmap_point;
  } else if (cmp_nocase_uh(string, "mipmap_linear") == 0) {
    return FT_mipmap_linear;
  } else if (cmp_nocase_uh(string, "mipmap_bilinear") == 0) {
    return FT_mipmap_bilinear;
  } else if (cmp_nocase_uh(string, "mipmap_trilinear") == 0) {
    return FT_mipmap_trilinear;
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
//     Function: Format output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &operator << (ostream &out, EggTexture::Format format) {
  switch (format) {
  case EggTexture::F_unspecified:
    return out << "unspecified";

  case EggTexture::F_rgba:
    return out << "rgba";
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

  case EggTexture::F_luminance_alpha:
    return out << "luminance-alpha";

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
  case EggTexture::FT_point:
    return out << "point";
  case EggTexture::FT_linear:
    return out << "linear";
  case EggTexture::FT_bilinear:
    return out << "bilinear";
  case EggTexture::FT_trilinear:
    return out << "trilinear";
  case EggTexture::FT_mipmap_point:
    return out << "mipmap_point";
  case EggTexture::FT_mipmap_linear:
    return out << "mipmap_linear";
  case EggTexture::FT_mipmap_bilinear:
    return out << "mipmap_bilinear";
  case EggTexture::FT_mipmap_trilinear:
    return out << "mipmap_trilinear";
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
