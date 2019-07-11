/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texture.h
 * @author mike
 * @date 1997-01-09
 * @author fperazzi, PandaSE
 * @date 2010-04-29
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "pandabase.h"

#include "filename.h"
#include "typedWritableReferenceCount.h"
#include "namable.h"
#include "internalName.h"
#include "graphicsStateGuardianBase.h"
#include "updateSeq.h"
#include "pmap.h"
#include "config_gobj.h"
#include "pStatCollector.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "conditionVar.h"
#include "loaderOptions.h"
#include "string_utils.h"
#include "cycleData.h"
#include "cycleDataLockedReader.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"
#include "samplerState.h"
#include "colorSpace.h"
#include "geomEnums.h"
#include "bamCacheRecord.h"
#include "pnmImage.h"
#include "pfmFile.h"
#include "asyncFuture.h"

class TextureContext;
class FactoryParams;
class PreparedGraphicsObjects;
class CullTraverser;
class CullTraverserData;
class TexturePeeker;
struct DDSHeader;

/**
 * Represents a texture object, which is typically a single 2-d image but may
 * also represent a 1-d or 3-d texture image, or the six 2-d faces of a cube
 * map texture.
 *
 * A texture's image data might be stored in system RAM (see get_ram_image())
 * or its image may be represented in texture memory on one or more
 * GraphicsStateGuardians (see prepare()), or both.  The typical usage pattern
 * is that a texture is loaded from an image file on disk, which copies its
 * image data into system RAM; then the first time the texture is rendered its
 * image data is copied to texture memory (actually, to the graphics API), and
 * the system RAM image is automatically freed.
 */
class EXPCL_PANDA_GOBJ Texture : public TypedWritableReferenceCount, public Namable {
PUBLISHED:
  typedef PT(Texture) MakeTextureFunc();

  enum TextureType {
    TT_1d_texture,
    TT_2d_texture,
    TT_3d_texture,
    TT_2d_texture_array,
    TT_cube_map,
    TT_buffer_texture,
    TT_cube_map_array,
    TT_1d_texture_array,
  };

  enum ComponentType {
    T_unsigned_byte,
    T_unsigned_short,
    T_float,
    T_unsigned_int_24_8,  // Packed
    T_int,
    T_byte,
    T_short,
    T_half_float,
    T_unsigned_int,
  };

  enum Format {
    F_depth_stencil = 1,
    F_color_index,
    F_red,
    F_green,
    F_blue,
    F_alpha,
    F_rgb,     // any suitable RGB mode, whatever the hardware prefers

    // The following request a particular number of bits for the GSG's
    // internal_format (as stored in the framebuffer), but this request is not
    // related to the pixel storage within the Texture object itself, which is
    // always get_num_components() * get_component_width().
    F_rgb5,    // 5 bits per R,G,B channel
    F_rgb8,    // 8 bits per R,G,B channel
    F_rgb12,   // 12 bits per R,G,B channel
    F_rgb332,  // 3 bits per R & G, 2 bits for B

    F_rgba,    // any suitable RGBA mode, whatever the hardware prefers

    // Again, the following bitdepth requests are only for the GSG; within the
    // Texture object itself, these are all equivalent.
    F_rgbm,    // as above, but only requires 1 bit for alpha (i.e. mask)
    F_rgba4,   // 4 bits per R,G,B,A channel
    F_rgba5,   // 5 bits per R,G,B channel, 1 bit alpha
    F_rgba8,   // 8 bits per R,G,B,A channel
    F_rgba12,  // 12 bits per R,G,B,A channel

    F_luminance,
    F_luminance_alpha,      // 8 bits luminance, 8 bits alpha
    F_luminance_alphamask,  // 8 bits luminance, only needs 1 bit of alpha

    F_rgba16,  // 16 bits per R,G,B,A channel
    F_rgba32,  // 32 bits per R,G,B,A channel

    F_depth_component,
    F_depth_component16,
    F_depth_component24,
    F_depth_component32,

    F_r16,
    F_rg16,
    F_rgb16,

    // These formats are in the sRGB color space.  RGB is 2.2 gamma corrected,
    // alpha is always linear.
    F_srgb,
    F_srgb_alpha,
    F_sluminance,
    F_sluminance_alpha,

    F_r32i,  // 32-bit integer, used for atomic access
    F_r32,
    F_rg32,
    F_rgb32,

    F_r8i, // 8 integer bits per R channel
    F_rg8i, // 8 integer bits per R,G channel
    F_rgb8i, // 8 integer bits per R,G,B channel
    F_rgba8i, // 8 integer bits per R,G,B,A channel

    F_r11_g11_b10, // unsigned floating-point, 11 Red, 11 Green, 10 Blue Bits
    F_rgb9_e5,
    F_rgb10_a2,

    F_rg,
    F_r16i
  };

  // Deprecated.  See SamplerState.FilterType.
  enum DeprecatedFilterType {
    FT_nearest = SamplerState::FT_nearest,
    FT_linear = SamplerState::FT_linear,
    FT_nearest_mipmap_nearest = SamplerState::FT_nearest_mipmap_nearest,
    FT_linear_mipmap_nearest = SamplerState::FT_linear_mipmap_nearest,
    FT_nearest_mipmap_linear = SamplerState::FT_nearest_mipmap_linear,
    FT_linear_mipmap_linear = SamplerState::FT_linear_mipmap_linear,
    FT_shadow = SamplerState::FT_shadow,
    FT_default = SamplerState::FT_default,
    FT_invalid = SamplerState::FT_invalid
  };
  typedef SamplerState::FilterType FilterType;

  // Deprecated.  See SamplerState.WrapMode.
  enum DeprecatedWrapMode {
    WM_clamp = SamplerState::WM_clamp,
    WM_repeat = SamplerState::WM_repeat,
    WM_mirror = SamplerState::WM_mirror,
    WM_mirror_once = SamplerState::WM_mirror_once,
    WM_border_color = SamplerState::WM_border_color,
    WM_invalid = SamplerState::WM_invalid
  };
  typedef SamplerState::WrapMode WrapMode;

