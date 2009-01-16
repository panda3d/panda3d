// Filename: loaderOptions.cxx
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

#include "loaderOptions.h"
#include "config_util.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LoaderOptions::
LoaderOptions(int flags) : 
  _flags(flags), _texture_flags(0)  
{
  if (preload_textures) {
    _texture_flags |= TF_preload;
  }
  if (preload_simple_textures) {
    _texture_flags |= TF_preload_simple;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void LoaderOptions::
output(ostream &out) const {
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
  if (sep.empty()) {
    out << "0";
  }

  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::write_flag
//       Access: Private
//  Description: Used to implement output().
////////////////////////////////////////////////////////////////////
void LoaderOptions::
write_flag(ostream &out, string &sep, 
           const string &flag_name, int flag) const {
  if ((_flags & flag) == flag) {
    out << sep << flag_name;
    sep = " | ";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderOptions::write_texture_flag
//       Access: Private
//  Description: Used to implement output().
////////////////////////////////////////////////////////////////////
void LoaderOptions::
write_texture_flag(ostream &out, string &sep, 
                   const string &flag_name, int flag) const {
  if ((_texture_flags & flag) == flag) {
    out << sep << flag_name;
    sep = " | ";
  }
}
