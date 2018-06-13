/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureProperties.cxx
 * @author drose
 * @date 2000-11-29
 */

#include "textureProperties.h"
#include "palettizer.h"
#include "pnmFileType.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"

using std::string;

TypeHandle TextureProperties::_type_handle;

/**
 *
 */
TextureProperties::
TextureProperties() {
  _got_num_channels = false;
  _num_channels = 0;
  _effective_num_channels = 0;
  _format = EggTexture::F_unspecified;
  _force_format = false;
  _generic_format = false;
  _keep_format = false;
  _minfilter = EggTexture::FT_unspecified;
  _magfilter = EggTexture::FT_unspecified;
  _quality_level = EggTexture::QL_unspecified;
  _anisotropic_degree = 0;
  _color_type = nullptr;
  _alpha_type = nullptr;
}

/**
 *
 */
TextureProperties::
TextureProperties(const TextureProperties &copy) :
  _format(copy._format),
  _force_format(copy._force_format),
  _generic_format(copy._generic_format),
  _keep_format(copy._keep_format),
  _minfilter(copy._minfilter),
  _magfilter(copy._magfilter),
  _quality_level(copy._quality_level),
  _anisotropic_degree(copy._anisotropic_degree),
  _color_type(copy._color_type),
  _alpha_type(copy._alpha_type),
  _got_num_channels(copy._got_num_channels),
  _num_channels(copy._num_channels),
  _effective_num_channels(copy._effective_num_channels)
{
}

/**
 *
 */
void TextureProperties::
operator = (const TextureProperties &copy) {
  _force_format = copy._force_format;
  _generic_format = copy._generic_format;
  _keep_format = copy._keep_format;
  _minfilter = copy._minfilter;
  _magfilter = copy._magfilter;
  _quality_level = copy._quality_level;
  _anisotropic_degree = copy._anisotropic_degree;
  _color_type = copy._color_type;
  _alpha_type = copy._alpha_type;
  _got_num_channels = copy._got_num_channels;
  _num_channels = copy._num_channels;
  _effective_num_channels = copy._effective_num_channels;
  _format = copy._format;
}

/**
 * Resets only the properties that might be changed by update_properties() to
 * a neutral state.
 */
void TextureProperties::
clear_basic() {
  if (!_force_format) {
    _format = EggTexture::F_unspecified;
  }

  _minfilter = EggTexture::FT_unspecified;
  _magfilter = EggTexture::FT_unspecified;
  _quality_level = EggTexture::QL_unspecified;
  _anisotropic_degree = 0;
}

/**
 * Returns true if the number of channels is known.
 */
bool TextureProperties::
has_num_channels() const {
  return _got_num_channels;
}

/**
 * Returns the number of channels (1 through 4) associated with the image.  It
 * is an error to call this unless has_num_channels() returns true.
 */
int TextureProperties::
get_num_channels() const {
  nassertr(_got_num_channels, 0);
  return _effective_num_channels;
}

/**
 * Sets the number of channels (1 through 4) associated with the image,
 * presumably after reading this information from the image header.
 */
void TextureProperties::
set_num_channels(int num_channels) {
  _num_channels = num_channels;
  _effective_num_channels = num_channels;
  _got_num_channels = true;
}

/**
 * Sets the actual number of channels to indicate a grayscale image,
 * presumably after discovering that the image contains no colored pixels.
 */
void TextureProperties::
force_grayscale() {
  nassertv(_got_num_channels && _num_channels >= 3);
  _num_channels -= 2;
  _effective_num_channels = _num_channels;
}

/**
 * Sets the actual number of channels to indicate an image with no alpha
 * channel, presumably after discovering that the alpha channel contains no
 * meaningful pixels.
 */
void TextureProperties::
force_nonalpha() {
  nassertv(_got_num_channels && (_num_channels == 2 || _num_channels == 4));
  _num_channels--;
  _effective_num_channels = _num_channels;
}

/**
 * Returns true if the texture uses an alpha channel, false otherwise.
 */
