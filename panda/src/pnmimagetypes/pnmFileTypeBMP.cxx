// Filename: pnmFileTypeBMP.cxx
// Created by:  drose (19Jun00)
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

#include "pnmFileTypeBMP.h"

#ifdef HAVE_BMP

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

static const char * const extensions_bmp[] = {
  "bmp"
};
static const int num_extensions_bmp = sizeof(extensions_bmp) / sizeof(const char *);

TypeHandle PNMFileTypeBMP::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeBMP::
PNMFileTypeBMP() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypeBMP::
get_name() const {
  return "Windows BMP";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypeBMP::
get_num_extensions() const {
  return num_extensions_bmp;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypeBMP::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_bmp, string());
  return extensions_bmp[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypeBMP::
get_suggested_extension() const {
  return "bmp";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeBMP::
has_magic_number() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeBMP::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);
  return (magic_number.substr(0, 2) == "BM");
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypeBMP::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypeBMP::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void PNMFileTypeBMP::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeBMP);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeBMP::make_PNMFileTypeBMP
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
TypedWritable *PNMFileTypeBMP::
make_PNMFileTypeBMP(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_BMP
