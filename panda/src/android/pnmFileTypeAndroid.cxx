/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeAndroid.cxx
 * @author rdb
 * @date 2013-01-11
 */

#include "pnmFileTypeAndroid.h"

#ifdef ANDROID

#include "config_pnmimagetypes.h"

#include "pnmFileTypeRegistry.h"
#include "bamReader.h"

static const char * const extensions_android[] = {
  "jpg", "jpeg", "gif", "png",//"webp" (android 4.0+)
};
static const int num_extensions_android = sizeof(extensions_android) / sizeof(const char *);

TypeHandle PNMFileTypeAndroid::_type_handle;

/**

 */
PNMFileTypeAndroid::
PNMFileTypeAndroid() {
}

/**
 * Returns a few words describing the file type.
 */
string PNMFileTypeAndroid::
get_name() const {
  return "Android Bitmap";
}

/**
 * Returns the number of different possible filename extensions associated with
 * this particular file type.
 */
int PNMFileTypeAndroid::
get_num_extensions() const {
  return num_extensions_android;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileTypeAndroid::
get_extension(int n) const {
  nassertr(n >= 0 && n < num_extensions_android, string());
  return extensions_android[n];
}

/**
 * Returns true if this particular file type uses a magic number to identify it,
 * false otherwise.
 */
bool PNMFileTypeAndroid::
has_magic_number() const {
  return false;
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported, returns
 * NULL.
 */
PNMReader *PNMFileTypeAndroid::
make_reader(istream *file, bool owns_file, const string &magic_number) {
  init_pnm();
  return new Reader(this, file, owns_file, magic_number);
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PNMFileTypeAndroid::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PNMFileTypeAndroid);
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.  In the case of the PNMFileType objects, since these
 * objects are all shared, we just pull the object from the registry.
 */
TypedWritable *PNMFileTypeAndroid::
make_PNMFileTypeAndroid(const FactoryParams &params) {
  return PNMFileTypeRegistry::get_global_ptr()->get_type_by_handle(get_class_type());
}

#endif  // ANDROID
