/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageFile.cxx
 * @author drose
 * @date 2000-11-29
 */

#include "imageFile.h"
#include "palettizer.h"
#include "filenameUnifier.h"
#include "paletteGroup.h"

#include "pnmImage.h"
#include "pnmFileType.h"
#include "eggTexture.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

using std::string;

TypeHandle ImageFile::_type_handle;

/**
 *
 */
ImageFile::
ImageFile() {
  _alpha_file_channel = 0;
  _size_known = false;
  _x_size = 0;
  _y_size = 0;
}

/**
 * Sets up the ImageFile as a "shadow image" of a particular PaletteImage.
 * This is a temporary ImageFile that's used to read and write the shadow
 * palette image, which is used to keep a working copy of the palette.
 *
 * Returns true if the filename changes from what it was previously, false
 * otherwise.
 */
bool ImageFile::
make_shadow_image(const string &basename) {
  bool any_changed = false;

  if (_properties._color_type != pal->_shadow_color_type ||
      _properties._alpha_type != pal->_shadow_alpha_type) {

    _properties._color_type = pal->_shadow_color_type;
    _properties._alpha_type = pal->_shadow_alpha_type;
    any_changed = true;
  }

  if (set_filename(pal->_shadow_dirname, basename)) {
    any_changed = true;
  }

  return any_changed;
}

/**
 * Returns true if the size of the image file is known, false otherwise.
 */
bool ImageFile::
is_size_known() const {
  return _size_known;
}

/**
 * Returns the size of the image file in pixels in the X direction.  It is an
 * error to call this unless is_size_known() returns true.
 */
int ImageFile::
get_x_size() const {
  nassertr(is_size_known(), 0);
  return _x_size;
}

/**
 * Returns the size of the image file in pixels in the Y direction.  It is an
 * error to call this unless is_size_known() returns true.
 */
int ImageFile::
get_y_size() const {
  nassertr(is_size_known(), 0);
  return _y_size;
}

/**
 * Returns true if the number of channels in the image is known, false
 * otherwise.
 */
bool ImageFile::
has_num_channels() const {
  return _properties.has_num_channels();
}

/**
 * Returns the number of channels of the image.  It is an error to call this
 * unless has_num_channels() returns true.
 */
int ImageFile::
get_num_channels() const {
  return _properties.get_num_channels();
}

/**
 * Returns the grouping properties of the image.
 */
const TextureProperties &ImageFile::
get_properties() const {
  return _properties;
}

/**
 * Resets the properties to a neutral state, for instance in preparation for
 * calling update_properties() with all the known contributing properties.
 */
void ImageFile::
clear_basic_properties() {
  _properties.clear_basic();
}

/**
 * If the indicate TextureProperties structure is more specific than this one,
 * updates this one.
 */
void ImageFile::
update_properties(const TextureProperties &properties) {
  _properties.update_properties(properties);
}

/**
 * Sets the filename, and if applicable, the alpha_filename, from the
 * indicated basename.  The extension appropriate to the image file type
 * specified in _color_type (and _alpha_type) is automatically applied.
 *
 * Returns true if the filename changes from what it was previously, false
 * otherwise.
 */
bool ImageFile::
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
        if (group != nullptr) {
          dirname += group->get_dirname();
        }
        break;
      }
    } else {
      dirname += *pi;
    }
    ++pi;
  }

  return set_filename(dirname, basename);
}

/**
 * Sets the filename, and if applicable, the alpha_filename, from the
 * indicated basename.  The extension appropriate to the image file type
 * specified in _color_type (and _alpha_type) is automatically applied.
 *
 * Returns true if the filename changes from what it was previously, false
 * otherwise.
 */
bool ImageFile::
set_filename(const string &dirname, const string &basename) {
  Filename orig_filename = _filename;
  Filename orig_alpha_filename = _alpha_filename;

  _filename = Filename(dirname, basename);
  _filename.standardize();

  // Since we use set_extension() here, if the file already contains a
  // filename extension it will be lost.

  // It is particularly important to note that a single embedded dot will
  // appear to begin a filename extension, so if the filename does *not*
  // contain an extension, but does contain an embedded dot, the filename will
  // be truncated at that dot.  It is therefore important that the supplied
  // basename always contains either an extension or a terminating dot.

  if (_properties._color_type != nullptr) {
    _filename.set_extension
      (_properties._color_type->get_suggested_extension());
  }

  if (_properties._alpha_type != nullptr) {
    _alpha_filename = _filename.get_fullpath_wo_extension() + "_a.";
    _alpha_filename.set_extension
      (_properties._alpha_type->get_suggested_extension());
  } else {
    _alpha_filename = Filename();
  }

  return (_filename != orig_filename ||
          _alpha_filename != orig_alpha_filename);
}

