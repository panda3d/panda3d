// Filename: loaderOptions.h
// Created by:  drose (05Oct05)
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

#ifndef LOADEROPTIONS_H
#define LOADEROPTIONS_H

#include "pandabase.h"
#include "autoTextureScale.h"

////////////////////////////////////////////////////////////////////
//       Class : LoaderOptions
// Description : Specifies parameters that may be passed to the
//               loader.
////////////////////////////////////////////////////////////////////
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
  };

  LoaderOptions(int flags = LF_search | LF_report_errors);
  INLINE LoaderOptions(int flags, int texture_flags);
  INLINE LoaderOptions(const LoaderOptions &copy);
  INLINE void operator = (const LoaderOptions &copy);

  INLINE void set_flags(int flags);
  INLINE int get_flags() const;

  INLINE void set_texture_flags(int flags);
  INLINE int get_texture_flags() const;
  INLINE void set_texture_num_views(int num_views);
  INLINE int get_texture_num_views() const;

  INLINE void set_auto_texture_scale(AutoTextureScale scale);
  INLINE AutoTextureScale get_auto_texture_scale() const;

  void output(ostream &out) const;

private:
  void write_flag(ostream &out, string &sep, 
                  const string &flag_name, int flag) const;
  void write_texture_flag(ostream &out, string &sep, 
                          const string &flag_name, int flag) const;
  int _flags;
  int _texture_flags;
  int _texture_num_views;
  AutoTextureScale _auto_texture_scale;
};

INLINE ostream &operator << (ostream &out, const LoaderOptions &opts) {
  opts.output(out);
  return out;
}

#include "loaderOptions.I"

#endif
