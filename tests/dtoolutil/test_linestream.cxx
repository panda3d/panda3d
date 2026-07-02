/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_linestream.cxx
 * @author drose
 * @date 2000-02-26
 */

#include "lineStream.h"

#include "catch_amalgamated.hpp"

TEST_CASE("a fresh LineStream has no text available", "[dtoolutil]") {
  LineStream ls;
  CHECK_FALSE(ls.is_text_available());
}

TEST_CASE("a complete line is reported with its newline", "[dtoolutil]") {
  LineStream ls;
  ls << "hello\n";

  REQUIRE(ls.is_text_available());
  CHECK(ls.get_line() == "hello");
  CHECK(ls.has_newline());

  // The single line has now been consumed.
  CHECK_FALSE(ls.is_text_available());
}

TEST_CASE("lines are extracted one at a time", "[dtoolutil]") {
  LineStream ls;
  ls << "one\ntwo\nthree\n";

  REQUIRE(ls.is_text_available());
  CHECK(ls.get_line() == "one");
  CHECK(ls.has_newline());
  CHECK(ls.get_line() == "two");
  CHECK(ls.has_newline());
  CHECK(ls.get_line() == "three");
  CHECK(ls.has_newline());
  CHECK_FALSE(ls.is_text_available());
}

TEST_CASE("a partial line is available but flagged as having no newline", "[dtoolutil]") {
  LineStream ls;
  ls << "partial";

  REQUIRE(ls.is_text_available());
  CHECK(ls.get_line() == "partial");
  CHECK_FALSE(ls.has_newline());
}

TEST_CASE("a line may be assembled from several writes", "[dtoolutil]") {
  LineStream ls;
  ls << "wor";
  ls << "ld";
  ls << "\n";

  REQUIRE(ls.is_text_available());
  CHECK(ls.get_line() == "world");
  CHECK(ls.has_newline());
}

TEST_CASE("a completed line and a trailing partial line are distinguished", "[dtoolutil]") {
  LineStream ls;
  ls << "done\nrest";

  CHECK(ls.get_line() == "done");
  CHECK(ls.has_newline());

  REQUIRE(ls.is_text_available());
  CHECK(ls.get_line() == "rest");
  CHECK_FALSE(ls.has_newline());
}