bool TextureProperties::
uses_alpha() const {
  switch (_format) {
  case EggTexture::F_rgba:
  case EggTexture::F_rgbm:
  case EggTexture::F_rgba12:
  case EggTexture::F_rgba8:
  case EggTexture::F_rgba4:
  case EggTexture::F_rgba5:
  case EggTexture::F_alpha:
  case EggTexture::F_luminance_alpha:
  case EggTexture::F_luminance_alphamask:
    return true;

  default:
    return false;
  }
}

/**
 * Returns a string corresponding to the TextureProperties object.  Each
 * unique set of TextureProperties will generate a unique string.  This is
 * used to generate unique palette image filenames.
 */
string TextureProperties::
get_string() const {
  string result;

  if (_got_num_channels) {
    std::ostringstream num;
    num << _effective_num_channels;
    result += num.str();
  }

  result += get_format_string(_format);
  result += get_filter_string(_minfilter);
  result += get_filter_string(_magfilter);
  result += get_anisotropic_degree_string(_anisotropic_degree);
  result += get_type_string(_color_type, _alpha_type);
  result += get_quality_level_string(_quality_level);
  return result;
}

/**
 * If the indicate TextureProperties structure is more specific than this one,
 * updates this one.
 */
void TextureProperties::
update_properties(const TextureProperties &other) {
  if (!_got_num_channels) {
    _got_num_channels = other._got_num_channels;
    _num_channels = other._num_channels;
    _effective_num_channels = _num_channels;
  }
  if (_force_format) {
    // If we've forced our own format, it doesn't change.
  } else if (other._force_format) {
    _format = other._format;
  } else {
    _format = union_format(_format, other._format);
  }

  _minfilter = union_filter(_minfilter, other._minfilter);
  _magfilter = union_filter(_magfilter, other._magfilter);
  _quality_level = union_quality_level(_quality_level, other._quality_level);

  _anisotropic_degree = other._anisotropic_degree;

  if (_color_type == nullptr) {
    _color_type = other._color_type;
    _alpha_type = other._alpha_type;
  }
}

/**
 * If any properties remain unspecified, specify them now.  Also reconcile
 * conflicting information.
 */
