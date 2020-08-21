/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sourceTextureImage.cxx
 * @author drose
 * @date 2000-11-29
 */

#include "sourceTextureImage.h"
#include "textureImage.h"
#include "filenameUnifier.h"

#include "pnmImageHeader.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle SourceTextureImage::_type_handle;

/**
 * The default constructor is only for the convenience of the Bam reader.
 */
SourceTextureImage::
SourceTextureImage() {
  _texture = nullptr;

  _egg_count = 0;
  _read_header = false;
  _successfully_read_header = false;
}

/**
 *
 */
SourceTextureImage::
SourceTextureImage(TextureImage *texture, const Filename &filename,
                   const Filename &alpha_filename, int alpha_file_channel) :
  _texture(texture)
{
  _filename = filename;
  _alpha_filename = alpha_filename;
  _alpha_file_channel = alpha_file_channel;
  _egg_count = 0;
  _read_header = false;
  _successfully_read_header = false;
}

/**
 * Returns the particular texture that this image is one of the sources for.
 */
TextureImage *SourceTextureImage::
get_texture() const {
  return _texture;
}

/**
 * Increments by one the number of egg files that are known to reference this
 * SourceTextureImage.
 */
void SourceTextureImage::
increment_egg_count() {
  _egg_count++;
}

/**
 * Returns the number of egg files that share this SourceTextureImage.
 */
int SourceTextureImage::
get_egg_count() const {
  return _egg_count;
}

/**
 * Determines the size of the SourceTextureImage, if it is not already known.
 * Returns true if the size was successfully determined (or if was already
 * known), or false if the size could not be determined (for instance, because
 * the image file is missing).  After this call returns true, get_x_size()
 * etc.  may be safely called to return the size.
 */
bool SourceTextureImage::
get_size() {
  if (!_size_known) {
    return read_header();
  }
  return true;
}

/**
 * Reads the actual image header to determine the image properties, like its
 * size.  Returns true if the image header is successfully read (or if has
 * previously been successfully read this session), false otherwise.  After
 * this call returns true, get_x_size() etc.  may be safely called to return
 * the newly determined size.
 */
bool SourceTextureImage::
read_header() {
  if (_read_header) {
    return _successfully_read_header;
  }

  _read_header = true;
  _successfully_read_header = false;

  PNMImageHeader header;
  if (!header.read_header(_filename)) {
    nout << "Warning: cannot read texture "
         << FilenameUnifier::make_user_filename(_filename) << "\n";
    return false;
  }

  set_header(header);

  return true;
}

/**
 * Sets the header information associated with this image, as if it were
 * loaded from the disk.
 */
void SourceTextureImage::
set_header(const PNMImageHeader &header) {
  _x_size = header.get_x_size();
  _y_size = header.get_y_size();
  int num_channels = header.get_num_channels();

  if (!_alpha_filename.empty() && _alpha_filename.exists()) {
    // Assume if we have an alpha filename, that we have an additional alpha
    // channel.
    if (num_channels == 1 || num_channels == 3) {
      num_channels++;
    }
  }
  _properties.set_num_channels(num_channels);

  _size_known = true;
  _successfully_read_header = true;
}


/**
 * Registers the current object as something that can be read from a Bam file.
 */
void SourceTextureImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_SourceTextureImage);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void SourceTextureImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);
  writer->write_pointer(datagram, _texture);

  // We don't store _egg_count; instead, we count these up again each session.

  // We don't store _read_header or _successfully_read_header in the Bam file;
  // these are transitory and we need to reread the image header for each
  // session (in case the image files change between sessions).
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int SourceTextureImage::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = ImageFile::complete_pointers(p_list, manager);

  DCAST_INTO_R(_texture, p_list[pi++], pi);
  return pi;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable *SourceTextureImage::
make_SourceTextureImage(const FactoryParams &params) {
  SourceTextureImage *me = new SourceTextureImage;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void SourceTextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);
  manager->read_pointer(scan); // _texture
}
