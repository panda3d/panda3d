// Filename: destTextureImage.cxx
// Created by:  drose (05Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "destTextureImage.h"
#include "sourceTextureImage.h"
#include "texturePlacement.h"
#include "textureImage.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle DestTextureImage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::Default Constructor
//       Access: Private
//  Description: The default constructor is only for the convenience
//               of the Bam reader.
////////////////////////////////////////////////////////////////////
DestTextureImage::
DestTextureImage() {
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DestTextureImage::
DestTextureImage(TexturePlacement *placement) {
  TextureImage *texture = placement->get_texture();
  _properties = texture->get_properties();
  _size_known = true;
  _x_size = texture->get_x_size();
  _y_size = texture->get_y_size();

  set_filename(placement->get_group(), texture->get_name());
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::copy
//       Access: Public
//  Description: Unconditionally copies the source texture into the
//               appropriate filename.
////////////////////////////////////////////////////////////////////
void DestTextureImage::
copy(TextureImage *texture) {
  write(texture->get_dest_image());
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::copy_if_stale
//       Access: Public
//  Description: Copies the source texture into the appropriate
//               filename only if the indicated old reference, which
//               represents the way it was last copied, is now
//               out-of-date.
////////////////////////////////////////////////////////////////////
void DestTextureImage::
copy_if_stale(const DestTextureImage *other, TextureImage *texture) {
  if (other->get_x_size() != get_x_size() ||
      other->get_y_size() != get_y_size() ||
      other->get_num_channels() != get_num_channels()) {
    copy(texture);

  } else {
    // Also check the timestamps.
    SourceTextureImage *source = texture->get_preferred_source();

    if (source != (SourceTextureImage *)NULL &&
	source->get_filename().compare_timestamps(get_filename()) > 0) {
      copy(texture);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void DestTextureImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_DestTextureImage);
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void DestTextureImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::make_DestTextureImage
//       Access: Protected
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
////////////////////////////////////////////////////////////////////
TypedWriteable* DestTextureImage::
make_DestTextureImage(const FactoryParams &params) {
  DestTextureImage *me = new DestTextureImage;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: DestTextureImage::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void DestTextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);
}
