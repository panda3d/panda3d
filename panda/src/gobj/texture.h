// Filename: texture.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTURE_H
#define TEXTURE_H

#include "pandabase.h"

#include "filename.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "graphicsStateGuardianBase.h"
#include "pmap.h"

class PNMImage;
class TextureContext;
class FactoryParams;
class PreparedGraphicsObjects;
class CullTraverser;
class CullTraverserData;

////////////////////////////////////////////////////////////////////
//       Class : Texture
// Description : Represents a texture object, which is typically a
//               single 2-d image but may also represent a 1-d or 3-d
//               texture image, or the six 2-d faces of a cube map
//               texture.
//
//               A texture's image data might be stored in system RAM
//               (see get_ram_image()) or its image may be represented
//               in texture memory on one or more
//               GraphicsStateGuardians (see prepare()), or both.  The
//               typical usage pattern is that a texture is loaded
//               from an image file on disk, which copies its image
//               data into system RAM; then the first time the texture
//               is rendered its image data is copied to texture
//               memory (actually, to the graphics API), and the
//               system RAM image is automatically freed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Texture : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  enum TextureType {
    TT_1d_texture,
    TT_2d_texture,
    TT_3d_texture,
    TT_cube_map,
  };

  enum ComponentType {
    T_unsigned_byte,
    T_unsigned_short,
    T_float,
  };

  enum Format {
    F_color_index,
    F_stencil_index,
    F_depth_component,
    F_red,
    F_green,
    F_blue,
    F_alpha,
    F_rgb,     // any suitable RGB mode, whatever the hardware prefers

    // The following request a particular number of bits for the GSG's
    // internal_format (as stored in the framebuffer), but this
    // request is not related to the pixel storage within the Texture
    // object itself, which is always get_num_components() *
    // get_component_width().
    F_rgb5,    // 5 bits per R,G,B channel
    F_rgb8,    // 8 bits per R,G,B channel
    F_rgb12,   // 12 bits per R,G,B channel
    F_rgb332,  // 3 bits per R & G, 2 bits for B

    F_rgba,    // any suitable RGBA mode, whatever the hardware prefers

    // Again, the following bitdepth requests are only for the GSG;
    // within the Texture object itself, these are all equivalent.
    F_rgbm,    // as above, but only requires 1 bit for alpha (i.e. mask)
    F_rgba4,   // 4 bits per R,G,B,A channel
    F_rgba5,   // 5 bits per R,G,B channel, 1 bit alpha
    F_rgba8,   // 8 bits per R,G,B,A channel
    F_rgba12,  // 12 bits per R,G,B,A channel

    F_luminance,
    F_luminance_alpha,      // 8 bits luminance, 8 bits alpha
    F_luminance_alphamask   // 8 bits luminance, only needs 1 bit of alpha
  };

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

    // A.k.a. trilinear filtering: Bilinear filter the pixel from
    // two mipmap levels, and linearly blend the results.
    FT_linear_mipmap_linear,

    // Returned by string_filter_type() for an invalid match.
    FT_invalid
  };

  enum WrapMode {
    WM_clamp,  // coords that would be outside [0-1] are clamped to 0 or 1
    WM_repeat,
    WM_mirror,
    WM_mirror_once,   // mirror once, then clamp
    WM_border_color,  // coords outside [0-1] use explict border color
    // Returned by string_wrap_mode() for an invalid match.
    WM_invalid
  };

PUBLISHED:
  Texture(const string &name = string());
protected:
  Texture(const Texture &copy);
PUBLISHED:
  virtual ~Texture();

  virtual PT(Texture) make_copy();

  void setup_texture(TextureType texture_type,
                     int x_size, int y_size, int z_size,
                     ComponentType component_type, Format format);

  INLINE void setup_1d_texture();
  INLINE void setup_1d_texture(int x_size,
                               ComponentType component_type, Format format);
  INLINE void setup_2d_texture();
  INLINE void setup_2d_texture(int x_size, int y_size,
                               ComponentType component_type, Format format);
  INLINE void setup_3d_texture(int z_size = 1);
  INLINE void setup_3d_texture(int x_size, int y_size, int z_size,
                               ComponentType component_type, Format format);
  INLINE void setup_cube_map();
  INLINE void setup_cube_map(int size,
                             ComponentType component_type, Format format);

  void generate_normalization_cube_map(int size);

  virtual bool read(const Filename &fullpath, int z = 0,
		    int primary_file_num_channels = 0);
  virtual bool read(const Filename &fullpath, const Filename &alpha_fullpath, 
		    int z = 0,
		    int primary_file_num_channels = 0, int alpha_file_channel = 0);
  bool write(const Filename &fullpath, int z = 0) const;

  bool read_pages(Filename fullpath_pattern, int z_size = 0);
  bool write_pages(Filename fullpath_pattern);

  virtual bool load(const PNMImage &pnmimage, int z = 0);
  bool store(PNMImage &pnmimage, int z = 0) const;

  INLINE bool has_filename() const;
  INLINE const Filename &get_filename() const;
  INLINE bool has_alpha_filename() const;
  INLINE const Filename &get_alpha_filename() const;

  INLINE bool has_fullpath() const;
  INLINE const Filename &get_fullpath() const;
  INLINE bool has_alpha_fullpath() const;
  INLINE const Filename &get_alpha_fullpath() const;

  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE int get_z_size() const;
  INLINE int get_num_components() const;
  INLINE int get_component_width() const;
  INLINE TextureType get_texture_type() const;
  INLINE Format get_format() const;
  INLINE ComponentType get_component_type() const;

  void set_wrap_u(WrapMode wrap);
  void set_wrap_v(WrapMode wrap);
  void set_wrap_w(WrapMode wrap);
  void set_minfilter(FilterType filter);
  void set_magfilter(FilterType filter);
  void set_anisotropic_degree(int anisotropic_degree);
  void set_border_color(const Colorf &color);

  INLINE WrapMode get_wrap_u() const;
  INLINE WrapMode get_wrap_v() const;
  INLINE WrapMode get_wrap_w() const;
  INLINE FilterType get_minfilter() const;
  INLINE FilterType get_magfilter() const;
  INLINE int get_anisotropic_degree() const;
  INLINE Colorf get_border_color() const;
  INLINE bool uses_mipmaps() const;

  virtual bool has_ram_image() const;
  INLINE bool might_have_ram_image() const;
  INLINE size_t get_ram_image_size() const;
  INLINE size_t get_expected_ram_image_size() const;
  INLINE size_t get_expected_ram_page_size() const;
  CPTA_uchar get_ram_image();
  PTA_uchar modify_ram_image();
  PTA_uchar make_ram_image();
  void set_ram_image(PTA_uchar image);
  void clear_ram_image();
  INLINE void set_keep_ram_image(bool keep_ram_image);
  virtual bool get_keep_ram_image() const;

  void prepare(PreparedGraphicsObjects *prepared_objects);
  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

