// Filename: eggTexture.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef EGGTEXTURE_H
#define EGGTEXTURE_H

#include "pandabase.h"

#include "eggRenderMode.h"
#include "eggFilenameNode.h"
#include "eggTransform.h"

#include "pset.h"
#include "luse.h"


////////////////////////////////////////////////////////////////////
//       Class : EggTexture
// Description : Defines a texture map that may be applied to
//               geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggTexture : public EggFilenameNode, public EggRenderMode, public EggTransform {
PUBLISHED:
  EggTexture(const string &tref_name, const Filename &filename);
  EggTexture(const EggTexture &copy);
  EggTexture &operator = (const EggTexture &copy);
  virtual ~EggTexture(); 

  virtual void write(ostream &out, int indent_level) const;

  enum Equivalence {
    E_basename             = 0x001,
    E_extension            = 0x002,
    E_dirname              = 0x004,
    E_complete_filename    = 0x007,
    E_transform            = 0x008,
    E_attributes           = 0x010,
    E_tref_name            = 0x020,
  };

  bool is_equivalent_to(const EggTexture &other, int eq) const;
  bool sorts_less_than(const EggTexture &other, int eq) const;

  bool has_alpha_channel(int num_components) const;

  enum TextureType {
    TT_unspecified, TT_1d_texture,
    TT_2d_texture, TT_3d_texture, TT_cube_map
  };
  enum Format {
    F_unspecified,
    F_rgba, F_rgbm, F_rgba12, F_rgba8, F_rgba4, F_rgba5,
    F_rgb, F_rgb12, F_rgb8, F_rgb5, F_rgb332,
    F_red, F_green, F_blue, F_alpha, F_luminance,
    F_luminance_alpha, F_luminance_alphamask
  };
  enum CompressionMode {
    CM_default, CM_off, CM_on,
    CM_fxt1, CM_dxt1, CM_dxt2, CM_dxt3, CM_dxt4, CM_dxt5,
  };
  enum WrapMode {
    WM_unspecified, WM_clamp, WM_repeat,
    WM_mirror, WM_mirror_once, WM_border_color
  };
  enum FilterType {
    // Note that these type values match up, name-for-name, with a
    // similar enumerated type in Panda's Texture object.  However,
    // they do *not* match up numerically.  You must convert between
    // them using a switch statement.
    FT_unspecified,

    // Mag Filter and Min Filter
    FT_nearest,
    FT_linear,

    // Min Filter Only
    FT_nearest_mipmap_nearest,   // "mipmap point"
    FT_linear_mipmap_nearest,    // "mipmap linear"
    FT_nearest_mipmap_linear,    // "mipmap bilinear"
    FT_linear_mipmap_linear,     // "mipmap trilinear"
  };
  enum EnvType {
    ET_unspecified, 
    ET_modulate, 
    ET_decal,
    ET_blend,
    ET_replace,
    ET_add,
    ET_blend_color_scale,
    ET_modulate_glow,
    ET_modulate_gloss,
    ET_normal,
    ET_normal_height,
    ET_glow,
    ET_gloss,
    ET_height,
    ET_selector,
    ET_normal_gloss,
  };
  enum CombineMode {
    CM_unspecified,
    CM_replace,
    CM_modulate,
    CM_add,
    CM_add_signed,
    CM_interpolate,
    CM_subtract,
    CM_dot3_rgb,
    CM_dot3_rgba,
  };
  enum CombineChannel {
    CC_rgb = 0,
    CC_alpha = 1,
    CC_num_channels = 2,
  };
  enum CombineIndex {
    CI_num_indices = 3
  };
  enum CombineSource {
    CS_unspecified,
    CS_texture,
    CS_constant,
    CS_primary_color,
    CS_previous,
    CS_constant_color_scale,
    CS_last_saved_result,
  };
  enum CombineOperand {
    CO_unspecified,
    CO_src_color,
    CO_one_minus_src_color,
    CO_src_alpha,
    CO_one_minus_src_alpha,
  };
  enum TexGen {
    TG_unspecified,

    TG_eye_sphere_map,

    TG_world_cube_map,
    TG_eye_cube_map,

    TG_world_normal,
    TG_eye_normal,

    TG_world_position,
    TG_eye_position,

    TG_point_sprite,
  };
  enum QualityLevel {
    QL_unspecified,
    QL_default,
    QL_fastest,
    QL_normal,
    QL_best,
  };

  INLINE void set_texture_type(TextureType texture_type);
  INLINE TextureType get_texture_type() const;

  INLINE void set_format(Format format);
  INLINE Format get_format() const;

  INLINE void set_compression_mode(CompressionMode mode);
  INLINE CompressionMode get_compression_mode() const;

  INLINE void set_wrap_mode(WrapMode mode);
  INLINE WrapMode get_wrap_mode() const;

  INLINE void set_wrap_u(WrapMode mode);
  INLINE WrapMode get_wrap_u() const;
  INLINE WrapMode determine_wrap_u() const;

  INLINE void set_wrap_v(WrapMode mode);
  INLINE WrapMode get_wrap_v() const;
  INLINE WrapMode determine_wrap_v() const;

  INLINE void set_wrap_w(WrapMode mode);
  INLINE WrapMode get_wrap_w() const;
  INLINE WrapMode determine_wrap_w() const;

  INLINE void set_minfilter(FilterType type);
  INLINE FilterType get_minfilter() const;

  INLINE void set_magfilter(FilterType type);
  INLINE FilterType get_magfilter() const;

  INLINE void set_anisotropic_degree(int anisotropic_degree);
  INLINE void clear_anisotropic_degree();
  INLINE bool has_anisotropic_degree() const;
  INLINE int get_anisotropic_degree() const;

  INLINE void set_env_type(EnvType type);
  INLINE EnvType get_env_type() const;
  bool affects_polygon_alpha() const;

  INLINE void set_combine_mode(CombineChannel channel, CombineMode cm);
  INLINE CombineMode get_combine_mode(CombineChannel channel) const;
  INLINE void set_combine_source(CombineChannel channel, int n, CombineSource cs);
  INLINE CombineSource get_combine_source(CombineChannel channel, int n) const;
  INLINE void set_combine_operand(CombineChannel channel, int n, CombineOperand co);
  INLINE CombineOperand get_combine_operand(CombineChannel channel, int n) const;

  INLINE void set_saved_result(bool saved_result);
  INLINE bool get_saved_result() const;

  INLINE void set_tex_gen(TexGen tex_gen);
  INLINE TexGen get_tex_gen() const;

  INLINE void set_quality_level(QualityLevel quality_level);
  INLINE QualityLevel get_quality_level() const;

  INLINE void set_stage_name(const string &stage_name);
  INLINE void clear_stage_name();
  INLINE bool has_stage_name() const;
  INLINE const string &get_stage_name() const;

  INLINE void set_priority(int priority);
  INLINE void clear_priority();
  INLINE bool has_priority() const;
  INLINE int get_priority() const;

  INLINE void set_color(const LColor &color);
  INLINE void clear_color();
  INLINE bool has_color() const;
  INLINE const LColor &get_color() const;

  INLINE void set_border_color(const LColor &border_color);
  INLINE void clear_border_color();
  INLINE bool has_border_color() const;
  INLINE const LColor &get_border_color() const;

  INLINE void set_uv_name(const string &uv_name);
  INLINE void clear_uv_name();
  INLINE bool has_uv_name() const;
  INLINE const string &get_uv_name() const;

  INLINE void set_rgb_scale(int rgb_scale);
  INLINE void clear_rgb_scale();
  INLINE bool has_rgb_scale() const;
  INLINE int get_rgb_scale() const;

  INLINE void set_alpha_scale(int alpha_scale);
  INLINE void clear_alpha_scale();
  INLINE bool has_alpha_scale() const;
  INLINE int get_alpha_scale() const;

  INLINE void set_alpha_filename(const Filename &filename);
  INLINE void clear_alpha_filename();
  INLINE bool has_alpha_filename() const;
  INLINE const Filename &get_alpha_filename() const;

  INLINE void set_alpha_fullpath(const Filename &fullpath);
  INLINE const Filename &get_alpha_fullpath() const;

  INLINE void set_alpha_file_channel(int alpha_file_channel);
  INLINE void clear_alpha_file_channel();
  INLINE bool has_alpha_file_channel() const;
  INLINE int get_alpha_file_channel() const;

  INLINE void set_multiview(bool multiview);
  INLINE bool get_multiview() const;

  INLINE void set_num_views(int num_views);
  INLINE void clear_num_views();
  INLINE bool has_num_views() const;
  INLINE int get_num_views() const;

  INLINE void set_read_mipmaps(bool read_mipmaps);
  INLINE bool get_read_mipmaps() const;

  INLINE void set_min_lod(double min_lod);
  INLINE void clear_min_lod();
  INLINE bool has_min_lod() const;
  INLINE double get_min_lod() const;

  INLINE void set_max_lod(double max_lod);
  INLINE void clear_max_lod();
  INLINE bool has_max_lod() const;
  INLINE double get_max_lod() const;

  INLINE void set_lod_bias(double lod_bias);
  INLINE void clear_lod_bias();
  INLINE bool has_lod_bias() const;
  INLINE double get_lod_bias() const;

  void clear_multitexture();
  bool multitexture_over(EggTexture *other);
  INLINE int get_multitexture_sort() const;

  static TextureType string_texture_type(const string &string);
  static Format string_format(const string &string);
  static CompressionMode string_compression_mode(const string &string);
  static WrapMode string_wrap_mode(const string &string);
  static FilterType string_filter_type(const string &string);
  static EnvType string_env_type(const string &string);
  static CombineMode string_combine_mode(const string &string);
  static CombineSource string_combine_source(const string &string);
  static CombineOperand string_combine_operand(const string &string);
  static TexGen string_tex_gen(const string &string);
  static QualityLevel string_quality_level(const string &string);

