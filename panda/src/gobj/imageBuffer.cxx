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

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "imageBuffer.h"
#include "config_gobj.h"
#include "config_util.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ImageBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ImageBuffer::
write_datagram(BamWriter *, Datagram &me)
{
  Filename filename = get_name();
  Filename alpha_filename = get_alpha_name();

  switch (bam_texture_mode) {
  case BTM_fullpath:
    break;

  case BTM_relative:
    filename.find_on_searchpath(get_texture_path());
    filename.find_on_searchpath(get_model_path());
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Texture file " << get_name() << " found as " << filename << "\n";
    }
    alpha_filename.find_on_searchpath(get_texture_path());
    alpha_filename.find_on_searchpath(get_model_path());
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "Alpha image " << get_alpha_name() << " found as " << alpha_filename << "\n";
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

  me.add_string(filename);
  me.add_string(alpha_filename);
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
  set_alpha_name(scan.get_string());
}