  enum CompressionMode {
    // Generic compression modes.  Usually, you should choose one of these.
    CM_default,  // on or off, according to compressed-textures
    CM_off,      // uncompressed image
    CM_on,       // whatever compression the driver supports

    // Specific compression modes.  Use only when you really want to use a
    // particular compression algorithm.  Use with caution; not all drivers
    // support all compression modes.  You can use
    // GSG::get_supports_compressed_texture_format() to query the available
    // compression modes for a particular GSG.
    CM_fxt1,
    CM_dxt1, // BC1: RGB with optional binary alpha.
    CM_dxt2, // Like DXT3, but assumes premultiplied alpha
    CM_dxt3, // BC2: RGB with uncompressed 4-bit alpha.
    CM_dxt4, // Like DXT5, but assumes premultiplied alpha
    CM_dxt5, // BC3: RGB with separately compressed 8-bit alpha.
    CM_pvr1_2bpp,
    CM_pvr1_4bpp,
    CM_rgtc, // BC4/BC5: 1 or 2 channels, individually compressed.
    CM_etc1,
    CM_etc2,
    CM_eac, // EAC: 1 or 2 channels.
  };

  enum QualityLevel {
    QL_default,   // according to texture-quality-level
    QL_fastest,
    QL_normal,
    QL_best,
  };

PUBLISHED:
  explicit Texture(const std::string &name = std::string());

protected:
  Texture(const Texture &copy);
  void operator = (const Texture &copy);

PUBLISHED:
  virtual ~Texture();

  INLINE PT(Texture) make_copy() const;
  INLINE void clear();

