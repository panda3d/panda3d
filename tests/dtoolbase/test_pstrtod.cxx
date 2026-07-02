/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pstrtod.cxx
 * @author drose
 * @date 2009-06-14
 */

#include "pstrtod.h"

#include "catch_amalgamated.hpp"

#include <locale.h>

TEST_CASE("pstrtod parses a simple decimal number", "[dtoolbase]") {
  char *endptr = nullptr;
  double result = pstrtod("3.14", &endptr);
  CHECK(result == Catch::Approx(3.14));
  // endptr should point at the terminating null.
  REQUIRE(endptr != nullptr);
  CHECK(*endptr == '\0');
}

TEST_CASE("pstrtod reports where parsing stopped", "[dtoolbase]") {
  const char *str = "2.5 rest";
  char *endptr = nullptr;
  double result = pstrtod(str, &endptr);
  CHECK(result == Catch::Approx(2.5));
  REQUIRE(endptr != nullptr);
  // Parsing stops at the first character that is not part of the number.
  CHECK(endptr == str + 3);
  CHECK(std::string(endptr) == " rest");
}

TEST_CASE("pstrtod handles scientific notation and sign", "[dtoolbase]") {
  CHECK(pstrtod("-1.5e3", nullptr) == Catch::Approx(-1500.0));
  CHECK(pstrtod("+0.25", nullptr) == Catch::Approx(0.25));
  CHECK(pstrtod("1E-2", nullptr) == Catch::Approx(0.01));
}

TEST_CASE("pstrtod is locale-independent", "[dtoolbase]") {
  // Some locales (e.g. de_DE) use ',' as the decimal separator.  pstrtod must
  // ignore that and always use '.', so "1,5" parses as 1 with ",5" left over,
  // never as 1.5.  Attempt to install such a locale; if it is not available on
  // this system, fall back to testing with the "C" locale.
  char *orig = setlocale(LC_NUMERIC, nullptr);
  std::string orig_locale = (orig != nullptr) ? orig : "C";

  const char *candidates[] = {"de_DE.UTF-8", "de_DE", "de_DE.utf8"};
  for (const char *loc : candidates) {
    if (setlocale(LC_NUMERIC, loc) != nullptr) {
      break;
    }
  }

  char *endptr = nullptr;
  double result = pstrtod("1,5", &endptr);
  CHECK(result == Catch::Approx(1.0));
  REQUIRE(endptr != nullptr);
  CHECK(std::string(endptr) == ",5");

  // And the '.' form still works no matter the locale.
  CHECK(pstrtod("1.5", nullptr) == Catch::Approx(1.5));

  setlocale(LC_NUMERIC, orig_locale.c_str());
}

TEST_CASE("patof parses like atof", "[dtoolbase]") {
  CHECK(patof("2.5") == Catch::Approx(2.5));
  CHECK(patof("-10") == Catch::Approx(-10.0));
  // Leading whitespace is skipped, trailing junk is ignored.
  CHECK(patof("  6.02xyz") == Catch::Approx(6.02));
}