void TextureProperties::
fully_define() {
  if (!_got_num_channels || _force_format) {
    switch (_format) {
    case EggTexture::F_rgba:
    case EggTexture::F_rgbm:
    case EggTexture::F_rgba12:
    case EggTexture::F_rgba8:
    case EggTexture::F_rgba4:
    case EggTexture::F_rgba5:
      _num_channels = 4;
      break;

    case EggTexture::F_unspecified:
    case EggTexture::F_rgb:
    case EggTexture::F_rgb12:
    case EggTexture::F_rgb8:
    case EggTexture::F_rgb5:
    case EggTexture::F_rgb332:
      _num_channels = 3;
      break;

    case EggTexture::F_luminance_alpha:
    case EggTexture::F_luminance_alphamask:
      _num_channels = 2;
      break;

    case EggTexture::F_red:
    case EggTexture::F_green:
    case EggTexture::F_blue:
    case EggTexture::F_alpha:
    case EggTexture::F_luminance:
      _num_channels = 1;
      break;
    }
    _got_num_channels = true;
  }

  _effective_num_channels = _num_channels;

  // Respect the _generic_format flag.  If this is set, it means the user has
  // indicated that we should strip off any bitcount-specific formats and
  // replace them with the more generic equivalents.
  if (_generic_format) {
    switch (_format) {
    case EggTexture::F_unspecified:
    case EggTexture::F_rgba:
    case EggTexture::F_rgbm:
    case EggTexture::F_rgb:
    case EggTexture::F_red:
    case EggTexture::F_green:
    case EggTexture::F_blue:
    case EggTexture::F_alpha:
    case EggTexture::F_luminance:
    case EggTexture::F_luminance_alpha:
    case EggTexture::F_luminance_alphamask:
      break;

    case EggTexture::F_rgba12:
    case EggTexture::F_rgba8:
    case EggTexture::F_rgba4:
    case EggTexture::F_rgba5:
      _format = EggTexture::F_rgba;
      break;

    case EggTexture::F_rgb12:
    case EggTexture::F_rgb8:
    case EggTexture::F_rgb5:
    case EggTexture::F_rgb332:
      _format = EggTexture::F_rgb;
      break;
    }
  }

  // Make sure the format reflects the number of channels, although we accept
  // a format that ignores an alpha channel.
  if (!_force_format && !_keep_format) {
    switch (_num_channels) {
    case 1:
      switch (_format) {
      case EggTexture::F_red:
      case EggTexture::F_green:
      case EggTexture::F_blue:
      case EggTexture::F_alpha:
      case EggTexture::F_luminance:
        break;

        // These formats suggest an alpha channel; they are quietly replaced
        // with non-alpha equivalents.
      case EggTexture::F_luminance_alpha:
      case EggTexture::F_luminance_alphamask:
        _format = EggTexture::F_luminance;
        break;

      default:
        _format = EggTexture::F_luminance;
      }
      break;

    case 2:
      switch (_format) {
      case EggTexture::F_luminance_alpha:
      case EggTexture::F_luminance_alphamask:
        break;

        // These formats implicitly reduce the number of channels to 1.
      case EggTexture::F_red:
      case EggTexture::F_green:
      case EggTexture::F_blue:
      case EggTexture::F_alpha:
      case EggTexture::F_luminance:
        break;

      default:
        _format = EggTexture::F_luminance_alpha;
      }
      break;

    case 3:
      switch (_format) {
      case EggTexture::F_rgb:
      case EggTexture::F_rgb12:
      case EggTexture::F_rgb8:
      case EggTexture::F_rgb5:
      case EggTexture::F_rgb332:
        break;

        // These formats suggest an alpha channel; they are quietly replaced
        // with non-alpha equivalents.
      case EggTexture::F_rgba8:
        _format = EggTexture::F_rgb8;
        break;

      case EggTexture::F_rgba5:
      case EggTexture::F_rgba4:
        _format = EggTexture::F_rgb5;
        break;

        // These formats implicitly reduce the number of channels to 1.
      case EggTexture::F_red:
      case EggTexture::F_green:
      case EggTexture::F_blue:
      case EggTexture::F_alpha:
      case EggTexture::F_luminance:
        break;

      default:
        _format = EggTexture::F_rgb;
      }
      break;

    case 4:
      switch (_format) {
      case EggTexture::F_rgba:
      case EggTexture::F_rgbm:
      case EggTexture::F_rgba12:
      case EggTexture::F_rgba8:
      case EggTexture::F_rgba4:
      case EggTexture::F_rgba5:
        break;

        // These formats implicitly reduce the number of channels to 3.
      case EggTexture::F_rgb:
      case EggTexture::F_rgb12:
      case EggTexture::F_rgb8:
      case EggTexture::F_rgb5:
      case EggTexture::F_rgb332:
        _effective_num_channels = 3;
        break;

        // These formats implicitly reduce the number of channels to 2.
      case EggTexture::F_luminance_alpha:
      case EggTexture::F_luminance_alphamask:
        _effective_num_channels = 2;
        break;

        // These formats implicitly reduce the number of channels to 1.
      case EggTexture::F_red:
      case EggTexture::F_green:
      case EggTexture::F_blue:
      case EggTexture::F_alpha:
      case EggTexture::F_luminance:
        _effective_num_channels = 1;
        break;

      default:
        _format = EggTexture::F_rgba;
      }
    }
  }

  switch (_minfilter) {
  case EggTexture::FT_unspecified:
    _minfilter = EggTexture::FT_linear;
    break;

  default:
    break;
  }

  switch (_magfilter) {
  case EggTexture::FT_unspecified:
  case EggTexture::FT_nearest_mipmap_nearest:
  case EggTexture::FT_linear_mipmap_nearest:
  case EggTexture::FT_nearest_mipmap_linear:
  case EggTexture::FT_linear_mipmap_linear:
    _magfilter = EggTexture::FT_linear;
    break;

  default:
    break;
  }

  if (_color_type == nullptr) {
    _color_type = pal->_color_type;
    _alpha_type = pal->_alpha_type;
  }
}

/**
 * Adjusts the texture properties of the indicated egg reference to match
 * these properties.
 */
void TextureProperties::
update_egg_tex(EggTexture *egg_tex) const {
  egg_tex->set_format(_format);
  egg_tex->set_minfilter(_minfilter);
  egg_tex->set_magfilter(_minfilter);
  egg_tex->set_quality_level(_quality_level);
  egg_tex->set_anisotropic_degree(_anisotropic_degree);
}

