// Filename: imageFile.cxx
// Created by:  drose (29Nov00)
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

#include "imageFile.h"
#include "palettizer.h"
#include "filenameUnifier.h"
#include "paletteGroup.h"

#include <pnmImage.h>
#include <pnmFileType.h>
#include <eggTexture.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle ImageFile::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ImageFile::
ImageFile() {
  _size_known = false;
  _x_size = 0;
  _y_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::make_shadow_image
//       Access: Public
//  Description: Sets up the ImageFile as a "shadow image" of a
//               particular PaletteImage.  This is a temporary
//               ImageFile that's used to read and write the shadow
//               palette image, which is used to keep a working copy
//               of the palette.
////////////////////////////////////////////////////////////////////
void ImageFile::
make_shadow_image(const string &basename) {
  _properties._color_type = pal->_shadow_color_type;
  _properties._alpha_type = pal->_shadow_alpha_type;

  set_filename(pal->_shadow_dirname, basename);
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::is_size_known
//       Access: Public
//  Description: Returns true if the size of the image file is known,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageFile::
is_size_known() const {
  return _size_known;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::get_x_size
//       Access: Public
//  Description: Returns the size of the image file in pixels in the X
//               direction.  It is an error to call this unless
//               is_size_known() returns true.
////////////////////////////////////////////////////////////////////
int ImageFile::
get_x_size() const {
  nassertr(is_size_known(), 0);
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::get_y_size
//       Access: Public
//  Description: Returns the size of the image file in pixels in the Y
//               direction.  It is an error to call this unless
//               is_size_known() returns true.
////////////////////////////////////////////////////////////////////
int ImageFile::
get_y_size() const {
  nassertr(is_size_known(), 0);
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::has_num_channels
//       Access: Public
//  Description: Returns true if the number of channels in the image
//               is known, false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageFile::
has_num_channels() const {
  return _properties.has_num_channels();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::get_num_channels
//       Access: Public
//  Description: Returns the number of channels of the image.  It is
//               an error to call this unless has_num_channels()
//               returns true.
////////////////////////////////////////////////////////////////////
int ImageFile::
get_num_channels() const {
  return _properties.get_num_channels();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::get_properties
//       Access: Public
//  Description: Returns the grouping properties of the image.
////////////////////////////////////////////////////////////////////
const TextureProperties &ImageFile::
get_properties() const {
  return _properties;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::clear_basic_properties
//       Access: Public
//  Description: Resets the properties to a neutral state, for
//               instance in preparation for calling
//               update_properties() with all the known contributing
//               properties.
////////////////////////////////////////////////////////////////////
void ImageFile::
clear_basic_properties() {
  _properties.clear_basic();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::update_properties
//       Access: Public
//  Description: If the indicate TextureProperties structure is more
//               specific than this one, updates this one.
////////////////////////////////////////////////////////////////////
void ImageFile::
update_properties(const TextureProperties &properties) {
  _properties.update_properties(properties);
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::set_filename
//       Access: Public
//  Description: Sets the filename, and if applicable, the
//               alpha_filename, from the indicated basename.  The
//               extension appropriate to the image file type
//               specified in _color_type (and _alpha_type) is
//               automatically applied.
////////////////////////////////////////////////////////////////////
void ImageFile::
set_filename(PaletteGroup *group, const string &basename) {
  // Synthesize the directory name based on the map_dirname set to the
  // palettizer, and the group's dirname.
  string dirname;
  string::iterator pi;
  pi = pal->_map_dirname.begin();
  while (pi != pal->_map_dirname.end()) {
    if (*pi == '%') {
      ++pi;
      switch (*pi) {
      case '%':
        dirname += '%';
        break;

      case 'g':
        if (group != (PaletteGroup *)NULL) {
          dirname += group->get_dirname();
        }
        break;
      }
    } else {
      dirname += *pi;
    }
    ++pi;
  }

  set_filename(dirname, basename);
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::set_filename
//       Access: Public
//  Description: Sets the filename, and if applicable, the
//               alpha_filename, from the indicated basename.  The
//               extension appropriate to the image file type
//               specified in _color_type (and _alpha_type) is
//               automatically applied.
////////////////////////////////////////////////////////////////////
void ImageFile::
set_filename(const string &dirname, const string &basename) {
  _filename = Filename(dirname, basename);

  // Since we use set_extension() here, if the file already contains a
  // filename extension it will be lost.

  // It is particularly important to note that a single embedded dot
  // will appear to begin a filename extension, so if the filename
  // does *not* contain an extension, but does contain an embedded
  // dot, the filename will be truncated at that dot.  It is therefore
  // important that the supplied basename always contains either an
  // extension or a terminating dot.

  if (_properties._color_type != (PNMFileType *)NULL) {
    _filename.set_extension
      (_properties._color_type->get_suggested_extension());
  }

  if (_properties._alpha_type != (PNMFileType *)NULL) {
    _alpha_filename = _filename.get_fullpath_wo_extension() + "_a.";
    _alpha_filename.set_extension
      (_properties._alpha_type->get_suggested_extension());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::get_filename
//       Access: Public
//  Description: Returns the primary filename of the image file.
////////////////////////////////////////////////////////////////////
const Filename &ImageFile::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::get_alpha_filename
//       Access: Public
//  Description: Returns the alpha filename of the image file.  This
//               is the name of the file that contains the alpha
//               channel, if it is stored in a separate file, or the
//               empty string if it is not.
////////////////////////////////////////////////////////////////////
const Filename &ImageFile::
get_alpha_filename() const {
  return _alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::exists
//       Access: Public
//  Description: Returns true if the file or files named by the image
//               file exist, false otherwise.
////////////////////////////////////////////////////////////////////
bool ImageFile::
exists() const {
  if (!_filename.exists()) {
    return false;
  }
  if (_properties._alpha_type != (PNMFileType *)NULL &&
      _properties.uses_alpha() &&
      !_alpha_filename.empty()) {
    if (!_alpha_filename.exists()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::read
//       Access: Public
//  Description: Reads in the image (or images, if the alpha_filename
//               is separate) and stores it in the indicated PNMImage.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool ImageFile::
read(PNMImage &image) const {
  nassertr(!_filename.empty(), false);

  image.set_type(_properties._color_type);
  nout << "Reading " << FilenameUnifier::make_user_filename(_filename) << "\n";
  if (!image.read(_filename)) {
    nout << "Unable to read.\n";
    return false;
  }

  if (!_alpha_filename.empty() && _alpha_filename.exists()) {
    // Read in a separate color image and an alpha channel image.
    PNMImage alpha_image;
    alpha_image.set_type(_properties._alpha_type);
    nout << "Reading " << FilenameUnifier::make_user_filename(_alpha_filename) << "\n";
    if (!alpha_image.read(_alpha_filename)) {
      nout << "Unable to read.\n";
      return false;
    }
    if (image.get_x_size() != alpha_image.get_x_size() ||
        image.get_y_size() != alpha_image.get_y_size()) {
      return false;
    }

    image.add_alpha();
    if (alpha_image.has_alpha()) {
      for (int y = 0; y < image.get_y_size(); y++) {
        for (int x = 0; x < image.get_x_size(); x++) {
          image.set_alpha(x, y, alpha_image.get_alpha(x, y));
        }
      }
    } else {
      for (int y = 0; y < image.get_y_size(); y++) {
        for (int x = 0; x < image.get_x_size(); x++) {
          image.set_alpha(x, y, alpha_image.get_gray(x, y));
        }
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::write
//       Access: Public
//  Description: Writes out the image in the indicated PNMImage to the
//               _filename and/or _alpha_filename.  Returns true on
//               success, false on failure.
////////////////////////////////////////////////////////////////////
bool ImageFile::
write(const PNMImage &image) const {
  nassertr(!_filename.empty(), false);

  if (!image.has_alpha() ||
      _properties._alpha_type == (PNMFileType *)NULL) {
    if (!_alpha_filename.empty() && _alpha_filename.exists()) {
      nout << "Deleting " << FilenameUnifier::make_user_filename(_alpha_filename) << "\n";
      _alpha_filename.unlink();
    }
    nout << "Writing " << FilenameUnifier::make_user_filename(_filename) << "\n";
    _filename.make_dir();
    if (!image.write(_filename, _properties._color_type)) {
      nout << "Unable to write.\n";
      return false;
    }
    return true;
  }

  // Write out a separate color image and an alpha channel image.
  PNMImage alpha_image(image.get_x_size(), image.get_y_size(), 1,
                       image.get_maxval());
  for (int y = 0; y < image.get_y_size(); y++) {
    for (int x = 0; x < image.get_x_size(); x++) {
      alpha_image.set_gray_val(x, y, image.get_alpha_val(x, y));
    }
  }

  PNMImage image_copy(image);
  image_copy.remove_alpha();
  nout << "Writing " << FilenameUnifier::make_user_filename(_filename) << "\n";
  _filename.make_dir();
  if (!image_copy.write(_filename, _properties._color_type)) {
    nout << "Unable to write.\n";
    return false;
  }

  nout << "Writing " << FilenameUnifier::make_user_filename(_alpha_filename) << "\n";
  _alpha_filename.make_dir();
  if (!alpha_image.write(_alpha_filename, _properties._alpha_type)) {
    nout << "Unable to write.\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::unlink
//       Access: Public
//  Description: Deletes the image file or files.
////////////////////////////////////////////////////////////////////
void ImageFile::
unlink() {
  if (!_filename.empty() && _filename.exists()) {
    nout << "Deleting " << FilenameUnifier::make_user_filename(_filename) << "\n";
    _filename.unlink();
  }
  if (!_alpha_filename.empty() && _alpha_filename.exists()) {
    nout << "Deleting " << FilenameUnifier::make_user_filename(_alpha_filename) << "\n";
    _alpha_filename.unlink();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::update_egg_tex
//       Access: Public
//  Description: Sets the indicated EggTexture to refer to this file.
////////////////////////////////////////////////////////////////////
void ImageFile::
update_egg_tex(EggTexture *egg_tex) const {
  nassertv(egg_tex != (EggTexture *)NULL);

  egg_tex->set_filename(FilenameUnifier::make_egg_filename(_filename));

  if (_properties.uses_alpha() &&
      !_alpha_filename.empty()) {
    egg_tex->set_alpha_filename(FilenameUnifier::make_egg_filename(_alpha_filename));
  } else {
    egg_tex->clear_alpha_filename();
  }

  _properties.update_egg_tex(egg_tex);
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::output_filename
//       Access: Public
//  Description: Writes the filename (or pair of filenames) to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
void ImageFile::
output_filename(ostream &out) const {
  out << FilenameUnifier::make_user_filename(_filename); 
  if (_properties.uses_alpha() && !_alpha_filename.empty()) {
    out << " " << FilenameUnifier::make_user_filename(_alpha_filename);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void ImageFile::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  _properties.write_datagram(writer, datagram);
  datagram.add_string(FilenameUnifier::make_bam_filename(_filename));
  datagram.add_string(FilenameUnifier::make_bam_filename(_alpha_filename));
  datagram.add_bool(_size_known);
  datagram.add_int32(_x_size);
  datagram.add_int32(_y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::complete_pointers
//       Access: Public, Virtual
//  Description: Called after the object is otherwise completely read
//               from a Bam file, this function's job is to store the
//               pointers that were retrieved from the Bam file for
//               each pointer object written.  The return value is the
//               number of pointers processed from the list.
////////////////////////////////////////////////////////////////////
int ImageFile::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  pi += _properties.complete_pointers(p_list + pi, manager);

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageFile::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void ImageFile::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  _properties.fillin(scan, manager);
  _filename = FilenameUnifier::get_bam_filename(scan.get_string());
  _alpha_filename = FilenameUnifier::get_bam_filename(scan.get_string());
  _size_known = scan.get_bool();
  _x_size = scan.get_int32();
  _y_size = scan.get_int32();
}
