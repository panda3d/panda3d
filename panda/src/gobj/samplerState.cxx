/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file samplerState.cxx
 * @author rdb
 * @date 2014-12-09
 */

#include "samplerState.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "samplerContext.h"
#include "preparedGraphicsObjects.h"

using std::string;

TypeHandle SamplerState::_type_handle;
SamplerState SamplerState::_default;

ConfigVariableEnum<SamplerState::FilterType> texture_minfilter
("texture-minfilter", SamplerState::FT_linear,
 PRC_DESC("This specifies the default minfilter that is applied to a texture "
          "in the absence of a specific minfilter setting.  Normally this "
          "is either 'linear' to disable mipmapping by default, or "
          "'mipmap', to enable trilinear mipmapping by default.  This "
          "does not apply to depth textures.  Note if this variable is "
          "changed at runtime, you may need to reload textures explicitly "
          "in order to change their visible properties."));

ConfigVariableEnum<SamplerState::FilterType> texture_magfilter
("texture-magfilter", SamplerState::FT_linear,
 PRC_DESC("This specifies the default magfilter that is applied to a texture "
          "in the absence of a specific magfilter setting.  Normally this "
          "is 'linear' (since mipmapping does not apply to magfilters).  This "
          "does not apply to depth textures.  Note if this variable is "
          "changed at runtime, you may need to reload textures explicitly "
          "in order to change their visible properties."));

ConfigVariableInt texture_anisotropic_degree
("texture-anisotropic-degree", 1,
 PRC_DESC("This specifies the default anisotropic degree that is applied "
          "to a texture in the absence of a particular anisotropic degree "
          "setting (that is, a texture for which the anisotropic degree "
          "is 0, meaning the default setting).  It should be 1 to disable "
          "anisotropic filtering, or a higher number to enable it.  "
          "Note if this variable is "
          "changed at runtime, you may need to reload textures explicitly "
          "in order to change their visible properties."));

/**
 * Returns the filter mode of the texture for minification, with special
 * treatment for FT_default.  This will normally not return FT_default, unless
 * there is an error in the config file.
 */
SamplerState::FilterType SamplerState::
get_effective_minfilter() const {
  if (_minfilter != FT_default) {
    return _minfilter;
  }
  return texture_minfilter;
}

/**
 * Returns the filter mode of the texture for magnification, with special
 * treatment for FT_default.  This will normally not return FT_default, unless
 * there is an error in the config file.
 */
SamplerState::FilterType SamplerState::
get_effective_magfilter() const {
  if (_magfilter != FT_default) {
    return _magfilter;
  }
  return texture_magfilter;
}

/**
 * Returns the indicated FilterType converted to a string word.
 */
string SamplerState::
format_filter_type(FilterType ft) {
  switch (ft) {
  case FT_nearest:
    return "nearest";
  case FT_linear:
    return "linear";

  case FT_nearest_mipmap_nearest:
    return "nearest_mipmap_nearest";
  case FT_linear_mipmap_nearest:
    return "linear_mipmap_nearest";
  case FT_nearest_mipmap_linear:
    return "nearest_mipmap_linear";
  case FT_linear_mipmap_linear:
    return "linear_mipmap_linear";

  case FT_shadow:
    return "shadow";

  case FT_default:
    return "default";

  case FT_invalid:
    return "invalid";
  }
  return "**invalid**";
}

/**
 * Returns the FilterType value associated with the given string
 * representation, or FT_invalid if the string does not match any known
 * FilterType value.
 */
SamplerState::FilterType SamplerState::
string_filter_type(const string &string) {
  if (cmp_nocase_uh(string, "nearest") == 0) {
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
  } else if (cmp_nocase_uh(string, "mipmap") == 0) {
    return FT_linear_mipmap_linear;
  } else if (cmp_nocase_uh(string, "shadow") == 0) {
    return FT_shadow;
  } else if (cmp_nocase_uh(string, "default") == 0) {
    return FT_default;
  } else {
    return FT_invalid;
  }
}

/**
 * Returns the indicated WrapMode converted to a string word.
 */
string SamplerState::
format_wrap_mode(WrapMode wm) {
  switch (wm) {
  case WM_clamp:
    return "clamp";
  case WM_repeat:
    return "repeat";
  case WM_mirror:
    return "mirror";
  case WM_mirror_once:
    return "mirror_once";
  case WM_border_color:
    return "border_color";

  case WM_invalid:
    return "invalid";
  }

  return "**invalid**";
}

/**
 * Returns the WrapMode value associated with the given string representation,
 * or WM_invalid if the string does not match any known WrapMode value.
 */
SamplerState::WrapMode SamplerState::
string_wrap_mode(const string &string) {
  if (cmp_nocase_uh(string, "repeat") == 0 ||
      cmp_nocase_uh(string, "wrap") == 0) {
    return WM_repeat;
  } else if (cmp_nocase_uh(string, "clamp") == 0) {
    return WM_clamp;
  } else if (cmp_nocase_uh(string, "mirror") == 0 ||
             cmp_nocase_uh(string, "mirrored_repeat") == 0) {
    return WM_mirror;
  } else if (cmp_nocase_uh(string, "mirror_once") == 0) {
    return WM_mirror_once;
  } else if (cmp_nocase_uh(string, "border_color") == 0 ||
             cmp_nocase_uh(string, "border") == 0) {
    return WM_border_color;
  } else {
    return WM_invalid;
  }
}

/**
 * Indicates that the sampler should be enqueued to be prepared in the
 * indicated prepared_objects at the beginning of the next frame.
 *
 * Use this function instead of prepare_now() to preload samplers from a user
 * interface standpoint.
 */
