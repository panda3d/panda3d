// Filename: imageBuffer.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "imageBuffer.h"
#include "config_gobj.h"
#include "config_util.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"

TypeHandle ImageBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ImageBuffer::
ImageBuffer() {
  _primary_file_num_channels = 0;
  _alpha_file_channel = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
ImageBuffer::
~ImageBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ImageBuffer::
write_datagram(BamWriter *, Datagram &me)
{
  Filename filename = get_filename();
  Filename alpha_filename = get_alpha_filename();

  switch (bam_texture_mode) {
  case BTM_unchanged:
    break;

  case BTM_fullpath:
    filename = get_fullpath();
    alpha_filename = get_alpha_fullpath();
    break;

  case BTM_relative:
    filename = get_fullpath();
    alpha_filename = get_alpha_fullpath();
    filename.find_on_searchpath(get_texture_path()) ||
      filename.find_on_searchpath(get_model_path());
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << get_filename() << " found as " << filename << "\n";
    }
    alpha_filename.find_on_searchpath(get_texture_path()) ||
      alpha_filename.find_on_searchpath(get_model_path());
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Alpha image " << get_alpha_filename() << " found as " << alpha_filename << "\n";
    }
    break;

  case BTM_basename:
    filename = filename.get_basename();
    alpha_filename = alpha_filename.get_basename();
    break;

  default:
    gobj_cat.error()
      << "Unsupported bam-texture-mode: " << (int)bam_texture_mode << "\n";
  }

  me.add_string(get_name());
  me.add_string(filename);
  me.add_string(alpha_filename);
  me.add_uint8(_primary_file_num_channels);
  me.add_uint8(_alpha_file_channel);
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void ImageBuffer::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_name(scan.get_string());
  set_filename(scan.get_string());
  set_alpha_filename(scan.get_string());

  if (manager->get_file_minor_ver() < 3) {
    _primary_file_num_channels = 0;
    _alpha_file_channel = 0;
  } else {
    _primary_file_num_channels = scan.get_uint8();
    _alpha_file_channel = scan.get_uint8();
  }
}
