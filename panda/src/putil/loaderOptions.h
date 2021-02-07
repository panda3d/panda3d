/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderOptions.h
 * @author drose
 * @date 2005-10-05
 */

#ifndef LOADEROPTIONS_H
#define LOADEROPTIONS_H

#include "pandabase.h"
#include "autoTextureScale.h"

/**
 * Specifies parameters that may be passed to the loader.
 */
class EXPCL_PANDA_PUTIL LoaderOptions {
PUBLISHED:
  // Flags for loading model files.
  enum LoaderFlags {
    LF_search            = 0x0001,
    LF_report_errors     = 0x0002,
    LF_convert_skeleton  = 0x0004,
    LF_convert_channels  = 0x0008,
    LF_convert_anim      = 0x000c,  // skeleton + channels
    LF_no_disk_cache     = 0x0010,  // disallow BamCache
    LF_no_ram_cache      = 0x0020,  // disallow ModelPool
    LF_no_cache          = 0x0030,  // no_disk + no_ram
    LF_cache_only        = 0x0040,  // fail if not in cache
    LF_allow_instance    = 0x0080,  // returned pointer might be shared
  };

  // Flags for loading texture files.
  enum TextureFlags {
    TF_preload           = 0x0004,  // Texture will have RAM image
    TF_preload_simple    = 0x0008,  // Texture will have simple RAM image
    TF_allow_1d          = 0x0010,  // If texture is Nx1, make a 1-d texture
    TF_generate_mipmaps  = 0x0020,  // Consider generating mipmaps
    TF_multiview         = 0x0040,  // Load a multiview texture in pages
    TF_integer           = 0x0080,  // Load as an integer (RGB) texture
    TF_float             = 0x0100,  // Load as a floating-point (depth) texture
    TF_allow_compression = 0x0200,  // Consider compressing RAM image
    TF_no_filters        = 0x0400,  // disallow using texture pool filters
  };

  enum TextureFormat {
    TFO1_unspecified        = 0x0001, //The format of the texture is unspecified
    TFO1_rgba               = 0x0002, //The rest of these formats correspond to texture formats
    TFO1_rgbm               = 0x0004, //see https://docs.panda3d.org/1.10/python/tools/model-export/egg-syntax 
    TFO1_rgba12             = 0x0008,  
    TFO1_rbga8              = 0x0010,  
    TFO1_rgba4              = 0x0020,  
    TFO1_rgba5              = 0x0040,  
    TFO1_rgb                = 0x0080,  
    TFO1_rgb12              = 0x0100,  
    TFO1_rgb8               = 0x0200,  
    TFO1_rgb5               = 0x0400,
    TF01_rgb332             = 0x0400,
    TF01_red                = 0x0800,
    TF01_green              = 0x1000,
    TF01_blue               = 0x2000,
    TF01_alpha              = 0x4000,
    TF01_luminance          = 0x8000,
  };

  enum TextureFormatCompress {
    TFC_luminance_alpha      = 0x0001,
    TFC_luminance_alphamask  = 0x0002,
    TFC_srgb                 = 0x0004,
    TFC_srbg_alpha           = 0x0008,
    TFC_CM_default           = 0x0010,
    TFC_CM_off               = 0x0020,
    TFC_CM_on                = 0x0040,
    TFC_CM_fxt1              = 0x0080,
    TFC_CM_dxt1              = 0x0100,
    TFC_CM_dxt2              = 0x0200,
    TFC_CM_dxt3              = 0x0400,
    TFC_CM_dxt4              = 0x0800,
    TFC_CM_dxt5              = 0x1000,
  };
  
  enum WrapOptionsOU {
    WO_wrap_unspecified      = 0x0001,
    WO_wrap_clamp            = 0x0002,
    WO_wrap_repeat           = 0x0004,
    WO_wrap_mirror           = 0x0008,
    WO_wrap_mirror_once      = 0x0010,
    WO_wrap_border_color     = 0x0020,
    WU_wrap_unspecified      = 0x0040,
    WU_wrap_clamp            = 0x0080,
    WU_wrap_repeat           = 0x0100,
    WU_wrap_mirror           = 0x0200,
    WU_wrap_mirror_once      = 0x0400,
    WU_wrap_border_color     = 0x0800,
  };

  enum WrapOptionsVW {
    WV_wrap_unspecified      = 0x0001,
    WV_wrap_clamp            = 0x0002,
    WV_wrap_repeat           = 0x0004,
    WV_wrap_mirror           = 0x0008,
    WV_wrap_mirror_once      = 0x0010,
    WV_wrap_border_color     = 0x0020,
    WW_wrap_unspecified      = 0x0040,
    WW_wrap_clamp            = 0x0080,
    WW_wrap_repeat           = 0x0100,
    WW_wrap_mirror           = 0x0200,
    WW_wrap_mirror_once      = 0x0400,
    WW_wrap_border_color     = 0x0800,
  };

  enum FilterOptions {
    MIN_unspecified             = 0x0001,
    MIN_nearest                 = 0x0002,
    MIN_linear                  = 0x0004,
    MIN_nearest_mipmap_nearest  = 0x0008,
    MIN_linear_mipmap_nearest   = 0x0010,
    MIN_nearest_mipmap_linear   = 0x0020,
    MIN_linear_mipmap_linear    = 0x0040,
    MAG_unspecified             = 0x0080,
    MAG_nearest                 = 0x0100,
    MAG_linear                  = 0x0200,
  };

  
  LoaderOptions(int flags = LF_search | LF_report_errors);
  constexpr LoaderOptions(int flags, int texture_flags);

  INLINE void set_flags(int flags);
  INLINE int get_flags() const;
  MAKE_PROPERTY(flags, get_flags, set_flags);
  INLINE void set_texture_flags(int flags);
  INLINE int get_texture_flags() const;
  INLINE void set_texture_format(int options);
  INLINE int get_texture_format() const;
  INLINE void set_texture_format_compress(int options);
  INLINE int get_texture_format_compress() const;
  INLINE void set_wrap_options_ou(int options);
  INLINE int get_wrap_options_ou() const;
  INLINE void set_wrap_options_vw(int options);
  INLINE int get_wrap_options_vw() const;
  INLINE void set_filter_options(int options);
  INLINE int get_filter_options() const;
  
  INLINE void set_texture_num_views(int num_views);
  INLINE int get_texture_num_views() const;
  MAKE_PROPERTY(texture_flags, get_texture_flags, set_texture_flags);
  MAKE_PROPERTY(texture_num_views, get_texture_num_views,
                                   set_texture_num_views);

  INLINE void set_auto_texture_scale(AutoTextureScale scale);
  INLINE AutoTextureScale get_auto_texture_scale() const;
  MAKE_PROPERTY(auto_texture_scale, get_auto_texture_scale,
                                    set_auto_texture_scale);

  void output(std::ostream &out) const;

private:
  void write_flag(std::ostream &out, std::string &sep,
                  const std::string &flag_name, int flag) const;
  void write_texture_flag(std::ostream &out, std::string &sep,
                          const std::string &flag_name, int flag) const;
  int _flags;
  int _texture_flags;
  int _texture_format;
  int _texture_format_compress;
  int _wrap_options_ou;
  int _wrap_options_vw;
  int _filter_options;
  int _texture_num_views;
  AutoTextureScale _auto_texture_scale;
};

INLINE std::ostream &operator << (std::ostream &out, const LoaderOptions &opts) {
  opts.output(out);
  return out;
}

#include "loaderOptions.I"

#endif