void SamplerState::
prepare(PreparedGraphicsObjects *prepared_objects) const {
  prepared_objects->enqueue_sampler(*this);
}

/**
 * Returns true if the sampler has already been prepared or enqueued for
 * preparation on the indicated GSG, false otherwise.
 */
bool SamplerState::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  return prepared_objects->is_sampler_queued(*this)
      || prepared_objects->is_sampler_prepared(*this);
}

/**
 * Frees the texture context only on the indicated object, if it exists there.
 * Returns true if it was released, false if it had not been prepared.
 */
void SamplerState::
release(PreparedGraphicsObjects *prepared_objects) const {
  prepared_objects->release_sampler(*this);
}

/**
 * Creates a context for the sampler on the particular GSG, if it does not
 * already exist.  Returns the new (or old) SamplerContext.  This assumes that
 * the GraphicsStateGuardian is the currently active rendering context and
 * that it is ready to accept new textures.  If this is not necessarily the
 * case, you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a sampler does not need to be explicitly prepared by the user before it may
 * be rendered.
 */
SamplerContext *SamplerState::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) const {
  return prepared_objects->prepare_sampler_now(*this, gsg);
}

/**
 * Returns a number less than zero if this sampler sorts before the other one,
 * greater than zero if it sorts after, or zero if they are equivalent.  The
 * sorting order is arbitrary and largely meaningless, except to differentiate
 * different sampler states.
 */
int SamplerState::
compare_to(const SamplerState &other) const {
  if (_wrap_u != other._wrap_u) {
    return (_wrap_u < other._wrap_u) ? -1 : 1;
  }
  if (_wrap_v != other._wrap_v) {
    return (_wrap_v < other._wrap_v) ? -1 : 1;
  }
  if (_wrap_w != other._wrap_w) {
    return (_wrap_w < other._wrap_w) ? -1 : 1;
  }
  if (_minfilter != other._minfilter) {
    return (_minfilter < other._minfilter) ? -1 : 1;
  }
  if (_magfilter != other._magfilter) {
    return (_magfilter < other._magfilter) ? -1 : 1;
  }
  if (_anisotropic_degree != other._anisotropic_degree) {
    return (_anisotropic_degree < other._anisotropic_degree) ? -1 : 1;
  }
  if (_border_color != other._border_color) {
    return (_border_color < other._border_color) ? -1 : 1;
  }
  if (_min_lod != other._min_lod) {
    return (_min_lod < other._min_lod) ? -1 : 1;
  }
  if (_max_lod != other._max_lod) {
    return (_max_lod < other._max_lod) ? -1 : 1;
  }
  if (_lod_bias != other._lod_bias) {
    return (_lod_bias < other._lod_bias) ? -1 : 1;
  }

  return 0;
}

/**
 *
 */
void SamplerState::
output(std::ostream &out) const {
  out
    << "sampler"
    << " wrap(u=" << _wrap_u << ", v=" << _wrap_v << ", w=" << _wrap_w
    << ", border=" << _border_color << ")"
    << " filter(min=" << _minfilter << ", mag=" << _magfilter
    << ", aniso=" << _anisotropic_degree << ")"
    << " lod(min=" << _min_lod << ", max=" << _max_lod
    << ", bias=" << _lod_bias << ")";
}

/**
 *
 */
void SamplerState::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "SamplerState\n";
  indent(out, indent_level) << "  wrap_u = " << _wrap_u << "\n";
  indent(out, indent_level) << "  wrap_v = " << _wrap_v << "\n";
  indent(out, indent_level) << "  wrap_w = " << _wrap_w << "\n";
  indent(out, indent_level) << "  minfilter = " << _minfilter << "\n";
  indent(out, indent_level) << "  magfilter = " << _magfilter << "\n";
  indent(out, indent_level) << "  anisotropic_degree = " << _anisotropic_degree << "\n";
  indent(out, indent_level) << "  border_color = " << _border_color << "\n";
  indent(out, indent_level) << "  min_lod = " << _min_lod << "\n";
  indent(out, indent_level) << "  max_lod = " << _max_lod << "\n";
  indent(out, indent_level) << "  lod_bias = " << _lod_bias << "\n";
}

/**
 * Encodes the sampler state into a datagram.
 */
void SamplerState::
write_datagram(Datagram &me) const {
  me.add_uint8(_wrap_u);
  me.add_uint8(_wrap_v);
  me.add_uint8(_wrap_w);
  me.add_uint8(_minfilter);
  me.add_uint8(_magfilter);
  me.add_int16(_anisotropic_degree);
  _border_color.write_datagram(me);
  me.add_stdfloat(_min_lod);
  me.add_stdfloat(_max_lod);
  me.add_stdfloat(_lod_bias);
}

/**
 * Reads the sampler state from the datagram that has been previously written
 * using write_datagram.
 */
void SamplerState::
read_datagram(DatagramIterator &scan, BamReader *manager) {
  _wrap_u = (WrapMode)scan.get_uint8();
  _wrap_v = (WrapMode)scan.get_uint8();
  _wrap_w = (WrapMode)scan.get_uint8();
  _minfilter = (FilterType)scan.get_uint8();
  _magfilter = (FilterType)scan.get_uint8();
  _anisotropic_degree = scan.get_int16();
  _border_color.read_datagram(scan);

  if (manager->get_file_minor_ver() >= 36) {
    // These were added with the introduction of SamplerState.  Since
    // Texture::do_fillin_body calls this, we still have to preserve backward
    // compatibility here.
    _min_lod = scan.get_stdfloat();
    _max_lod = scan.get_stdfloat();
    _lod_bias = scan.get_stdfloat();
  } else {
    _min_lod = -1000;
    _max_lod = 1000;
    _lod_bias = 0;
  }
}