PUBLISHED:
  // These are published, but in general, you shouldn't be mucking
  // with these values; they are set automatically when a texture is
  // loaded.
  INLINE void set_filename(const Filename &filename);
  INLINE void clear_filename();
  INLINE void set_alpha_filename(const Filename &alpha_filename);
  INLINE void clear_alpha_filename();

  INLINE void set_fullpath(const Filename &fullpath);
  INLINE void clear_fullpath();
  INLINE void set_alpha_fullpath(const Filename &alpha_fullpath);
  INLINE void clear_alpha_fullpath();

  INLINE void set_x_size(int x_size);
  INLINE void set_y_size(int y_size);
  INLINE void set_z_size(int z_size);
  void set_format(Format format);
  void set_component_type(ComponentType component_type);
  INLINE void set_loaded_from_disk();
  INLINE bool get_loaded_from_disk() const;

public:
  INLINE bool get_match_framebuffer_format() const;
  INLINE void set_match_framebuffer_format(bool flag);

  static bool is_mipmap(FilterType type);

  TextureContext *prepare_now(PreparedGraphicsObjects *prepared_objects, 
                              GraphicsStateGuardianBase *gsg);

  // These bits are used as parameters to Texture::mark_dirty() and
  // also TextureContext::mark_dirty() (and related functions in
  // TextureContext).
  enum DirtyFlags {
    DF_image      = 0x001,  // The image pixels have changed.
    DF_wrap       = 0x002,  // The wrap properties have changed.
    DF_filter     = 0x004,  // The minfilter or magfilter have changed.
    DF_mipmap     = 0x008,  // The use of mipmaps or not has changed.
    DF_border     = 0x010,  // The border has changed.
  };

  void mark_dirty(int flags_to_set);

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

  static WrapMode string_wrap_mode(const string &string);
  static FilterType string_filter_type(const string &string);

  static PT(Texture) make_texture();

protected:
  virtual void reconsider_dirty();
  virtual void reload_ram_image();

  static int up_to_power_2(int value);
  static int down_to_power_2(int value);
  bool reconsider_z_size(int z);
  bool reconsider_image_properties(int x_size, int y_size, int num_components,
				   ComponentType component_type, int z);

private:
  void clear_prepared(PreparedGraphicsObjects *prepared_objects);

  void consider_rescale(PNMImage &pnmimage);
  void consider_downgrade(PNMImage &pnmimage, int num_channels);

  INLINE void store_unscaled_byte(int &index, int value);
  INLINE void store_unscaled_short(int &index, int value);
  INLINE void store_scaled_byte(int &index, int value, double scale);
  INLINE void store_scaled_short(int &index, int value, double scale);
  INLINE double get_unsigned_byte(int &index) const;
  INLINE double get_unsigned_short(int &index) const;

protected:
  Filename _filename;
  Filename _alpha_filename;
  Filename _fullpath;
  Filename _alpha_fullpath;

  // The number of channels of the primary file we use.  1, 2, 3, or 4.
  int _primary_file_num_channels;

  // If we have a separate alpha file, this designates which channel
  // in the alpha file provides the alpha channel.  0 indicates the
  // combined grayscale value of rgb; otherwise, 1, 2, 3, or 4 are
  // valid.
  int _alpha_file_channel;

  int _x_size;
  int _y_size;
  int _z_size;
  int _num_components;
  int _component_width;
  TextureType _texture_type;
  Format _format;
  ComponentType _component_type;

  bool _loaded_from_disk;

  WrapMode _wrap_u;
  WrapMode _wrap_v;
  WrapMode _wrap_w;
  FilterType _minfilter;
  FilterType _magfilter;
  int _anisotropic_degree;
  bool _keep_ram_image;
  Colorf _border_color;
  bool _match_framebuffer_format;

  // A Texture keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.
  // Each PGO conversely keeps a list (a set) of all the Textures that
  // have been prepared there.  When either destructs, it removes
  // itself from the other's list.
  typedef pmap<PreparedGraphicsObjects *, TextureContext *> Contexts;
  Contexts _contexts;

  // This value represents the intersection of all the dirty flags of
  // the various TextureContexts that might be associated with this
  // texture.
  int _all_dirty_flags;

  PTA_uchar _image;


  // Datagram stuff
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager, bool has_rawdata);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Texture",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

  friend class TextureContext;
  friend class PreparedGraphicsObjects;
};

EXPCL_PANDA ostream &operator << (ostream &out, Texture::FilterType ft);
EXPCL_PANDA istream &operator >> (istream &in, Texture::FilterType &ft);

#include "texture.I"

#endif

