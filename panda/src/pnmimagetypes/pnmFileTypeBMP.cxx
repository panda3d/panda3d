// Filename: pnmFileTypeBMP.cxx
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeBMP.h"
#include "config_pnmimagetypes.h"

static const char * const extensions[] = {
  "bmp"
};
static const int num_extensions = sizeof(extensions) / sizeof(const char *);

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
  return num_extensions;
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
  nassertr(n >= 0 && n < num_extensions, string());
  return extensions[n];
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
make_reader(FILE *file, bool owns_file, const string &magic_number) {
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
make_writer(FILE *file, bool owns_file) {
  return new Writer(this, file, owns_file);
}