public:
  virtual EggTransform *as_transform();

protected:
  virtual bool egg_start_parse_body();

private:
  typedef pset<EggTexture *> MultiTextures;
  bool r_min_multitexture_sort(int sort, MultiTextures &cycle_detector);

  enum Flags {
    F_has_alpha_filename     = 0x0002,
    F_has_anisotropic_degree = 0x0004,
    F_has_alpha_file_channel = 0x0008,
    F_has_stage_name         = 0x0010,
    F_has_uv_name            = 0x0020,
    F_has_priority           = 0x0040,
    F_has_color              = 0x0080,
    F_has_rgb_scale          = 0x0100,
    F_has_alpha_scale        = 0x0200,
    F_has_border_color       = 0x0400,
    F_has_num_views          = 0x0800,
    F_has_min_lod            = 0x1000,
    F_has_max_lod            = 0x2000,
    F_has_lod_bias           = 0x4000,
  };

  TextureType _texture_type;
  Format _format;
  CompressionMode _compression_mode;
  WrapMode _wrap_mode, _wrap_u, _wrap_v, _wrap_w;
  FilterType _minfilter, _magfilter;
  int _anisotropic_degree;
  EnvType _env_type;
  bool _saved_result;
  bool _multiview;
  int _num_views;
  TexGen _tex_gen;
  QualityLevel _quality_level;
  string _stage_name;
  int _priority;
  LColor _color;
  LColor _border_color;
  string _uv_name;
  int _rgb_scale;
  int _alpha_scale;
  int _flags;
  Filename _alpha_filename;
  Filename _alpha_fullpath;
  int _alpha_file_channel;
  bool _read_mipmaps;
  int _multitexture_sort;
  double _min_lod;
  double _max_lod;
  double _lod_bias;

  class SourceAndOperand {
  public:
    INLINE SourceAndOperand();
    CombineSource _source;
    CombineOperand _operand;
  };

  class Combiner {
  public:
    INLINE Combiner();
    CombineMode _mode;
    SourceAndOperand _ops[CI_num_indices];
  };

  Combiner _combiner[CC_num_channels];

  // This is the set of all of the textures that are multitextured on
  // top of (and under) this one.  This is filled in by
  // multitexture_over().
  MultiTextures _over_textures, _under_textures;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggFilenameNode::init_type();
    EggRenderMode::init_type();
    register_type(_type_handle, "EggTexture",
                  EggFilenameNode::get_class_type(),
          EggRenderMode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

////////////////////////////////////////////////////////////////////
//       Class : UniqueEggTextures
// Description : An STL function object for sorting textures into
//               order by properties.  Returns true if the two
//               referenced EggTexture pointers are in sorted order,
//               false otherwise.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG UniqueEggTextures {
public:
  INLINE UniqueEggTextures(int eq = ~0);
  INLINE bool operator ()(const EggTexture *t1, const EggTexture *t2) const;

  int _eq;
};

INLINE ostream &operator << (ostream &out, const EggTexture &n) {
  return out << n.get_filename();
}

EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::TextureType texture_type);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::Format format);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::CompressionMode mode);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::WrapMode mode);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::FilterType type);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::EnvType type);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::CombineMode cm);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::CombineChannel cc);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::CombineSource cs);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::CombineOperand co);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::TexGen tex_gen);
EXPCL_PANDAEGG ostream &operator << (ostream &out, EggTexture::QualityLevel quality_level);

#include "eggTexture.I"

#endif
