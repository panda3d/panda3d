/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file destTextureImage.cxx
 * @author drose
 * @date 2000-12-05
 */

#include "destTextureImage.h"
#include "sourceTextureImage.h"
#include "texturePlacement.h"
#include "textureImage.h"
#include "palettizer.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle DestTextureImage::_type_handle;


/**
 * The default constructor is only for the convenience of the Bam reader.
 */
DestTextureImage::
DestTextureImage() {
}

/**
 *
 */
DestTextureImage::
DestTextureImage(TexturePlacement *placement) {
  TextureImage *texture = placement->get_texture();
  _properties = texture->get_properties();
  _size_known = texture->is_size_known();
  if (_size_known) {
    _x_size = texture->get_x_size();
    _y_size = texture->get_y_size();

    if (pal->_force_power_2) {
      _x_size = to_power_2(_x_size);
      _y_size = to_power_2(_y_size);
    } else {
      _x_size = std::max(_x_size, 1);
      _y_size = std::max(_y_size, 1);
    }
  }

  set_filename(placement->get_group(), texture->get_name());
}

/**
 * Unconditionally copies the source texture into the appropriate filename.
 */
void DestTextureImage::
copy(TextureImage *texture) {
  const PNMImage &source_image = texture->read_source_image();
  if (source_image.is_valid()) {
    PNMImage dest_image(_x_size, _y_size, texture->get_num_channels(),
                        source_image.get_maxval());
    dest_image.quick_filter_from(source_image);
    write(dest_image);

  } else {
    // Couldn't read the texture, so fill it with red.
    PNMImage dest_image(_x_size, _y_size, texture->get_num_channels());
    dest_image.fill(1.0, 0.0, 0.0);
    if (dest_image.has_alpha()) {
      dest_image.alpha_fill(1.0);
    }

    write(dest_image);
  }

  texture->release_source_image();
}

/**
 * Copies the source texture into the appropriate filename only if the
 * indicated old reference, which represents the way it was last copied, is
 * now out-of-date.
 */
void DestTextureImage::
copy_if_stale(const DestTextureImage *other, TextureImage *texture) {
  if (other->get_x_size() != get_x_size() ||
      other->get_y_size() != get_y_size() ||
      other->get_num_channels() != get_num_channels()) {
    copy(texture);

  } else {
    // Also check the timestamps.
    SourceTextureImage *source = texture->get_preferred_source();

    if (source != nullptr &&
        source->get_filename().compare_timestamps(get_filename()) > 0) {
      copy(texture);
    }
  }
}

/**
 * Returns the largest power of 2 less than or equal to value.
 */
int DestTextureImage::
to_power_2(int value) {
  int x = 1;
  while ((x << 1) <= value) {
    x = (x << 1);
  }
  return x;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void DestTextureImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_DestTextureImage);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void DestTextureImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable* DestTextureImage::
make_DestTextureImage(const FactoryParams &params) {
  DestTextureImage *me = new DestTextureImage;
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
void DestTextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);
}
