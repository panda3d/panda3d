// Filename: imageBuffer.cxx
// Created by:  mike (09Jan97)
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

  if (manager->get_file_minor_ver() >= 3) {
    set_alpha_name(scan.get_string());
  } else {
    clear_alpha_name();
  }
}
