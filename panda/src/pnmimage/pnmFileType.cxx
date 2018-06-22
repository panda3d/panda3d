/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileType.cxx
 * @author drose
 * @date 2000-06-15
 */

#include "pnmFileType.h"

#include "string_utils.h"
#include "executionEnvironment.h"
#include "bamReader.h"
#include "bamWriter.h"

using std::string;

bool PNMFileType::_did_init_pnm = false;
TypeHandle PNMFileType::_type_handle;

/**
 *
 */
PNMFileType::
PNMFileType() {
}

/**
 *
 */
PNMFileType::
~PNMFileType() {
}

/**
 * Returns the number of different possible filename extensions associated
 * with this particular file type.
 */
int PNMFileType::
get_num_extensions() const {
  return 0;
}

/**
 * Returns the nth possible filename extension associated with this particular
 * file type, without a leading dot.
 */
string PNMFileType::
get_extension(int) const {
  nassertr(false, string());
  return string();
}

/**
 * Returns a suitable filename extension (without a leading dot) to suggest
 * for files of this type, or empty string if no suggestions are available.
 */
string PNMFileType::
get_suggested_extension() const {
  if (get_num_extensions() > 0) {
    return get_extension(0);
  }
  return string();
}

/**
 * Returns true if this particular file type uses a magic number to identify
 * it, false otherwise.
 */
bool PNMFileType::
has_magic_number() const {
  return false;
}

/**
 * Returns true if the indicated "magic number" byte stream (the initial few
 * bytes read from the file) matches this particular file type, false
 * otherwise.
 */
bool PNMFileType::
matches_magic_number(const string &) const {
  return false;
}

/**
 * Allocates and returns a new PNMReader suitable for reading from this file
 * type, if possible.  If reading from this file type is not supported,
 * returns NULL.
 */
PNMReader *PNMFileType::
make_reader(std::istream *, bool, const string &) {
  return nullptr;
}

/**
 * Allocates and returns a new PNMWriter suitable for reading from this file
 * type, if possible.  If writing files of this type is not supported, returns
 * NULL.
 */
PNMWriter *PNMFileType::
make_writer(std::ostream *, bool) {
  return nullptr;
}

/**
 * Initializes the underlying PNM library, if it has not already been
 * initialized.  This should be called by every implementation of
 * make_reader() and make_writer(), to ensure that the library is properly
 * initialized before any I/O is attempted.
 */
void PNMFileType::
init_pnm() {
  if (!_did_init_pnm) {
    _did_init_pnm = true;

    // No reason to do anything here nowadays.
  }
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 *
 * None of the particular PNMFileType objects store any extra data--at least,
 * not yet--so we just define this up here to do nothing.
 */
void PNMFileType::
write_datagram(BamWriter *, Datagram &) {
}
