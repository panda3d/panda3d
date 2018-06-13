/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramGenerator.cxx
 * @author jason
 * @date 2000-06-07
 */

#include "pandabase.h"

#include "datagramGenerator.h"
#include "temporaryFile.h"

/**
 * Does nothing since this is class is just the definition of an interface
 */
DatagramGenerator::
~DatagramGenerator() {
}

/**
 * Skips over the next datagram without extracting it, but saves the relevant
 * file information in the SubfileInfo object so that its data may be read
 * later.  For non-file-based datagram generators, this may mean creating a
 * temporary file and copying the contents of the datagram to disk.
 *
 * Returns true on success, false on failure or if this method is
 * unimplemented.
 */
bool DatagramGenerator::
save_datagram(SubfileInfo &info) {
  return false;
}

/**
 * Returns the filename that provides the source for these datagrams, if any,
 * or empty string if the datagrams do not originate from a file on disk.
 */
const Filename &DatagramGenerator::
get_filename() {
  const FileReference *file = get_file();
  if (file != nullptr) {
    return file->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

/**
 * Returns the on-disk timestamp of the file that was read, at the time it was
 * opened, if that is available, or 0 if it is not.
 */
time_t DatagramGenerator::
get_timestamp() const {
  return 0;
}

/**
 * Returns the FileReference that provides the source for these datagrams, if
 * any, or NULL if the datagrams do not originate from a file on disk.
 */
const FileReference *DatagramGenerator::
get_file() {
  return nullptr;
}

/**
 * Returns the VirtualFile that provides the source for these datagrams, if
 * any, or NULL if the datagrams do not originate from a VirtualFile.
 */
VirtualFile *DatagramGenerator::
get_vfile() {
  return nullptr;
}

/**
 * Returns the current file position within the data stream, if any, or 0 if
 * the file position is not meaningful or cannot be determined.
 *
 * For DatagramGenerators that return a meaningful file position, this will be
 * pointing to the first byte following the datagram returned after a call to
 * get_datagram().
 */
std::streampos DatagramGenerator::
get_file_pos() {
  return 0;
}