  INLINE void setup_texture(TextureType texture_type,
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
  INLINE void setup_2d_texture_array(int z_size = 1);
  INLINE void setup_2d_texture_array(int x_size, int y_size, int z_size,
                                     ComponentType component_type, Format format);
  INLINE void setup_cube_map_array(int num_cube_maps);
  INLINE void setup_cube_map_array(int size, int num_cube_maps,
                                   ComponentType component_type, Format format);
  INLINE void setup_buffer_texture(int size, ComponentType component_type,
                                   Format format, GeomEnums::UsageHint usage);
  void generate_normalization_cube_map(int size);
  void generate_alpha_scale_map();

  INLINE void clear_image();
  INLINE bool has_clear_color() const;
  INLINE LColor get_clear_color() const;
  INLINE void set_clear_color(const LColor &color);
  INLINE void clear_clear_color();
  INLINE vector_uchar get_clear_data() const;
  MAKE_PROPERTY2(clear_color, has_clear_color, get_clear_color,
                              set_clear_color, clear_clear_color);

  BLOCKING bool read(const Filename &fullpath, const LoaderOptions &options = LoaderOptions());
  BLOCKING bool read(const Filename &fullpath, const Filename &alpha_fullpath,
                     int primary_file_num_channels, int alpha_file_channel,
                     const LoaderOptions &options = LoaderOptions());
  BLOCKING bool read(const Filename &fullpath, int z, int n,
                     bool read_pages, bool read_mipmaps,
                     const LoaderOptions &options = LoaderOptions());
  BLOCKING bool read(const Filename &fullpath, const Filename &alpha_fullpath,
                     int primary_file_num_channels, int alpha_file_channel,
                     int z, int n, bool read_pages, bool read_mipmaps,
                     BamCacheRecord *record = nullptr,
                     const LoaderOptions &options = LoaderOptions());

  BLOCKING INLINE bool write(const Filename &fullpath);
  BLOCKING INLINE bool write(const Filename &fullpath, int z, int n,
                             bool write_pages, bool write_mipmaps);

  BLOCKING bool read_txo(std::istream &in, const std::string &filename = "");
  BLOCKING static PT(Texture) make_from_txo(std::istream &in, const std::string &filename = "");
  BLOCKING bool write_txo(std::ostream &out, const std::string &filename = "") const;
  BLOCKING bool read_dds(std::istream &in, const std::string &filename = "", bool header_only = false);
  BLOCKING bool read_ktx(std::istream &in, const std::string &filename = "", bool header_only = false);

  BLOCKING INLINE bool load(const PNMImage &pnmimage, const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE bool load(const PNMImage &pnmimage, int z, int n, const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE bool load(const PfmFile &pfm, const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE bool load(const PfmFile &pfm, int z, int n, const LoaderOptions &options = LoaderOptions());
  BLOCKING INLINE bool load_sub_image(const PNMImage &pnmimage, int x, int y, int z=0, int n=0);
  BLOCKING INLINE bool store(PNMImage &pnmimage) const;
  BLOCKING INLINE bool store(PNMImage &pnmimage, int z, int n) const;
  BLOCKING INLINE bool store(PfmFile &pfm) const;
  BLOCKING INLINE bool store(PfmFile &pfm, int z, int n) const;

  BLOCKING INLINE bool reload();
  BLOCKING Texture *load_related(const InternalName *suffix) const;

  INLINE bool has_filename() const;
  INLINE const Filename &get_filename() const;
  INLINE void set_filename(const Filename &filename);
  INLINE void clear_filename();
  MAKE_PROPERTY2(filename, has_filename, get_filename, set_filename, clear_filename);

  INLINE bool has_alpha_filename() const;
  INLINE const Filename &get_alpha_filename() const;
  INLINE void set_alpha_filename(const Filename &alpha_filename);
  INLINE void clear_alpha_filename();
  MAKE_PROPERTY2(alpha_filename, has_alpha_filename, get_alpha_filename, set_alpha_filename, clear_alpha_filename);

  INLINE bool has_fullpath() const;
  INLINE const Filename &get_fullpath() const;
  INLINE void set_fullpath(const Filename &fullpath);
  INLINE void clear_fullpath();
  MAKE_PROPERTY2(fullpath, has_fullpath, get_fullpath, set_fullpath, clear_fullpath);

  INLINE bool has_alpha_fullpath() const;
  INLINE const Filename &get_alpha_fullpath() const;
  INLINE void set_alpha_fullpath(const Filename &alpha_fullpath);
  INLINE void clear_alpha_fullpath();
  MAKE_PROPERTY2(alpha_fullpath, has_alpha_fullpath, get_alpha_fullpath, set_alpha_fullpath, clear_alpha_fullpath);

  INLINE int get_x_size() const;
  INLINE void set_x_size(int x_size);
  MAKE_PROPERTY(x_size, get_x_size, set_x_size);

  INLINE int get_y_size() const;
  INLINE void set_y_size(int y_size);
  MAKE_PROPERTY(y_size, get_y_size, set_y_size);

  INLINE int get_z_size() const;
  INLINE void set_z_size(int z_size);
  MAKE_PROPERTY(z_size, get_z_size, set_z_size);

  INLINE int get_num_views() const;
  INLINE void set_num_views(int num_views);
  MAKE_PROPERTY(num_views, get_num_views, set_num_views);

  INLINE int get_num_pages() const;
  INLINE int get_num_components() const;
  INLINE int get_component_width() const;
  INLINE TextureType get_texture_type() const;
  INLINE GeomEnums::UsageHint get_usage_hint() const;

  MAKE_PROPERTY(num_pages, get_num_pages);
  MAKE_PROPERTY(num_components, get_num_components);
  MAKE_PROPERTY(component_width, get_component_width);
  MAKE_PROPERTY(texture_type, get_texture_type);
  MAKE_PROPERTY(usage_hint, get_usage_hint);

  INLINE Format get_format() const;
  INLINE void set_format(Format format);
  MAKE_PROPERTY(format, get_format, set_format);

  INLINE ComponentType get_component_type() const;
  INLINE void set_component_type(ComponentType component_type);
  MAKE_PROPERTY(component_type, get_component_type, set_component_type);

  INLINE SamplerState::WrapMode get_wrap_u() const;
  INLINE void set_wrap_u(WrapMode wrap);
  MAKE_PROPERTY(wrap_u, get_wrap_u, set_wrap_u);

  INLINE SamplerState::WrapMode get_wrap_v() const;
  INLINE void set_wrap_v(WrapMode wrap);
  MAKE_PROPERTY(wrap_v, get_wrap_v, set_wrap_v);

  INLINE SamplerState::WrapMode get_wrap_w() const;
  INLINE void set_wrap_w(WrapMode wrap);
  MAKE_PROPERTY(wrap_w, get_wrap_w, set_wrap_w);

  INLINE SamplerState::FilterType get_minfilter() const;
  INLINE SamplerState::FilterType get_effective_minfilter() const;
  INLINE void set_minfilter(FilterType filter);
  MAKE_PROPERTY(minfilter, get_minfilter, set_minfilter);
  MAKE_PROPERTY(effective_minfilter, get_effective_minfilter);

  INLINE SamplerState::FilterType get_magfilter() const;
  INLINE SamplerState::FilterType get_effective_magfilter() const;
  INLINE void set_magfilter(FilterType filter);
  MAKE_PROPERTY(magfilter, get_magfilter, set_magfilter);
  MAKE_PROPERTY(effective_magfilter, get_effective_magfilter);

  INLINE int get_anisotropic_degree() const;
  INLINE int get_effective_anisotropic_degree() const;
  INLINE void set_anisotropic_degree(int anisotropic_degree);
  MAKE_PROPERTY(anisotropic_degree, get_anisotropic_degree, set_anisotropic_degree);
  MAKE_PROPERTY(effective_anisotropic_degree, get_effective_anisotropic_degree);

  INLINE LColor get_border_color() const;
  INLINE void set_border_color(const LColor &color);
  MAKE_PROPERTY(border_color, get_border_color, set_border_color);

  INLINE bool has_compression() const;
  INLINE CompressionMode get_compression() const;
  INLINE void set_compression(CompressionMode compression);
  MAKE_PROPERTY(compression, get_compression, set_compression); // Could maybe use has_compression here, too

  INLINE bool get_render_to_texture() const;
  INLINE void set_render_to_texture(bool render_to_texture);
  MAKE_PROPERTY(render_to_texture, get_render_to_texture, set_render_to_texture);

  INLINE const SamplerState &get_default_sampler() const;
  INLINE void set_default_sampler(const SamplerState &sampler);
  MAKE_PROPERTY(default_sampler, get_default_sampler, set_default_sampler);
  INLINE bool uses_mipmaps() const;

  INLINE QualityLevel get_quality_level() const;
  INLINE QualityLevel get_effective_quality_level() const;
  INLINE void set_quality_level(QualityLevel quality_level);
  MAKE_PROPERTY(quality_level, get_quality_level, set_quality_level);
  MAKE_PROPERTY(effective_quality_level, get_effective_quality_level);

  INLINE int get_expected_num_mipmap_levels() const;
  INLINE int get_expected_mipmap_x_size(int n) const;
  INLINE int get_expected_mipmap_y_size(int n) const;
  INLINE int get_expected_mipmap_z_size(int n) const;
  INLINE int get_expected_mipmap_num_pages(int n) const;
  MAKE_PROPERTY(expected_num_mipmap_levels, get_expected_num_mipmap_levels);

  INLINE bool has_ram_image() const;
  INLINE bool has_uncompressed_ram_image() const;
  INLINE bool might_have_ram_image() const;
  INLINE size_t get_ram_image_size() const;
  INLINE size_t get_ram_view_size() const;
  INLINE size_t get_ram_page_size() const;
  INLINE size_t get_expected_ram_image_size() const;
  INLINE size_t get_expected_ram_page_size() const;
  MAKE_PROPERTY(ram_image_size, get_ram_image_size);
  MAKE_PROPERTY(ram_view_size, get_ram_view_size);
  MAKE_PROPERTY(ram_page_size, get_ram_page_size);
  MAKE_PROPERTY(expected_ram_image_size, get_expected_ram_image_size);
  MAKE_PROPERTY(expected_ram_page_size, get_expected_ram_page_size);

  INLINE CPTA_uchar get_ram_image();
  INLINE CompressionMode get_ram_image_compression() const;
  INLINE CPTA_uchar get_uncompressed_ram_image();
  CPTA_uchar get_ram_image_as(const std::string &requested_format);
  INLINE PTA_uchar modify_ram_image();
  INLINE PTA_uchar make_ram_image();
#ifndef CPPPARSER
  INLINE void set_ram_image(CPTA_uchar image, CompressionMode compression = CM_off,
                            size_t page_size = 0);
  void set_ram_image_as(CPTA_uchar image, const std::string &provided_format);
#else
  EXTEND void set_ram_image(PyObject *image, CompressionMode compression = CM_off,
                            size_t page_size = 0);
  EXTEND void set_ram_image_as(PyObject *image, const std::string &provided_format);
#endif
  INLINE void clear_ram_image();
  INLINE void set_keep_ram_image(bool keep_ram_image);
  virtual bool get_keep_ram_image() const;
  virtual bool is_cacheable() const;

  MAKE_PROPERTY(ram_image_compression, get_ram_image_compression);
  MAKE_PROPERTY(keep_ram_image, get_keep_ram_image, set_keep_ram_image);
  MAKE_PROPERTY(cacheable, is_cacheable);

  BLOCKING INLINE bool compress_ram_image(CompressionMode compression = CM_on,
                                          QualityLevel quality_level = QL_default,
                                          GraphicsStateGuardianBase *gsg = nullptr);
  BLOCKING INLINE bool uncompress_ram_image();

  INLINE int get_num_ram_mipmap_images() const;
  INLINE bool has_ram_mipmap_image(int n) const;
  int get_num_loadable_ram_mipmap_images() const;
  INLINE bool has_all_ram_mipmap_images() const;
  INLINE size_t get_ram_mipmap_image_size(int n) const;
  INLINE size_t get_ram_mipmap_view_size(int n) const;
  INLINE size_t get_ram_mipmap_page_size(int n) const;
  INLINE size_t get_expected_ram_mipmap_image_size(int n) const;
  INLINE size_t get_expected_ram_mipmap_view_size(int n) const;
  INLINE size_t get_expected_ram_mipmap_page_size(int n) const;
  CPTA_uchar get_ram_mipmap_image(int n) const;
  void *get_ram_mipmap_pointer(int n) const;
  INLINE PTA_uchar modify_ram_mipmap_image(int n);
  INLINE PTA_uchar make_ram_mipmap_image(int n);
  void set_ram_mipmap_pointer(int n, void *image, size_t page_size = 0);
  void set_ram_mipmap_pointer_from_int(long long pointer, int n, int page_size);
  INLINE void set_ram_mipmap_image(int n, CPTA_uchar image, size_t page_size = 0);
  void clear_ram_mipmap_image(int n);
  INLINE void clear_ram_mipmap_images();
  INLINE void generate_ram_mipmap_images();

  MAKE_PROPERTY(num_ram_mipmap_images, get_num_ram_mipmap_images);
  MAKE_PROPERTY(num_loadable_ram_mipmap_images, get_num_loadable_ram_mipmap_images);

  INLINE int get_simple_x_size() const;
  INLINE int get_simple_y_size() const;
  INLINE bool has_simple_ram_image() const;
  INLINE size_t get_simple_ram_image_size() const;
  INLINE CPTA_uchar get_simple_ram_image() const;
  INLINE void set_simple_ram_image(CPTA_uchar image, int x_size, int y_size);
  PTA_uchar modify_simple_ram_image();
  PTA_uchar new_simple_ram_image(int x_size, int y_size);
  void generate_simple_ram_image();
  INLINE void clear_simple_ram_image();

  MAKE_PROPERTY(simple_x_size, get_simple_x_size);
  MAKE_PROPERTY(simple_y_size, get_simple_y_size);
  MAKE_PROPERTY2(simple_ram_image, has_simple_ram_image, get_simple_ram_image);

  PT(TexturePeeker) peek();

  INLINE UpdateSeq get_properties_modified() const;
  INLINE UpdateSeq get_image_modified() const;
  INLINE UpdateSeq get_simple_image_modified() const;
  MAKE_PROPERTY(properties_modified, get_properties_modified);
  MAKE_PROPERTY(image_modified, get_image_modified);
  MAKE_PROPERTY(simple_image_modified, get_simple_image_modified);

  INLINE bool has_auto_texture_scale() const;
  INLINE AutoTextureScale get_auto_texture_scale() const;
  INLINE void set_auto_texture_scale(AutoTextureScale scale);
  MAKE_PROPERTY(auto_texture_scale, get_auto_texture_scale,
                                    set_auto_texture_scale);

  PT(AsyncFuture) prepare(PreparedGraphicsObjects *prepared_objects);
  bool is_prepared(PreparedGraphicsObjects *prepared_objects) const;
  bool was_image_modified(PreparedGraphicsObjects *prepared_objects) const;
  size_t get_data_size_bytes(PreparedGraphicsObjects *prepared_objects) const;
  bool get_active(PreparedGraphicsObjects *prepared_objects) const;
  bool get_resident(PreparedGraphicsObjects *prepared_objects) const;

  bool release(PreparedGraphicsObjects *prepared_objects);
  int release_all();

  void write(std::ostream &out, int indent_level) const;

  size_t estimate_texture_memory() const;

  void set_aux_data(const std::string &key, TypedReferenceCount *aux_data);
  void clear_aux_data(const std::string &key);
  TypedReferenceCount *get_aux_data(const std::string &key) const;
  MAKE_MAP_PROPERTY(aux_data, get_aux_data, get_aux_data,
                    set_aux_data, clear_aux_data);

  INLINE static void set_textures_power_2(AutoTextureScale scale);
  INLINE static AutoTextureScale get_textures_power_2();
  INLINE static bool has_textures_power_2();

PUBLISHED:

  INLINE int get_pad_x_size() const;
  INLINE int get_pad_y_size() const;
  INLINE int get_pad_z_size() const;
  INLINE LVecBase2 get_tex_scale() const;

  INLINE void set_pad_size(int x=0, int y=0, int z=0);
  void set_size_padded(int x=1, int y=1, int z=1);

  INLINE int get_orig_file_x_size() const;
  INLINE int get_orig_file_y_size() const;
  INLINE int get_orig_file_z_size() const;

  MAKE_PROPERTY(orig_file_x_size, get_orig_file_x_size);
  MAKE_PROPERTY(orig_file_y_size, get_orig_file_y_size);
  MAKE_PROPERTY(orig_file_z_size, get_orig_file_z_size);

  void set_orig_file_size(int x, int y, int z = 1);

  INLINE void set_loaded_from_image(bool flag = true);
  INLINE bool get_loaded_from_image() const;
  MAKE_PROPERTY(loaded_from_image, get_loaded_from_image, set_loaded_from_image);

  INLINE void set_loaded_from_txo(bool flag = true);
  INLINE bool get_loaded_from_txo() const;
  MAKE_PROPERTY(loaded_from_txo, get_loaded_from_txo, set_loaded_from_txo);

  INLINE bool get_match_framebuffer_format() const;
  INLINE void set_match_framebuffer_format(bool flag);
  MAKE_PROPERTY(match_framebuffer_format, get_match_framebuffer_format,
                                          set_match_framebuffer_format);

  INLINE bool get_post_load_store_cache() const;
  INLINE void set_post_load_store_cache(bool flag);
  MAKE_PROPERTY(post_load_store_cache, get_post_load_store_cache,
                                       set_post_load_store_cache);

  TextureContext *prepare_now(int view,
                              PreparedGraphicsObjects *prepared_objects,
                              GraphicsStateGuardianBase *gsg);

  static int up_to_power_2(int value);
  static int down_to_power_2(int value);

  void consider_rescale(PNMImage &pnmimage);
  static void consider_rescale(PNMImage &pnmimage, const std::string &name, AutoTextureScale auto_texture_scale = ATS_unspecified);
  INLINE bool rescale_texture();

  static std::string format_texture_type(TextureType tt);
  static TextureType string_texture_type(const std::string &str);

  static std::string format_component_type(ComponentType ct);
  static ComponentType string_component_type(const std::string &str);

  static std::string format_format(Format f);
  static Format string_format(const std::string &str);

  static std::string format_compression_mode(CompressionMode cm);
  static CompressionMode string_compression_mode(const std::string &str);

  static std::string format_quality_level(QualityLevel tql);
  static QualityLevel string_quality_level(const std::string &str);

public:
  void texture_uploaded();

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

  static PT(Texture) make_texture();

public:
  static bool is_unsigned(ComponentType ctype);
  static bool is_specific(CompressionMode compression);
  static bool has_alpha(Format format);
  static bool has_binary_alpha(Format format);
  static bool is_srgb(Format format);

  static bool adjust_size(int &x_size, int &y_size, const std::string &name,
                          bool for_padding, AutoTextureScale auto_texture_scale = ATS_unspecified);
  INLINE bool adjust_this_size(int &x_size, int &y_size, const std::string &name,
                               bool for_padding) const;

  virtual void ensure_loader_type(const Filename &filename);

protected:
  class CData;
  class RamImage;

  virtual void reconsider_dirty();

  // All of the functions in this class that begin "do_" are protected
  // methods.  Many of them are implementations of public-facing versions of
  // the same methods.

  // All of these assume the CData lock is already held (and receive a CData
  // pointer representing that lock); generally, they also avoid adjusting the
  // _properties_modified and _image_modified semaphores.
  virtual bool do_adjust_this_size(const CData *cdata,
                                   int &x_size, int &y_size, const std::string &name,
                                   bool for_padding) const;

  virtual bool do_read(CData *cdata,
                       const Filename &fullpath, const Filename &alpha_fullpath,
                       int primary_file_num_channels, int alpha_file_channel,
                       int z, int n, bool read_pages, bool read_mipmaps,
                       const LoaderOptions &options, BamCacheRecord *record);
  virtual bool do_read_one(CData *cdata,
                           const Filename &fullpath, const Filename &alpha_fullpath,
                           int z, int n, int primary_file_num_channels, int alpha_file_channel,
                           const LoaderOptions &options,
                           bool header_only, BamCacheRecord *record);
  virtual bool do_load_one(CData *cdata,
                           const PNMImage &pnmimage, const std::string &name,
                           int z, int n, const LoaderOptions &options);
  virtual bool do_load_one(CData *cdata,
                           const PfmFile &pfm, const std::string &name,
                           int z, int n, const LoaderOptions &options);
  virtual bool do_load_sub_image(CData *cdata, const PNMImage &image,
                                 int x, int y, int z, int n);
  bool do_read_txo_file(CData *cdata, const Filename &fullpath);
  bool do_read_txo(CData *cdata, std::istream &in, const std::string &filename);
  bool do_read_dds_file(CData *cdata, const Filename &fullpath, bool header_only);
  bool do_read_dds(CData *cdata, std::istream &in, const std::string &filename, bool header_only);
  bool do_read_ktx_file(CData *cdata, const Filename &fullpath, bool header_only);
  bool do_read_ktx(CData *cdata, std::istream &in, const std::string &filename, bool header_only);

  bool do_write(CData *cdata, const Filename &fullpath, int z, int n,
                bool write_pages, bool write_mipmaps);
  bool do_write_one(CData *cdata, const Filename &fullpath, int z, int n);
  bool do_store_one(CData *cdata, PNMImage &pnmimage, int z, int n);
  bool do_store_one(CData *cdata, PfmFile &pfm, int z, int n);
  bool do_write_txo_file(const CData *cdata, const Filename &fullpath) const;
  bool do_write_txo(const CData *cdata, std::ostream &out, const std::string &filename) const;

  virtual CData *unlocked_ensure_ram_image(bool allow_compression);
  virtual void do_reload_ram_image(CData *cdata, bool allow_compression);

  PTA_uchar do_modify_ram_image(CData *cdata);
  PTA_uchar do_make_ram_image(CData *cdata);
  void do_set_ram_image(CData *cdata, CPTA_uchar image,
                        CompressionMode compression = CM_off, size_t page_size = 0);
  PTA_uchar do_modify_ram_mipmap_image(CData *cdata, int n);
  PTA_uchar do_make_ram_mipmap_image(CData *cdata, int n);
  void do_set_ram_mipmap_image(CData *cdata, int n, CPTA_uchar image, size_t page_size);
  size_t do_get_clear_data(const CData *cdata, unsigned char *into) const;

  bool consider_auto_process_ram_image(bool generate_mipmaps, bool allow_compression);
  bool do_consider_auto_process_ram_image(CData *cdata, bool generate_mipmaps,
                                          bool allow_compression);
  bool do_compress_ram_image(CData *cdata, CompressionMode compression,
                             QualityLevel quality_level,
                             GraphicsStateGuardianBase *gsg);
  bool do_uncompress_ram_image(CData *cdata);

  static void do_compress_ram_image_bc4(const RamImage &src, RamImage &dest,
                                        int x_size, int y_size, int z_size);
  static void do_compress_ram_image_bc5(const RamImage &src, RamImage &dest,
                                        int x_size, int y_size, int z_size);
  static void do_uncompress_ram_image_bc4(const RamImage &src, RamImage &dest,
                                          int x_size, int y_size, int z_size);
  static void do_uncompress_ram_image_bc5(const RamImage &src, RamImage &dest,
                                          int x_size, int y_size, int z_size);
  bool do_has_all_ram_mipmap_images(const CData *cdata) const;

  bool do_reconsider_z_size(CData *cdata, int z, const LoaderOptions &options);
  virtual void do_allocate_pages(CData *cdata);
  bool do_reconsider_image_properties(CData *cdata,
                                      int x_size, int y_size, int num_components,
                                      ComponentType component_type, int z,
                                      const LoaderOptions &options);
  bool do_rescale_texture(CData *cdata);

  virtual PT(Texture) make_copy_impl() const;
  PT(Texture) do_make_copy(const CData *cdata) const;
  void do_assign(CData *cdata, const Texture *copy, const CData *cdata_copy);
  virtual void do_clear(CData *cdata);
  void do_setup_texture(CData *cdata,
                        TextureType texture_type, int x_size, int y_size,
                        int z_size, ComponentType component_type,
                        Format format);
  void do_set_num_views(CData *cdata, int num_views);
  void do_set_format(CData *cdata, Format format);
  void do_set_component_type(CData *cdata, ComponentType component_type);
  void do_set_x_size(CData *cdata, int x_size);
  void do_set_y_size(CData *cdata, int y_size);
  void do_set_z_size(CData *cdata, int z_size);

  void do_set_wrap_u(CData *cdata, WrapMode wrap);
  void do_set_wrap_v(CData *cdata, WrapMode wrap);
  void do_set_wrap_w(CData *cdata, WrapMode wrap);
  void do_set_minfilter(CData *cdata, FilterType filter);
  void do_set_magfilter(CData *cdata, FilterType filter);
  void do_set_anisotropic_degree(CData *cdata, int anisotropic_degree);
  void do_set_border_color(CData *cdata, const LColor &color);
  void do_set_compression(CData *cdata, CompressionMode compression);
  void do_set_quality_level(CData *cdata, QualityLevel quality_level);

  bool do_has_compression(const CData *cdata) const;
  virtual bool do_has_ram_image(const CData *cdata) const;
  virtual bool do_has_uncompressed_ram_image(const CData *cdata) const;
  CPTA_uchar do_get_ram_image(CData *cdata);
  CPTA_uchar do_get_uncompressed_ram_image(CData *cdata);
  void do_set_simple_ram_image(CData *cdata, CPTA_uchar image, int x_size, int y_size);
  INLINE size_t do_get_ram_image_size(const CData *cdata) const;
  INLINE bool do_has_ram_mipmap_image(const CData *cdata, int n) const;
  int do_get_expected_num_mipmap_levels(const CData *cdata) const;
  INLINE size_t do_get_expected_ram_image_size(const CData *cdata) const;
  INLINE size_t do_get_expected_ram_view_size(const CData *cdata) const;
  INLINE size_t do_get_expected_ram_page_size(const CData *cdata) const;
  size_t do_get_ram_mipmap_page_size(const CData *cdata, int n) const;
  INLINE size_t do_get_expected_ram_mipmap_image_size(const CData *cdata, int n) const;
  INLINE size_t do_get_expected_ram_mipmap_view_size(const CData *cdata, int n) const;
  INLINE size_t do_get_expected_ram_mipmap_page_size(const CData *cdata, int n) const;
  int do_get_expected_mipmap_x_size(const CData *cdata, int n) const;
  int do_get_expected_mipmap_y_size(const CData *cdata, int n) const;
  int do_get_expected_mipmap_z_size(const CData *cdata, int n) const;
  INLINE int do_get_expected_mipmap_num_pages(const CData *cdata, int n) const;
  INLINE void do_clear_ram_image(CData *cdata);
  void do_clear_simple_ram_image(CData *cdata);
  void do_clear_ram_mipmap_images(CData *cdata);
  void do_generate_ram_mipmap_images(CData *cdata, bool allow_recompress);
  void do_set_pad_size(CData *cdata, int x, int y, int z);
  virtual bool do_can_reload(const CData *cdata) const;
  bool do_reload(CData *cdata);

  INLINE AutoTextureScale do_get_auto_texture_scale(const CData *cdata) const;

  virtual bool do_has_bam_rawdata(const CData *cdata) const;
  virtual void do_get_bam_rawdata(CData *cdata);

  // This nested class declaration is used below.
  class RamImage {
  public:
    INLINE RamImage();

    PTA_uchar _image;
    size_t _page_size;

    // If _pointer_image is non-NULL, it represents an external block of
    // memory that is used instead of the above PTA_uchar.
    void *_pointer_image;
  };

private:
  static void convert_from_pnmimage(PTA_uchar &image, size_t page_size,
                                    int row_stride, int x, int y, int z,
                                    const PNMImage &pnmimage,
                                    int num_components, int component_width);
  static void convert_from_pfm(PTA_uchar &image, size_t page_size,
                               int z, const PfmFile &pfm,
                               int num_components, int component_width);
  static bool convert_to_pnmimage(PNMImage &pnmimage, int x_size, int y_size,
                                  int num_components,
                                  ComponentType component_type, bool is_srgb,
                                  CPTA_uchar image, size_t page_size,
                                  int z);
  static bool convert_to_pfm(PfmFile &pfm, int x_size, int y_size,
                             int num_components, int component_width,
                             CPTA_uchar image, size_t page_size,
                             int z);
  static PTA_uchar read_dds_level_bgr8(Texture *tex, CData *cdata, const DDSHeader &header,
                                       int n, std::istream &in);
  static PTA_uchar read_dds_level_rgb8(Texture *tex, CData *cdata, const DDSHeader &header,
                                       int n, std::istream &in);
  static PTA_uchar read_dds_level_abgr8(Texture *tex, CData *cdata, const DDSHeader &header,
                                        int n, std::istream &in);
  static PTA_uchar read_dds_level_rgba8(Texture *tex, CData *cdata, const DDSHeader &header,
                                        int n, std::istream &in);
  static PTA_uchar read_dds_level_abgr16(Texture *tex, CData *cdata, const DDSHeader &header,
                                         int n, std::istream &in);
  static PTA_uchar read_dds_level_abgr32(Texture *tex, CData *cdata, const DDSHeader &header,
                                         int n, std::istream &in);
  static PTA_uchar read_dds_level_raw(Texture *tex, CData *cdata, const DDSHeader &header,
                                      int n, std::istream &in);
  static PTA_uchar read_dds_level_generic_uncompressed(Texture *tex, CData *cdata,
                                                       const DDSHeader &header,
                                                       int n, std::istream &in);
  static PTA_uchar read_dds_level_luminance_uncompressed(Texture *tex, CData *cdata,
                                                         const DDSHeader &header,
                                                         int n, std::istream &in);
  static PTA_uchar read_dds_level_bc1(Texture *tex, CData *cdata,
                                      const DDSHeader &header,
                                      int n, std::istream &in);
  static PTA_uchar read_dds_level_bc2(Texture *tex, CData *cdata,
                                      const DDSHeader &header,
                                      int n, std::istream &in);
  static PTA_uchar read_dds_level_bc3(Texture *tex, CData *cdata,
                                      const DDSHeader &header,
                                      int n, std::istream &in);
  static PTA_uchar read_dds_level_bc4(Texture *tex, CData *cdata,
                                      const DDSHeader &header,
                                      int n, std::istream &in);
  static PTA_uchar read_dds_level_bc5(Texture *tex, CData *cdata,
                                      const DDSHeader &header,
                                      int n, std::istream &in);

  void clear_prepared(int view, PreparedGraphicsObjects *prepared_objects);

  static void consider_downgrade(PNMImage &pnmimage, int num_channels, const std::string &name);

  static bool compare_images(const PNMImage &a, const PNMImage &b);

  INLINE static void store_unscaled_byte(unsigned char *&p, int value);
  INLINE static void store_unscaled_short(unsigned char *&p, int value);
  INLINE static void store_scaled_byte(unsigned char *&p, int value, double scale);
  INLINE static void store_scaled_short(unsigned char *&p, int value, double scale);
  INLINE static double get_unsigned_byte(const unsigned char *&p);
  INLINE static double get_unsigned_short(const unsigned char *&p);
  INLINE static double get_unsigned_int(const unsigned char *&p);
  INLINE static double get_unsigned_int_24(const unsigned char *&p);
  INLINE static double get_float(const unsigned char *&p);
  INLINE static double get_half_float(const unsigned char *&p);

  INLINE static bool is_txo_filename(const Filename &fullpath);
  INLINE static bool is_dds_filename(const Filename &fullpath);
  INLINE static bool is_ktx_filename(const Filename &fullpath);

  void do_filter_2d_mipmap_pages(const CData *cdata,
                                 RamImage &to, const RamImage &from,
                                 int x_size, int y_size) const;

  void do_filter_3d_mipmap_level(const CData *cdata,
                                 RamImage &to, const RamImage &from,
                                 int x_size, int y_size, int z_size) const;

  typedef void Filter2DComponent(unsigned char *&p,
                                 const unsigned char *&q,
                                 size_t pixel_size, size_t row_size);

  typedef void Filter3DComponent(unsigned char *&p,
                                 const unsigned char *&q,
                                 size_t pixel_size, size_t row_size,
                                 size_t page_size);

  static void filter_2d_unsigned_byte(unsigned char *&p,
                                      const unsigned char *&q,
                                      size_t pixel_size, size_t row_size);
  static void filter_2d_unsigned_byte_srgb(unsigned char *&p,
                                           const unsigned char *&q,
                                           size_t pixel_size, size_t row_size);
  static void filter_2d_unsigned_byte_srgb_sse2(unsigned char *&p,
                                                const unsigned char *&q,
                                                size_t pixel_size, size_t row_size);
  static void filter_2d_unsigned_short(unsigned char *&p,
                                       const unsigned char *&q,
                                       size_t pixel_size, size_t row_size);
  static void filter_2d_float(unsigned char *&p, const unsigned char *&q,
                              size_t pixel_size, size_t row_size);

  static void filter_3d_unsigned_byte(unsigned char *&p,
                                      const unsigned char *&q,
                                      size_t pixel_size, size_t row_size,
                                      size_t page_size);
  static void filter_3d_unsigned_byte_srgb(unsigned char *&p,
                                           const unsigned char *&q,
                                           size_t pixel_size, size_t row_size,
                                           size_t page_size);
  static void filter_3d_unsigned_byte_srgb_sse2(unsigned char *&p,
                                                const unsigned char *&q,
                                                size_t pixel_size, size_t row_size,
                                                size_t page_size);
  static void filter_3d_unsigned_short(unsigned char *&p,
                                       const unsigned char *&q,
                                       size_t pixel_size, size_t row_size,
                                       size_t page_size);
  static void filter_3d_float(unsigned char *&p, const unsigned char *&q,
                              size_t pixel_size, size_t row_size, size_t page_size);

  bool do_squish(CData *cdata, CompressionMode compression, int squish_flags);
  bool do_unsquish(CData *cdata, int squish_flags);

protected:
  typedef pvector<RamImage> RamImages;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_GOBJ CData : public CycleData {
  public:
    CData();
    CData(const CData &copy);
    ALLOC_DELETED_CHAIN(CData);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return Texture::get_class_type();
    }

    void do_assign(const CData *copy);
    INLINE void inc_properties_modified();
    INLINE void inc_image_modified();
    INLINE void inc_simple_image_modified();

    Filename _filename;
    Filename _alpha_filename;
    Filename _fullpath;
    Filename _alpha_fullpath;

    // The number of channels of the primary file we use.  1, 2, 3, or 4.
    int _primary_file_num_channels;

    // If we have a separate alpha file, this designates which channel in the
    // alpha file provides the alpha channel.  0 indicates the combined
    // grayscale value of rgb; otherwise, 1, 2, 3, or 4 are valid.
    int _alpha_file_channel;

    int _x_size;
    int _y_size;
    int _z_size;
    int _num_views;
    int _num_components;
    int _component_width;
    TextureType _texture_type;
    Format _format;
    ComponentType _component_type;
    GeomEnums::UsageHint _usage_hint;

    bool _loaded_from_image;
    bool _loaded_from_txo;
    bool _has_read_pages;
    bool _has_read_mipmaps;
    int _num_mipmap_levels_read;

    SamplerState _default_sampler;
    bool _keep_ram_image;
    LColor _border_color;
    CompressionMode _compression;
    bool _render_to_texture;
    bool _match_framebuffer_format;
    bool _post_load_store_cache;
    QualityLevel _quality_level;

    int _pad_x_size;
    int _pad_y_size;
    int _pad_z_size;

    int _orig_file_x_size;
    int _orig_file_y_size;

    AutoTextureScale _auto_texture_scale;
    CompressionMode _ram_image_compression;

    // There is usually one RamImage for the mipmap level 0 (the base image).
    // There may or may not also be additional images for the additional
    // mipmap levels.
    RamImages _ram_images;

    // This is the simple image, which may be loaded before the texture is
    // loaded from disk.  It exists only for 2-d textures.
    RamImage _simple_ram_image;
    int _simple_x_size;
    int _simple_y_size;
    int32_t _simple_image_date_generated;

    // This is the color that should be used when no image was given.
    bool _has_clear_color;
    LColor _clear_color;

    UpdateSeq _properties_modified;
    UpdateSeq _image_modified;
    UpdateSeq _simple_image_modified;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "Texture::CData");
    }

  private:
    static TypeHandle _type_handle;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

  // Protects the remaining members of this class.
  Mutex _lock;

  // Used to implement unlocked_reload_ram_image().
  ConditionVar _cvar;  // condition: _reloading is true.
  bool _reloading;

  // A Texture keeps a list (actually, a map) of all the
  // PreparedGraphicsObjects tables that it has been prepared into.  Each PGO
  // conversely keeps a list (a set) of all the Textures that have been
  // prepared there.  When either destructs, it removes itself from the
  // other's list.
  typedef pmap<int, TextureContext *> Contexts;
  typedef pmap<PreparedGraphicsObjects *, Contexts> PreparedViews;
  PreparedViews _prepared_views;

  // It is common, when using normal maps, specular maps, gloss maps, and
  // such, to use a file naming convention where the filenames of the special
  // maps are derived by concatenating a suffix to the name of the diffuse
  // map.  The following table enables lookup of the special maps given the
  // diffuse map and the suffix.
  typedef pmap<CPT(InternalName), PT(Texture)> RelatedTextures;
  RelatedTextures _related_textures;

  // The TexturePool finds this useful.
  Filename _texture_pool_key;

private:
  // The auxiliary data is not recorded to a bam file.
  typedef pmap<std::string, PT(TypedReferenceCount) > AuxData;
  AuxData _aux_data;

  static AutoTextureScale _textures_power_2;
  static PStatCollector _texture_read_pcollector;

  // Datagram stuff
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

  virtual void finalize(BamReader *manager);

protected:
  void do_write_datagram_header(CData *cdata, BamWriter *manager, Datagram &me, bool &has_rawdata);
  virtual void do_write_datagram_body(CData *cdata, BamWriter *manager, Datagram &me);
  virtual void do_write_datagram_rawdata(CData *cdata, BamWriter *manager, Datagram &me);
  static TypedWritable *make_from_bam(const FactoryParams &params);
  virtual TypedWritable *make_this_from_bam(const FactoryParams &params);
  virtual void do_fillin_body(CData *cdata, DatagramIterator &scan, BamReader *manager);
  virtual void do_fillin_rawdata(CData *cdata, DatagramIterator &scan, BamReader *manager);
  virtual void do_fillin_from(CData *cdata, const Texture *dummy);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "Texture",
                  TypedWritableReferenceCount::get_class_type());
    CData::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

  friend class TextureContext;
  friend class PreparedGraphicsObjects;
  friend class TexturePool;
  friend class TexturePeeker;
};

extern EXPCL_PANDA_GOBJ ConfigVariableEnum<Texture::QualityLevel> texture_quality_level;

EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, Texture::TextureType tt);
EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, Texture::ComponentType ct);
EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, Texture::Format f);

EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, Texture::CompressionMode cm);
EXPCL_PANDA_GOBJ std::ostream &operator << (std::ostream &out, Texture::QualityLevel tql);
EXPCL_PANDA_GOBJ std::istream &operator >> (std::istream &in, Texture::QualityLevel &tql);

#include "texture.I"

#endif