/**
 * Returns true if all of the properties that are reflected directly in an egg
 * file match between this TextureProperties object and the other, or false if
 * any of them differ.
 */
bool TextureProperties::
egg_properties_match(const TextureProperties &other) const {
  return (_format == other._format &&
          _minfilter == other._minfilter &&
          _magfilter == other._magfilter &&
          _quality_level == other._quality_level &&
          _anisotropic_degree == other._anisotropic_degree);
}

/**
 *
 */
bool TextureProperties::
operator < (const TextureProperties &other) const {
  if (_format != other._format) {
    return (int)_format < (int)other._format;
  }
  if (_minfilter != other._minfilter) {
    return (int)_minfilter < (int)other._minfilter;
  }
  if (_magfilter != other._magfilter) {
    return (int)_magfilter < (int)other._magfilter;
  }
  if (_quality_level != other._quality_level) {
    return (int)_quality_level < (int)other._quality_level;
  }
  if (_anisotropic_degree != other._anisotropic_degree) {
    return _anisotropic_degree < other._anisotropic_degree;
  }
  if (_color_type != other._color_type) {
    return _color_type < other._color_type;
  }
  if (_color_type != nullptr) {
    if (_alpha_type != other._alpha_type) {
      return _alpha_type < other._alpha_type;
    }
  }
  return false;
}

/**
 *
 */
bool TextureProperties::
operator == (const TextureProperties &other) const {
  return (_format == other._format &&
          _minfilter == other._minfilter &&
          _magfilter == other._magfilter &&
          _quality_level == other._quality_level &&
          _anisotropic_degree == other._anisotropic_degree &&
          _color_type == other._color_type &&
          (_color_type == nullptr ||
           _alpha_type == other._alpha_type));
}

/**
 *
 */
bool TextureProperties::
operator != (const TextureProperties &other) const {
  return !operator == (other);
}

/**
 * Returns a short string representing the given EggTexture format.
 */
string TextureProperties::
get_format_string(EggTexture::Format format) {
  switch (format) {
  case EggTexture::F_unspecified:
    return "u";

  case EggTexture::F_rgba:
    return "a";

  case EggTexture::F_rgbm:
    return "m";

  case EggTexture::F_rgba12:
    return "a12";

  case EggTexture::F_rgba8:
    return "a8";

  case EggTexture::F_rgba4:
    return "a4";

  case EggTexture::F_rgba5:
    return "a5";

  case EggTexture::F_rgb:
    return "c";

  case EggTexture::F_rgb12:
    return "c12";

  case EggTexture::F_rgb8:
    return "c8";

  case EggTexture::F_rgb5:
    return "c5";

  case EggTexture::F_rgb332:
    return "c3";

  case EggTexture::F_luminance_alpha:
    return "t"; // t for two-channel

  case EggTexture::F_luminance_alphamask:
    return "t1";

  case EggTexture::F_red:
    return "r";

  case EggTexture::F_green:
    return "g";

  case EggTexture::F_blue:
    return "b";

  case EggTexture::F_alpha:
    return "a";

  case EggTexture::F_luminance:
    return "l";
  }

  return "x";
}

/**
 * Returns a short string representing the given EggTexture filter type.
 */
string TextureProperties::
get_filter_string(EggTexture::FilterType filter_type) {
  switch (filter_type) {
  case EggTexture::FT_unspecified:
    return "u";

  case EggTexture::FT_nearest:
    return "n";

  case EggTexture::FT_linear:
    return "l";

  case EggTexture::FT_nearest_mipmap_nearest:
    return "m1";

  case EggTexture::FT_linear_mipmap_nearest:
    return "m2";

  case EggTexture::FT_nearest_mipmap_linear:
    return "m3";

  case EggTexture::FT_linear_mipmap_linear:
    return "m";
  }

  return "x";
}

/**
 * Returns a short string describing the anisotropic degree.
 */
string TextureProperties::
get_anisotropic_degree_string(int aniso_degree) {
  if (aniso_degree <= 1) {
    return "";
  } else {
    return string("an") + format_string(aniso_degree);
  }
}

/**
 * Returns a short string describing the quality level.
 */
