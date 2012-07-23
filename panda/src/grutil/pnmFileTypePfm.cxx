// Filename: pnmFileTypePfm.cxx
// Created by:  drose (04Apr98)
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

#include "pnmFileTypePfm.h"
#include "config_grutil.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

TypeHandle PNMFileTypePfm::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypePfm::
PNMFileTypePfm() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypePfm::
get_name() const {
  return "Portable Float Map";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypePfm::
get_num_extensions() const {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypePfm::
get_extension(int n) const {
  return "pfm";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypePfm::
get_suggested_extension() const {
  return "pfm";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePfm::
has_magic_number() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypePfm::
matches_magic_number(const string &magic_number) const {
  return (magic_number.size() >= 2) &&
    (magic_number.substr(0, 2) == "PF" ||
     magic_number.substr(0, 2) == "Pf" ||
     magic_number.substr(0, 2) == "pf");
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypePfm::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypePfm::
make_writer(ostream *file, bool owns_file) {
  return new Writer(this, file, owns_file);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::Reader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypePfm::Reader::
Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number) :
  PNMReader(type, file, owns_file)
{
  PfmFile pfm;
  if (!pfm.read(*file, Filename(), magic_number)) {
    _is_valid = false;
    return;
  }

  pfm.store(_image);
  PNMImageHeader::operator = (_image);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::Reader::read_data
//       Access: Public, Virtual
//  Description: Reads in an entire image all at once, storing it in
//               the pre-allocated _x_size * _y_size array and alpha
//               pointers.  (If the image type has no alpha channel,
//               alpha is ignored.)  Returns the number of rows
//               correctly read.
//
//               Derived classes need not override this if they
//               instead provide supports_read_row() and read_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMFileTypePfm::Reader::
read_data(xel *array, xelval *alpha) {
  if (!is_valid()) {
    return 0;
  }

  nassertr(_image.get_x_size() == get_x_size() &&
           _image.get_y_size() == get_y_size(), 0);

  memcpy(array, _image.get_array(), get_x_size() * get_y_size() * sizeof(xel));

  if (has_alpha()) {
    memcpy(alpha, _image.get_alpha_array(), get_x_size() * get_y_size() * sizeof(xelval));
  }

  return get_y_size();
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::Writer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypePfm::Writer::
Writer(PNMFileType *type, ostream *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::write_data
//       Access: Public, Virtual
//  Description: Writes out an entire image all at once, including the
//               header, based on the image data stored in the given
//               _x_size * _y_size array and alpha pointers.  (If the
//               image type has no alpha channel, alpha is ignored.)
//               Returns the number of rows correctly written.
//
//               It is the user's responsibility to fill in the header
//               data via calls to set_x_size(), set_num_channels(),
//               etc., or copy_header_from(), before calling
//               write_data().
//
//               It is important to delete the PNMWriter class after
//               successfully writing the data.  Failing to do this
//               may result in some data not getting flushed!
//
//               Derived classes need not override this if they
//               instead provide supports_streaming() and write_row(),
//               below.
////////////////////////////////////////////////////////////////////
int PNMFileTypePfm::Writer::
write_data(xel *array, xelval *alpha) {
  if (_x_size <= 0 || _y_size <= 0) {
    return 0;
  }

  PNMImage image;
  image.copy_header_from(*this);
  nassertr(image.get_x_size() == get_x_size() && 
           image.get_y_size() == get_y_size(), 0);
  memcpy(image.get_array(), array, get_x_size() * get_y_size() * sizeof(xel));
  if (has_alpha()) {
    memcpy(image.get_alpha_array(), alpha, get_x_size() * get_y_size() * sizeof(xelval));
  }

  PfmFile pfm;
  if (!pfm.load(image)) {
    return 0;
  }

  if (!pfm.write(*_file)) {
    return 0;
  }

  return get_y_size();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void PNMFileTypePfm::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypePfm);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypePfm::make_PNMFileTypePfm
//       Access: Protected, Static
//  Description: This method is called by the BamReader when an object
//               of this type is encountered in a Bam file; it should
//               allocate and return a new object with all the data
//               read.
//
//               In the case of the PNMFileType objects, since these
//               objects are all shared, we just pull the object from
//               the registry.
////////////////////////////////////////////////////////////////////
TypedWritable *PNMFileTypePfm::
make_PNMFileTypePfm(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}
