// Filename: eggTexture.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGTEXTURE_H
#define EGGTEXTURE_H

#include <pandabase.h>

#include "eggAlphaMode.h"
#include "eggFilenameNode.h"

#include <luse.h>


////////////////////////////////////////////////////////////////////
// 	 Class : EggTexture
// Description : Defines a texture map that may be applied to
//               geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggTexture : public EggFilenameNode, public EggAlphaMode {
public:
  EggTexture(const string &tref_name, const string &filename);
  EggTexture(const EggTexture &copy);
  EggTexture &operator = (const EggTexture &copy);
 
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

  enum Format {
    F_unspecified, 
    F_rgba, F_rgba12, F_rgba8, F_rgba4, F_rgba5,
    F_rgb, F_rgb12, F_rgb8, F_rgb5, F_rgb332,
    F_luminance_alpha,
    F_red, F_green, F_blue, F_alpha, F_luminance, 
  };
  enum WrapMode {
    WM_unspecified, WM_repeat, WM_clamp
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
    FT_nearest_mipmap_nearest,
    FT_linear_mipmap_nearest,
    FT_nearest_mipmap_linear,
    FT_linear_mipmap_linear,
  };
  enum EnvType {
    ET_unspecified, ET_modulate, ET_decal
  };

  INLINE void set_format(Format format);
  INLINE Format get_format() const;

  INLINE void set_wrap_mode(WrapMode mode);
  INLINE WrapMode get_wrap_mode() const;

  INLINE void set_wrap_u(WrapMode mode);
  INLINE WrapMode get_wrap_u() const;
  INLINE WrapMode determine_wrap_u() const;

  INLINE void set_wrap_v(WrapMode mode);
  INLINE WrapMode get_wrap_v() const;
  INLINE WrapMode determine_wrap_v() const;

  INLINE void set_minfilter(FilterType type);
  INLINE FilterType get_minfilter() const;

  INLINE void set_magfilter(FilterType type);
  INLINE FilterType get_magfilter() const;

  INLINE void set_magfilteralpha(FilterType type);
  INLINE FilterType get_magfilteralpha() const;

  INLINE void set_magfiltercolor(FilterType type);
  INLINE FilterType get_magfiltercolor() const;

  INLINE void set_anisotropic_degree(int anisotropic_degree);
  INLINE void clear_anisotropic_degree();
  INLINE bool has_anisotropic_degree() const;
  INLINE int get_anisotropic_degree() const;

  INLINE void set_env_type(EnvType type);
  INLINE EnvType get_env_type() const;

  INLINE void set_transform(const LMatrix3d &transform);
  INLINE void clear_transform();
  INLINE bool has_transform() const;
  INLINE LMatrix3d get_transform() const;
  INLINE bool transform_is_identity() const;

  INLINE void set_alpha_file(const Filename &filename);
  INLINE void clear_alpha_file();
  INLINE bool has_alpha_file() const;
  INLINE const Filename &get_alpha_file() const;
  INLINE Filename &update_alpha_file();

  static Format string_format(const string &string);
  static WrapMode string_wrap_mode(const string &string);
  static FilterType string_filter_type(const string &string);
  static EnvType string_env_type(const string &string);

private:
  enum Flags {
    F_has_transform          = 0x0001,
    F_has_alpha_file         = 0x0002,
    F_has_anisotropic_degree = 0x0004,
  };

  Format _format;
  WrapMode _wrap_mode, _wrap_u, _wrap_v;
  FilterType _minfilter, _magfilter, _magfilteralpha, _magfiltercolor;
  int _anisotropic_degree;
  EnvType _env_type;
  int _flags;
  LMatrix3d _transform;
  Filename _alpha_file;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggFilenameNode::init_type();
    EggAlphaMode::init_type();
    register_type(_type_handle, "EggTexture",
                  EggFilenameNode::get_class_type(),
		  EggAlphaMode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

///////////////////////////////////////////////////////////////////
// 	 Class : UniqueEggTextures
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

///////////////////////////////////////////////////////////////////
// 	 Class : TRefEggTextures
// Description : An STL function object for sorting textures into
//               order by TRef name.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG TRefEggTextures {
public:
  INLINE bool operator ()(const EggTexture *t1, const EggTexture *t2) const;
};

INLINE ostream &operator << (ostream &out, const EggTexture &n) {
  return out << (Filename &)n;
}

ostream EXPCL_PANDAEGG &operator << (ostream &out, EggTexture::Format format);
ostream EXPCL_PANDAEGG &operator << (ostream &out, EggTexture::WrapMode mode);
ostream EXPCL_PANDAEGG &operator << (ostream &out, EggTexture::FilterType type);
ostream EXPCL_PANDAEGG &operator << (ostream &out, EggTexture::EnvType type);

#include "eggTexture.I"

#endif
