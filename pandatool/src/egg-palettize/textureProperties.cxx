// Filename: textureProperties.cxx
// Created by:  drose (29Nov00)
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

#include "textureProperties.h"
#include "palettizer.h"

#include <pnmFileType.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle TextureProperties::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TextureProperties::
TextureProperties() {
  _got_num_channels = false;
  _num_channels = 0;
  _format = EggTexture::F_unspecified;
  _minfilter = EggTexture::FT_unspecified;
  _magfilter = EggTexture::FT_unspecified;
  _color_type = (PNMFileType *)NULL;
  _alpha_type = (PNMFileType *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TextureProperties::
TextureProperties(const TextureProperties &copy) :
  _got_num_channels(copy._got_num_channels),
  _num_channels(copy._num_channels),
  _format(copy._format),
  _minfilter(copy._minfilter),
  _magfilter(copy._magfilter),
  _color_type(copy._color_type),
  _alpha_type(copy._alpha_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void TextureProperties::
operator = (const TextureProperties &copy) {
  _got_num_channels = copy._got_num_channels;
  _num_channels = copy._num_channels;
  _format = copy._format;
  _minfilter = copy._minfilter;
  _magfilter = copy._magfilter;
  _color_type = copy._color_type;
  _alpha_type = copy._alpha_type;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::has_num_channels
//       Access: Public
//  Description: Returns true if the number of channels is known.
////////////////////////////////////////////////////////////////////
bool TextureProperties::
has_num_channels() const {
  return _got_num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::get_num_channels
//       Access: Public
//  Description: Returns the number of channels (1 through 4)
//               associated with the image.  It is an error to call
//               this unless has_num_channels() returns true.
////////////////////////////////////////////////////////////////////
int TextureProperties::
get_num_channels() const {
  nassertr(_got_num_channels, 0);
  return _num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::uses_alpha
//       Access: Public
//  Description: Returns true if the texture uses an alpha channel,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool TextureProperties::
uses_alpha() const {
  return (_num_channels == 2 || _num_channels == 4 ||
          _format == EggTexture::F_alpha);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::get_string
//       Access: Public
//  Description: Returns a string corresponding to the
//               TextureProperties object.  Each unique set of
//               TextureProperties will generate a unique string.
//               This is used to generate unique palette image
//               filenames.
////////////////////////////////////////////////////////////////////
string TextureProperties::
get_string() const {
  string result;

  if (_got_num_channels) {
    ostringstream num;
    num << _num_channels;
    result += num.str();
  }
  result += get_format_string(_format);
  result += get_filter_string(_minfilter);
  result += get_filter_string(_magfilter);
  result += get_type_string(_color_type, _alpha_type);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::update_properties
//       Access: Public
//  Description: If the indicate TextureProperties structure is more
//               specific than this one, updates this one.
////////////////////////////////////////////////////////////////////
void TextureProperties::
update_properties(const TextureProperties &other) {
  if (!_got_num_channels) {
    _got_num_channels = other._got_num_channels;
    _num_channels = other._num_channels;
  }
  _format = union_format(_format, other._format);
  _minfilter = union_filter(_minfilter, other._minfilter);
  _magfilter = union_filter(_magfilter, other._magfilter);

  if (_color_type == (PNMFileType *)NULL) {
    _color_type = other._color_type;
    _alpha_type = other._alpha_type;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::fully_define
//       Access: Public
//  Description: If any properties remain unspecified, specify them
//               now.  Also reconcile conflicting information.
////////////////////////////////////////////////////////////////////
void TextureProperties::
fully_define() {
  if (!_got_num_channels) {
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

  // Make sure the format reflects the number of channels.
  switch (_num_channels) {
  case 1:
    switch (_format) {
    case EggTexture::F_red:
    case EggTexture::F_green:
    case EggTexture::F_blue:
    case EggTexture::F_alpha:
    case EggTexture::F_luminance:
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

    case EggTexture::F_rgba8:
      _format = EggTexture::F_rgb8;
      break;

    case EggTexture::F_rgba5:
    case EggTexture::F_rgba4:
      _format = EggTexture::F_rgb5;
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

    default:
      _format = EggTexture::F_rgba;
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

  if (_color_type == (PNMFileType *)NULL) {
    _color_type = pal->_color_type;
    _alpha_type = pal->_alpha_type;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::update_egg_tex
//       Access: Public
//  Description: Adjusts the texture properties of the indicated egg
//               reference to match these properties.
////////////////////////////////////////////////////////////////////
void TextureProperties::
update_egg_tex(EggTexture *egg_tex) const {
  egg_tex->set_format(_format);
  egg_tex->set_minfilter(_minfilter);
  egg_tex->set_magfilter(_minfilter);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::egg_properties_match
//       Access: Public
//  Description: Returns true if all of the properties that are
//               reflected directly in an egg file match between this
//               TextureProperties object and the other, or false if
//               any of them differ.
////////////////////////////////////////////////////////////////////
bool TextureProperties::
egg_properties_match(const TextureProperties &other) const {
  return (_format == other._format &&
          _minfilter == other._minfilter &&
          _magfilter == other._magfilter);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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
  if (_color_type != other._color_type) {
    return _color_type < other._color_type;
  }
  if (_color_type != (PNMFileType *)NULL) {
    if (_alpha_type != other._alpha_type) {
      return _alpha_type < other._alpha_type;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool TextureProperties::
operator == (const TextureProperties &other) const {
  return (_format == other._format &&
          _minfilter == other._minfilter &&
          _magfilter == other._magfilter &&
          _color_type == other._color_type &&
          (_color_type == (PNMFileType *)NULL ||
           _alpha_type == other._alpha_type));
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::Nonequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool TextureProperties::
operator != (const TextureProperties &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::get_format_string
//       Access: Private, Static
//  Description: Returns a short string representing the given
//               EggTexture format.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::get_filter_string
//       Access: Private, Static
//  Description: Returns a short string representing the given
//               EggTexture filter type.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::get_type_string
//       Access: Private, Static
//  Description: Returns a short string representing whether the color
//               and/or alpha type has been specified or not.
////////////////////////////////////////////////////////////////////
string TextureProperties::
get_type_string(PNMFileType *color_type, PNMFileType *alpha_type) {
  if (color_type == (PNMFileType *)NULL) {
    return "";
  }
  if (alpha_type == (PNMFileType *)NULL) {
    return "c";
  }
  return "a";
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::union_format
//       Access: Private, Static
//  Description: Returns the EggTexture format which is the more
//               specific of the two.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::union_filter
//       Access: Private, Static
//  Description: Returns the EggTexture filter type which is the more
//               specific of the two.
////////////////////////////////////////////////////////////////////
EggTexture::FilterType TextureProperties::
union_filter(EggTexture::FilterType a, EggTexture::FilterType b) {
  if ((int)a < (int)b) {
    return b;
  } else {
    return a;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void TextureProperties::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_TextureProperties);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void TextureProperties::
write_datagram(BamWriter *writer, Datagram &datagram) {
  datagram.add_bool(_got_num_channels);
  datagram.add_int32(_num_channels);
  datagram.add_int32((int)_format);
  datagram.add_int32((int)_minfilter);
  datagram.add_int32((int)_magfilter);
  writer->write_pointer(datagram, _color_type);
  writer->write_pointer(datagram, _alpha_type);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::complete_pointers
//       Access: Public, Virtual
//  Description: Called after the object is otherwise completely read
//               from a Bam file, this function's job is to store the
//               pointers that were retrieved from the Bam file for
//               each pointer object written.  The return value is the
//               number of pointers processed from the list.
////////////////////////////////////////////////////////////////////
int TextureProperties::
complete_pointers(vector_typedWritable &p_list, BamReader *manager) {
  nassertr(p_list.size() >= 2, 0);
  int index = 0;

  if (p_list[index] != (TypedWritable *)NULL) {
    DCAST_INTO_R(_color_type, p_list[index], index);
  }
  index++;

  if (p_list[index] != (TypedWritable *)NULL) {
    DCAST_INTO_R(_alpha_type, p_list[index], index);
  }
  index++;

  return index;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::make_TextureProperties
//       Access: Protected
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
////////////////////////////////////////////////////////////////////
TypedWritable* TextureProperties::
make_TextureProperties(const FactoryParams &params) {
  TextureProperties *me = new TextureProperties;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureProperties::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void TextureProperties::
fillin(DatagramIterator &scan, BamReader *manager) {
  _got_num_channels = scan.get_bool();
  _num_channels = scan.get_int32();
  _format = (EggTexture::Format)scan.get_int32();
  _minfilter = (EggTexture::FilterType)scan.get_int32();
  _magfilter = (EggTexture::FilterType)scan.get_int32();
  manager->read_pointer(scan, this);  // _color_type
  manager->read_pointer(scan, this);  // _alpha_type
}
