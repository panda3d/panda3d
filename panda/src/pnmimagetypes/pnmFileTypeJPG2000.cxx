// Filename: pnmFileTypeJPG2000.cxx
// Created by:  mike (19Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeJPG2000.h"
#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

static const char * const extensions_jpg2000[] = {
  "JP2","JPC"
};
static const int num_extensions_jpg2000 = sizeof(extensions_jpg2000) / sizeof(const char *);

TypeHandle PNMFileTypeJPG2000::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileTypeJPG2000::
PNMFileTypeJPG2000() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::get_name
//       Access: Public, Virtual
//  Description: Returns a few words describing the file type.
////////////////////////////////////////////////////////////////////
string PNMFileTypeJPG2000::
get_name() const {
  return "JPEG 2000";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::get_num_extensions
//       Access: Public, Virtual
//  Description: Returns the number of different possible filename
//               extensions_jpg2000 associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileTypeJPG2000::
get_num_extensions() const {
  return num_extensions_jpg2000;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::get_extension
//       Access: Public, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileTypeJPG2000::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_jpg2000, string());
  return extensions_jpg2000[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::get_suggested_extension
//       Access: Public, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileTypeJPG2000::
get_suggested_extension() const {
  return "JP2";
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeJPG2000::
has_magic_number() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileTypeJPG2000::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);
  return ((char)magic_number[0] == (char)0xff &&
          (char)magic_number[1] == (char)0xd8);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileTypeJPG2000::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileTypeJPG2000::
make_writer(ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::register_with_read_factory
//       Access: Public, Static
//  Description: Registers the current object as something that can be
//               read from a Bam file.
////////////////////////////////////////////////////////////////////
void PNMFileTypeJPG2000::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeJPG2000);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeJPG2000::make_PNMFileTypeJPG2000
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
TypedWritable *PNMFileTypeJPG2000::
make_PNMFileTypeJPG2000(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_ptr()->get_type_by_handle(get_class_type());
}
