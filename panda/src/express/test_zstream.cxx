/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_zstream.cxx
 * @author drose
 * @date 2002-08-05
 */

#include "pandabase.h"
#include "zStream.h"
#include "filename.h"

#include <zlib.h>

using std::cerr;
using std::cout;
using std::istream;

void
stream_decompress(istream &source) {
  IDecompressStream zstream(&source, false);

  int ch = zstream.get();
  while (!zstream.eof() && !zstream.fail()) {
    cout.put(ch);
    ch = zstream.get();
  }
}

void
stream_compress(istream &source) {
  OCompressStream zstream(&cout, false);

  int ch = source.get();
  while (!source.eof() && !source.fail()) {
    zstream.put(ch);
    ch = source.get();
  }
}

void
zlib_decompress(istream &source) {
  // First, read the entire contents into a buffer.
  std::string data;

  int ch = source.get();
  while (!source.eof() && !source.fail()) {
    data += (char)ch;
    ch = source.get();
  }

  // Now call zlib to decompress the buffer directly.
  size_t source_len = data.length();
  // Take a stab on the appropriate size of the output buffer.
  size_t dest_len = source_len * 4;
  char *dest = (char *)PANDA_MALLOC_ARRAY(dest_len);

  uLongf actual_dest_len = dest_len;
  int result = uncompress((Bytef *)dest, &actual_dest_len,
                          (const Bytef *)data.data(), source_len);
  if (result != Z_OK) {
    cerr << "compress result == " << result << "\n";
  }

  while (result == Z_BUF_ERROR) {
    dest_len *= 2;
    cerr << "Increasing buffer size to " << dest_len << "\n";
    dest = (char *)PANDA_REALLOC_ARRAY(dest, dest_len);

    actual_dest_len = dest_len;
    result = uncompress((Bytef *)dest, &actual_dest_len,
                        (const Bytef *)data.data(), source_len);
    if (result != Z_OK) {
      cerr << "compress result == " << result << "\n";
    }
  }

  cout.write(dest, actual_dest_len);
}

void
zlib_compress(istream &source) {
  // First, read the entire contents into a buffer.
  std::string data;

  int ch = source.get();
  while (!source.eof() && !source.fail()) {
    data += (char)ch;
    ch = source.get();
  }

  // Now call zlib to compress the buffer directly.
  size_t source_len = data.length();
  size_t dest_len = (size_t)(source_len * 1.1) + 12;
  char *dest = (char *)PANDA_MALLOC_ARRAY(dest_len);

  uLongf actual_dest_len = dest_len;
  int result = compress((Bytef *)dest, &actual_dest_len,
                        (const Bytef *)data.data(), source_len);

  if (result != Z_OK) {
    cerr << "compress result == " << result << "\n";
  }

  cout.write(dest, actual_dest_len);
}

int
main(int argc, char *argv[]) {
  bool zlib_direct = false;

  if (argc >= 2 && strcmp(argv[1], "-z") == 0) {
    zlib_direct = true;
    argc--;
    argv++;
  }

  if (argc != 2) {
    cerr << "test_zstream [-z] file\n"
         << "compresses file to standard output, or decompresses it if the\n"
         << "filename ends in .pz.\n\n"
         << "With -z, calls zlib directly instead of using the zstream interface.\n";
    return (1);
  }

  Filename source_filename = argv[1];
  source_filename.set_binary();

  pifstream source;

  if (!source_filename.open_read(source)) {
    cerr << "Unable to open source " << source_filename << ".\n";
    return (1);
  }

  if (zlib_direct) {
    if (source_filename.get_extension() == "pz") {
      zlib_decompress(source);
    } else {
      zlib_compress(source);
    }
  } else {
    if (source_filename.get_extension() == "pz") {
      stream_decompress(source);
    } else {
      stream_compress(source);
    }
  }

  return (0);
}
