// Filename: sourceTextureImage.cxx
// Created by:  drose (29Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "sourceTextureImage.h"
#include "textureImage.h"

#include <pnmImageHeader.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle SourceTextureImage::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::Default Constructor
//       Access: Private
//  Description: The default constructor is only for the convenience
//               of the Bam reader.
////////////////////////////////////////////////////////////////////
SourceTextureImage::
SourceTextureImage() {
  _texture = (TextureImage *)NULL;

  _read_header = false;
  _successfully_read_header = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SourceTextureImage::
SourceTextureImage(TextureImage *texture, const Filename &filename,
		   const Filename &alpha_filename) :
  _texture(texture)
{
  _filename = filename;
  _alpha_filename = alpha_filename;
  _read_header = false;
  _successfully_read_header = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::get_texture
//       Access: Public
//  Description: Returns the particular texture that this image is one
//               of the sources for.
////////////////////////////////////////////////////////////////////
TextureImage *SourceTextureImage::
get_texture() const {
  return _texture;
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::get_size
//       Access: Public
//  Description: Determines the size of the SourceTextureImage, if it
//               is not already known.  Returns true if the size was
//               successfully determined (or if was already known), or
//               false if the size could not be determined (for
//               instance, because the image file is missing).  After
//               this call returns true, get_x_size() etc. may be
//               safely called to return the size.
////////////////////////////////////////////////////////////////////
bool SourceTextureImage::
get_size() {
  if (!_size_known) {
    return read_header();
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::read_header
//       Access: Public
//  Description: Reads the actual image header to determine the image
//               properties, like its size.  Returns true if the image
//               header is successfully read (or if has previously
//               been successfully read this session), false
//               otherwise.  After this call returns true,
//               get_x_size() etc. may be safely called to return the
//               newly determined size.
////////////////////////////////////////////////////////////////////
bool SourceTextureImage::
read_header() {
  if (_read_header) {
    return _successfully_read_header;
  }

  _read_header = true;
  _successfully_read_header = false;

  PNMImageHeader header;
  if (!header.read_header(_filename)) {
    nout << "Warning: cannot read texture " << _filename << "\n";
    return false;
  }

  _x_size = header.get_x_size();
  _y_size = header.get_y_size();
  _properties._got_num_channels = true;
  _properties._num_channels = header.get_num_channels();

  if (!_alpha_filename.empty()) {
    // Assume if we have an alpha filename, that we have an additional
    // alpha channel.
    if (_properties._num_channels == 1 || _properties._num_channels == 3) {
      _properties._num_channels++;
    }
  }

  _size_known = true;
  _successfully_read_header = true;

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void SourceTextureImage::
output(ostream &out) const {
  out << _filename;
  if (!_alpha_filename.empty()) {
    out << " (" << _alpha_filename << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void SourceTextureImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_SourceTextureImage);
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
////////////////////////////////////////////////////////////////////
void SourceTextureImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);
  writer->write_pointer(datagram, _texture);

  // We don't store _read_header or _successfully_read_header in the
  // Bam file; these are transitory and we need to reread the image
  // header for each session (in case the image files change between
  // sessions).
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::complete_pointers
//       Access: Public, Virtual
//  Description: Called after the object is otherwise completely read
//               from a Bam file, this function's job is to store the
//               pointers that were retrieved from the Bam file for
//               each pointer object written.  The return value is the
//               number of pointers processed from the list.
////////////////////////////////////////////////////////////////////
int SourceTextureImage::
complete_pointers(vector_typedWriteable &plist, BamReader *manager) {
  nassertr(plist.size() >= 1, 0);

  DCAST_INTO_R(_texture, plist[0], 0);
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::make_SourceTextureImage
//       Access: Protected
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
////////////////////////////////////////////////////////////////////
TypedWriteable* SourceTextureImage::
make_SourceTextureImage(const FactoryParams &params) {
  SourceTextureImage *me = new SourceTextureImage;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: SourceTextureImage::fillin
//       Access: Protected
//  Description: Reads the binary data from the given datagram
//               iterator, which was written by a previous call to
//               write_datagram().
////////////////////////////////////////////////////////////////////
void SourceTextureImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);
  manager->read_pointer(scan, this); // _texture
}