/**
 * Returns the primary filename of the image file.
 */
const Filename &ImageFile::
get_filename() const {
  return _filename;
}

/**
 * Returns the alpha filename of the image file.  This is the name of the file
 * that contains the alpha channel, if it is stored in a separate file, or the
 * empty string if it is not.
 */
const Filename &ImageFile::
get_alpha_filename() const {
  return _alpha_filename;
}

/**
 * Returns the particular channel number of the alpha image file from which
 * the alpha channel should be extracted.  This is normally 0 to represent the
 * grayscale combination of r, g, and b; or it may be a 1-based channel number
 * (for instance, 4 for the alpha channel of a 4-component image).
 */
int ImageFile::
get_alpha_file_channel() const {
  return _alpha_file_channel;
}


/**
 * Returns true if the file or files named by the image file exist, false
 * otherwise.
 */
bool ImageFile::
exists() const {
  if (!_filename.exists()) {
    return false;
  }
  if (_properties._alpha_type != nullptr &&
      _properties.uses_alpha() &&
      !_alpha_filename.empty()) {
    if (!_alpha_filename.exists()) {
      return false;
    }
  }

  return true;
}

/**
 * Reads in the image (or images, if the alpha_filename is separate) and
 * stores it in the indicated PNMImage.  Returns true on success, false on
 * failure.
 */
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

    if (_alpha_file_channel == 4 ||
        (_alpha_file_channel == 2 && alpha_image.get_num_channels() == 2)) {
      // Use the alpha channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_alpha(x, y));
        }
      }

    } else if (_alpha_file_channel >= 1 && _alpha_file_channel <= 3 &&
               alpha_image.get_num_channels() >= 3) {
      // Use the appropriate red, green, or blue channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_channel_val(x, y, _alpha_file_channel - 1));
        }
      }

    } else {
      // Use the grayscale channel.
      for (int x = 0; x < image.get_x_size(); x++) {
        for (int y = 0; y < image.get_y_size(); y++) {
          image.set_alpha(x, y, alpha_image.get_gray(x, y));
        }
      }
    }
  }

  return true;
}

/**
 * Writes out the image in the indicated PNMImage to the _filename and/or
 * _alpha_filename.  Returns true on success, false on failure.
 */
bool ImageFile::
write(const PNMImage &image) const {
  nassertr(!_filename.empty(), false);

  if (!image.has_alpha() ||
      _properties._alpha_type == nullptr) {
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

/**
 * Deletes the image file or files.
 */
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

/**
 * Sets the indicated EggTexture to refer to this file.
 */
void ImageFile::
update_egg_tex(EggTexture *egg_tex) const {
  nassertv(egg_tex != nullptr);

  egg_tex->set_filename(FilenameUnifier::make_egg_filename(_filename));

  if (_properties.uses_alpha() &&
      !_alpha_filename.empty()) {
    egg_tex->set_alpha_filename(FilenameUnifier::make_egg_filename(_alpha_filename));
    egg_tex->set_alpha_file_channel(_alpha_file_channel);
  } else {
    egg_tex->clear_alpha_filename();
    egg_tex->clear_alpha_file_channel();
  }

  _properties.update_egg_tex(egg_tex);
}

/**
 * Writes the filename (or pair of filenames) to the indicated output stream.
 */
void ImageFile::
output_filename(std::ostream &out) const {
  out << FilenameUnifier::make_user_filename(_filename);
  if (_properties.uses_alpha() && !_alpha_filename.empty()) {
    out << " " << FilenameUnifier::make_user_filename(_alpha_filename);
  }
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void ImageFile::
write_datagram(BamWriter *writer, Datagram &datagram) {
  TypedWritable::write_datagram(writer, datagram);
  _properties.write_datagram(writer, datagram);
  datagram.add_string(FilenameUnifier::make_bam_filename(_filename));
  datagram.add_string(FilenameUnifier::make_bam_filename(_alpha_filename));
  datagram.add_uint8(_alpha_file_channel);
  datagram.add_bool(_size_known);
  datagram.add_int32(_x_size);
  datagram.add_int32(_y_size);
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int ImageFile::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  pi += _properties.complete_pointers(p_list + pi, manager);

  return pi;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void ImageFile::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  _properties.fillin(scan, manager);
  _filename = FilenameUnifier::get_bam_filename(scan.get_string());
  _alpha_filename = FilenameUnifier::get_bam_filename(scan.get_string());
  if (Palettizer::_read_pi_version >= 10) {
    _alpha_file_channel = scan.get_uint8();
  } else {
    _alpha_file_channel = 0;
  }
  _size_known = scan.get_bool();
  _x_size = scan.get_int32();
  _y_size = scan.get_int32();
}
