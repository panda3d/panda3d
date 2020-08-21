/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file samplerState.h
 * @author rdb
 * @date 2014-12-09
 */

#ifndef SAMPLERSTATE_H
#define SAMPLERSTATE_H

#include "pandabase.h"

#include "typedObject.h"
#include "namable.h"
#include "luse.h"
#include "numeric_types.h"
#include "bamReader.h"
#include "config_gobj.h"

class FactoryParams;
class GraphicsStateGuardianBase;
class PreparedGraphicsObjects;
class SamplerContext;

/**
 * Represents a set of settings that indicate how a texture is sampled.  This
 * can be used to sample the same texture using different settings in
 * different places.
 */
class EXPCL_PANDA_GOBJ SamplerState {
PUBLISHED:
  enum FilterType {
    // Mag Filter and Min Filter

    // Point sample the pixel
    FT_nearest,

    // Bilinear filtering of four neighboring pixels
    FT_linear,

    // Min Filter Only

    // Point sample the pixel from the nearest mipmap level
    FT_nearest_mipmap_nearest,

    // Bilinear filter the pixel from the nearest mipmap level
    FT_linear_mipmap_nearest,

    // Point sample the pixel from two mipmap levels, and linearly blend
    FT_nearest_mipmap_linear,

    // A.k.a.  trilinear filtering: Bilinear filter the pixel from two mipmap
    // levels, and linearly blend the results.
    FT_linear_mipmap_linear,

    // The OpenGL ARB_shadow extension can be thought of as a kind of
    // filtering.
    FT_shadow,

    // Default is usually linear, but it depends on format.  This was added at
    // the end of the list to avoid bumping TXO version #.
    FT_default,

    // Returned by string_filter_type() for an invalid match.
    FT_invalid
  };

  enum WrapMode {
    WM_clamp,  // coords that would be outside [0-1] are clamped to 0 or 1
    WM_repeat,
    WM_mirror,
    WM_mirror_once,   // mirror once, then clamp
    WM_border_color,  // coords outside [0-1] use explicit border color
    // Returned by string_wrap_mode() for an invalid match.
    WM_invalid
  };

  INLINE SamplerState();
  INLINE static const SamplerState &get_default();

  INLINE void set_wrap_u(WrapMode wrap);
  INLINE void set_wrap_v(WrapMode wrap);
  INLINE void set_wrap_w(WrapMode wrap);
  INLINE void set_minfilter(FilterType filter);
  INLINE void set_magfilter(FilterType filter);
  INLINE void set_anisotropic_degree(int anisotropic_degree);
  INLINE void set_border_color(const LColor &color);
  INLINE void set_min_lod(PN_stdfloat min_lod);
  INLINE void set_max_lod(PN_stdfloat max_lod);
  INLINE void set_lod_bias(PN_stdfloat lod_bias);

  INLINE WrapMode get_wrap_u() const;
  INLINE WrapMode get_wrap_v() const;
  INLINE WrapMode get_wrap_w() const;
  INLINE FilterType get_minfilter() const;
  INLINE FilterType get_magfilter() const;
  FilterType get_effective_minfilter() const;
  FilterType get_effective_magfilter() const;
  INLINE int get_anisotropic_degree() const;
  INLINE int get_effective_anisotropic_degree() const;
  INLINE const LColor &get_border_color() const;
  INLINE PN_stdfloat get_min_lod() const;
  INLINE PN_stdfloat get_max_lod() const;
  INLINE PN_stdfloat get_lod_bias() const;

  MAKE_PROPERTY(wrap_u, get_wrap_u, set_wrap_u);
  MAKE_PROPERTY(wrap_v, get_wrap_v, set_wrap_v);
  MAKE_PROPERTY(wrap_w, get_wrap_w, set_wrap_w);
  MAKE_PROPERTY(minfilter, get_minfilter, set_minfilter);
  MAKE_PROPERTY(magfilter, get_magfilter, set_magfilter);
  MAKE_PROPERTY(effective_minfilter, get_effective_minfilter);
  MAKE_PROPERTY(effective_magfilter, get_effective_magfilter);
  MAKE_PROPERTY(anisotropic_degree, get_anisotropic_degree, set_anisotropic_degree);
  MAKE_PROPERTY(effective_anisotropic_degree, get_effective_anisotropic_degree);
  MAKE_PROPERTY(border_color, get_border_color, set_border_color);
  MAKE_PROPERTY(min_lod, get_min_lod, set_min_lod);
  MAKE_PROPERTY(max_lod, get_max_lod, set_max_lod);
  MAKE_PROPERTY(lod_bias, get_lod_bias, set_lod_bias);

  INLINE bool uses_mipmaps() const;
  INLINE static bool is_mipmap(FilterType type);

  static std::string format_filter_type(FilterType ft);
  static FilterType string_filter_type(const std::string &str);

  static std::string format_wrap_mode(WrapMode wm);
  static WrapMode string_wrap_mode(const std::string &str);

  INLINE bool operator == (const SamplerState &other) const;
  INLINE bool operator != (const SamplerState &other) const;
  INLINE bool operator < (const SamplerState &other) const;

  void prepare(PreparedGraphicsObjects *prepared_objects) const;
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;
  void release(PreparedGraphicsObjects *prepared_objects) const;

  SamplerContext *prepare_now(PreparedGraphicsObjects *prepared_objects,
                              GraphicsStateGuardianBase *gsg) const;

public:
  int compare_to(const SamplerState &other) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent) const;

private:
  LColor _border_color;
  PN_stdfloat _min_lod;
  PN_stdfloat _max_lod;
  PN_stdfloat _lod_bias;

  // These are packed in a way that this class conveniently fits in 32 bytes;
  // feel free to change the packing as necessary when more enum values are
  // added.
  FilterType _minfilter : 4;
  FilterType _magfilter : 4;
  WrapMode _wrap_u : 4;
  WrapMode _wrap_v : 4;
  WrapMode _wrap_w : 4;
  int _anisotropic_degree : 12;

  static SamplerState _default;

public:
  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "SamplerState",
                  TypedObject::get_class_type());
  }
  /*virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}*/

private:

  static TypeHandle _type_handle;
};

extern EXPCL_PANDA_GOBJ ConfigVariableEnum<SamplerState::FilterType> texture_minfilter;
extern EXPCL_PANDA_GOBJ ConfigVariableEnum<SamplerState::FilterType> texture_magfilter;
extern EXPCL_PANDA_GOBJ ConfigVariableInt texture_anisotropic_degree;

INLINE std::ostream &operator << (std::ostream &out, const SamplerState &m) {
  m.output(out);
  return out;
}

INLINE std::ostream &operator << (std::ostream &out, SamplerState::FilterType ft) {
  return out << SamplerState::format_filter_type(ft);
}

INLINE std::istream &operator >> (std::istream &in, SamplerState::FilterType &ft) {
  std::string word;
  in >> word;
  ft = SamplerState::string_filter_type(word);
  return in;
}

INLINE std::ostream &operator << (std::ostream &out, SamplerState::WrapMode wm) {
  return out << SamplerState::format_wrap_mode(wm);
}

INLINE std::istream &operator >> (std::istream &in, SamplerState::WrapMode &wm) {
  std::string word;
  in >> word;
  wm = SamplerState::string_wrap_mode(word);
  return in;
}

#include "samplerState.I"

#endif
