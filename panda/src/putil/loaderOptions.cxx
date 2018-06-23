/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file loaderOptions.cxx
 * @author drose
 * @date 2005-10-05
 */

#include "loaderOptions.h"
#include "config_putil.h"
#include "indent.h"

using std::string;

/**
 *
 */
LoaderOptions::
LoaderOptions(int flags) :
  _flags(flags),
  _texture_flags(0),
  _texture_num_views(0),
  _auto_texture_scale(ATS_unspecified)
{
  // Shadowing the variables in config_putil for static init ordering issues.
  static ConfigVariableBool *preload_textures;
  static ConfigVariableBool *preload_simple_textures;
  static ConfigVariableBool *compressed_textures;
  if (preload_textures == nullptr) {
    preload_textures = new ConfigVariableBool("preload-textures", true);
  }
  if (preload_simple_textures == nullptr) {
    preload_simple_textures = new ConfigVariableBool("preload-simple-textures", false);
  }
  if (compressed_textures == nullptr) {
    compressed_textures = new ConfigVariableBool("compressed-textures", false);
  }

  if (*preload_textures) {
    _texture_flags |= TF_preload;
  }
  if (*preload_simple_textures) {
    _texture_flags |= TF_preload_simple;
  }
  if (*compressed_textures) {
    _texture_flags |= TF_allow_compression;
  }
}

/**
 *
 */
void LoaderOptions::
output(std::ostream &out) const {
  out << "LoaderOptions(";

  string sep = "";
  write_flag(out, sep, "LF_search", LF_search);
  write_flag(out, sep, "LF_report_errors", LF_report_errors);
  if ((_flags & LF_convert_anim) == LF_convert_anim) {
    write_flag(out, sep, "LF_convert_anim", LF_convert_anim);
  } else {
    write_flag(out, sep, "LF_convert_skeleton", LF_convert_skeleton);
    write_flag(out, sep, "LF_convert_channels", LF_convert_channels);
  }
  if ((_flags & LF_no_cache) == LF_no_cache) {
    write_flag(out, sep, "LF_no_cache", LF_no_cache);
  } else {
    write_flag(out, sep, "LF_no_disk_cache", LF_no_disk_cache);
    write_flag(out, sep, "LF_no_ram_cache", LF_no_ram_cache);
  }
  write_flag(out, sep, "LF_allow_instance", LF_allow_instance);
  if (sep.empty()) {
    out << "0";
  }

  out << ", ";

  sep = "";
  write_texture_flag(out, sep, "TF_preload", TF_preload);
  write_texture_flag(out, sep, "TF_preload_simple", TF_preload_simple);
  write_texture_flag(out, sep, "TF_allow_1d", TF_allow_1d);
  write_texture_flag(out, sep, "TF_generate_mipmaps", TF_generate_mipmaps);
  write_texture_flag(out, sep, "TF_allow_compression", TF_allow_compression);
  if (sep.empty()) {
    out << "0";
  }

  if (_auto_texture_scale != ATS_unspecified) {
    out << ", ATS_" << _auto_texture_scale;
  }

  out << ")";
}

/**
 * Used to implement output().
 */
void LoaderOptions::
write_flag(std::ostream &out, string &sep,
           const string &flag_name, int flag) const {
  if ((_flags & flag) == flag) {
    out << sep << flag_name;
    sep = " | ";
  }
}

/**
 * Used to implement output().
 */
void LoaderOptions::
write_texture_flag(std::ostream &out, string &sep,
                   const string &flag_name, int flag) const {
  if ((_texture_flags & flag) == flag) {
    out << sep << flag_name;
    sep = " | ";
  }
}
