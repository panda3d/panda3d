/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file download_utils.cxx
 * @author mike
 * @date 1999-01-18
 */

#include "pandabase.h"

#ifdef HAVE_ZLIB

#include "download_utils.h"
#include "config_downloader.h"
#include <zlib.h>

using std::ios;

unsigned long
check_crc(Filename name) {
  pifstream read_stream;
  name.set_binary();
  if (!name.open_read(read_stream)) {
    downloader_cat.error()
      << "check_crc() - Failed to open input file: " << name << std::endl;
    return 0;
  }

  // Determine the length of the file and read it into the buffer
  read_stream.seekg(0, ios::end);
  int buffer_length = read_stream.tellg();
  char *buffer = new char[buffer_length];
  read_stream.seekg(0, ios::beg);
  read_stream.read(buffer, buffer_length);

  // Compute the crc
  unsigned long crc = crc32(0L, Z_NULL, 0);
  crc = crc32(crc, (unsigned char *)buffer, buffer_length);

  delete[] buffer;

  return crc;
}

unsigned long
check_adler(Filename name) {
  pifstream read_stream;
  name.set_binary();
  if (!name.open_read(read_stream)) {
    downloader_cat.error()
      << "check_adler() - Failed to open input file: " << name << std::endl;
    return 0;
  }

  // Determine the length of the file and read it into the buffer
  read_stream.seekg(0, ios::end);
  int buffer_length = read_stream.tellg();
  char *buffer = new char[buffer_length];
  read_stream.seekg(0, ios::beg);
  read_stream.read(buffer, buffer_length);

  // Compute the adler checksum
  unsigned long adler = adler32(0L, Z_NULL, 0);
  adler = adler32(adler, (unsigned char *)buffer, buffer_length);

  delete[] buffer;

  return adler;
}

#endif  // HAVE_ZLIB
