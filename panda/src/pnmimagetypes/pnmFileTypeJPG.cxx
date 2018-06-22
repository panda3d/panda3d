/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeJPG.cxx
 * @author mike
 * @date 2000-06-19
 */

#include "pnmFileTypeJPG.h"

#ifdef HAVE_JPEG

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

using std::string;

static const char *const extensions_jpg[] = {
  "jpg", "jpeg"
};
static const int num_extensions_jpg = sizeof(extensions_jpg) / sizeof(const char *);

TypeHandle PNMFileTypeJPG::_type_handle;

/**
 *
 */
PNMFileTypeJPG::
PNMFileTypeJPG() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeJPG::
get_name() const {
  return "JPEG";
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileTypeJPG::
get_num_extensions() const {
  return num_extensions_jpg;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeJPG::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_jpg, string());
  return extensions_jpg[n];
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileTypeJPG::
get_suggested_extension() const {
  return "jpg";
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileTypeJPG::
has_magic_number() const {
  return true;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileTypeJPG::
matches_magic_number(const string &magic_number) const {
  nassertr(magic_number.size() >= 2, false);
  return ((char)magic_number[0] == (char)0xff &&
          (char)magic_number[1] == (char)0xd8);
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileTypeJPG::
make_reader(std::istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileTypeJPG::
make_writer(std::ostream *file, bool owns_file) {
  init_pnm();
  return new Writer(this, file, owns_file);
}


/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeJPG::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeJPG);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 *
 * In the case of the PNMFileType objects, since these objects are all shared,
 * we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeJPG::
make_PNMFileTypeJPG(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // HAVE_JPEG
