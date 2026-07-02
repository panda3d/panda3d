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

// zStream is only compiled when zlib is available.
#ifdef HAVE_ZLIB

#include "zStream.h"

#include "catch_amalgamated.hpp"

#include <sstream>

// OCompressStream / IDecompressStream wrap zlib to (de)compress a byte stream
// on the fly.  The compression happens through the inherited iostream
// operators, which interrogate does not expose, so this round-trip is tested
// in C++ rather than through the published API.

namespace {
  std::string compress(const std::string &data, int level = 6) {
    std::ostringstream dest;
    {
      OCompressStream zstream(&dest, false, level);
      zstream.write(data.data(), data.size());
    }  // Destructor flushes and finishes the zlib stream.
    return dest.str();
  }

  std::string decompress(const std::string &compressed) {
    std::istringstream source(compressed);
    IDecompressStream zstream(&source, false);

    std::string result;
    int ch = zstream.get();
    while (!zstream.eof() && !zstream.fail()) {
      result += (char)ch;
      ch = zstream.get();
    }
    return result;
  }
}

TEST_CASE("compress/decompress is a faithful round trip", "[express]") {
  std::string original =
    "The quick brown fox jumps over the lazy dog. "
    "The quick brown fox jumps over the lazy dog. "
    "The quick brown fox jumps over the lazy dog.";

  std::string compressed = compress(original);
  CHECK(compressed != original);
  CHECK(decompress(compressed) == original);
}

TEST_CASE("compression shrinks highly redundant data", "[express]") {
  std::string original(10000, 'a');

  std::string compressed = compress(original);
  CHECK(compressed.size() < original.size());
  CHECK(decompress(compressed) == original);
}

TEST_CASE("round trip preserves arbitrary binary bytes", "[express]") {
  std::string original;
  for (int i = 0; i < 1000; ++i) {
    original += (char)((i * 37) & 0xff);
  }

  std::string compressed = compress(original);
  std::string result = decompress(compressed);
  REQUIRE(result.size() == original.size());
  CHECK(result == original);
}

TEST_CASE("an empty stream round-trips to empty", "[express]") {
  std::string compressed = compress("");
  CHECK(decompress(compressed).empty());
}

#endif  // HAVE_ZLIB