string TextureProperties::
get_quality_level_string(EggTexture::QualityLevel quality_level) {
  switch (quality_level) {
  case EggTexture::QL_unspecified:
  case EggTexture::QL_default:
    return "";

  case EggTexture::QL_fastest:
    return "f";

  case EggTexture::QL_normal:
    return "n";

  case EggTexture::QL_best:
    return "b";
  }
  return "";
}

/**
 * Returns a short string representing whether the color and/or alpha type has
 * been specified or not.
 */
string TextureProperties::
get_type_string(PNMFileType *color_type, PNMFileType *alpha_type) {
  if (color_type == nullptr) {
    return "";
  }
  if (alpha_type == nullptr) {
    return "c";
  }
  return "a";
}

/**
 * Returns the EggTexture format which is the more specific of the two.
 */
EggTexture::Format TextureProperties::
union_format(EggTexture::Format a, EggTexture::Format b) {
  switch (a) {
  case EggTexture::F_unspecified:
    return b;

  case EggTexture::F_rgba:
    switch (b) {
    case EggTexture::F_rgbm:
    case EggTexture::F_rgba12:
    case EggTexture::F_rgba8:
    case EggTexture::F_rgba4:
    case EggTexture::F_rgba5:
    case EggTexture::F_red:
    case EggTexture::F_green:
    case EggTexture::F_blue:
    case EggTexture::F_alpha:
      return b;

    default:
      return a;
    };

  case EggTexture::F_rgb:
    if (b != EggTexture::F_unspecified) {
      return b;
    }
    return a;

  default:
    return a;
  }
}

/**
 * Returns the EggTexture filter type which is the more specific of the two.
 */
EggTexture::FilterType TextureProperties::
union_filter(EggTexture::FilterType a, EggTexture::FilterType b) {
  if ((int)a < (int)b) {
    return b;
  } else {
    return a;
  }
}

/**
 * Returns the EggTexture quality level which is the more specific of the two.
 */
EggTexture::QualityLevel TextureProperties::
union_quality_level(EggTexture::QualityLevel a, EggTexture::QualityLevel b) {
  if ((int)a < (int)b) {
    return b;
  } else {
    return a;
  }
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void TextureProperties::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_TextureProperties);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void TextureProperties::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  datagram.add_bool(_got_num_channels);
  datagram.add_int32(_num_channels);
  datagram.add_int32(_effective_num_channels);
  datagram.add_int32((int)_format);
  datagram.add_bool(_force_format);
  datagram.add_bool(_generic_format);
  datagram.add_bool(_keep_format);
  datagram.add_int32((int)_minfilter);
  datagram.add_int32((int)_magfilter);
  datagram.add_int32((int)_quality_level);
  datagram.add_int32(_anisotropic_degree);
  writer->write_pointer(datagram, _color_type);
  writer->write_pointer(datagram, _alpha_type);
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int TextureProperties::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = TypedWritable::complete_pointers(p_list, manager);

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_color_type, p_list[index], index);
  }
  index++;

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_alpha_type, p_list[index], index);
  }
  index++;

  return index;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable* TextureProperties::
make_TextureProperties(const FactoryParams &params) {
  TextureProperties *me = new TextureProperties;
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
void TextureProperties::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  _got_num_channels = scan.get_bool();
  _num_channels = scan.get_int32();
  _effective_num_channels = _num_channels;
  if (Palettizer::_read_pi_version >= 9) {
    _effective_num_channels = scan.get_int32();
  }
  _format = (EggTexture::Format)scan.get_int32();
  _force_format = scan.get_bool();
  _generic_format = false;
  if (Palettizer::_read_pi_version >= 9) {
    _generic_format = scan.get_bool();
  }
  _keep_format = false;
  if (Palettizer::_read_pi_version >= 13) {
    _keep_format = scan.get_bool();
  }
  _minfilter = (EggTexture::FilterType)scan.get_int32();
  _magfilter = (EggTexture::FilterType)scan.get_int32();
  if (Palettizer::_read_pi_version >= 18) {
    _quality_level = (EggTexture::QualityLevel)scan.get_int32();
  }
  _anisotropic_degree = scan.get_int32();

  manager->read_pointer(scan);  // _color_type
  manager->read_pointer(scan);  // _alpha_type
}
