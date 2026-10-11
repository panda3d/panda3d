/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pointertoarray.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "pta_int.h"

#include "catch_amalgamated.hpp"

TEST_CASE("PointerToArray copies share the underlying vector", "[express]") {
  PTA_int a;
  a.push_back(1);
  a.push_back(2);
  a.push_back(3);
  REQUIRE(a.get_ref_count() == 1);

  PTA_int b = a;
  CHECK(b.get_ref_count() == 2);
  CHECK(b.p() == a.p());

  // Assignment is by pointer, so writes through one handle are visible
  // through the other.
  b.set_element(0, 42);
  CHECK(a.get_element(0) == 42);
}

TEST_CASE("cast_non_const on an lvalue CPTA adds a reference", "[express]") {
  CPTA_int c(pvector<int>({1, 2, 3}));
  REQUIRE(c.get_ref_count() == 1);

  PTA_int nc = c.cast_non_const();
  CHECK(nc.p() == c.p());
  CHECK(c.get_ref_count() == 2);
  CHECK_FALSE(c.is_null());
}

TEST_CASE("cast_non_const on an rvalue CPTA transfers the reference", "[express]") {
  CPTA_int c(pvector<int>({1, 2, 3}));
  REQUIRE(c.get_ref_count() == 1);

  PTA_int nc = std::move(c).cast_non_const();

  // The reference was handed over without touching the count, so sole
  // ownership remains detectable on the result.
  CHECK(c.is_null());
  REQUIRE(nc.get_ref_count() == 1);
  REQUIRE(nc.size() == 3);
  CHECK(nc[0] == 1);

  // Which is the point of the rvalue overload: the sole owner may donate the
  // underlying vector to a consumer that wants to own it outright.
  pvector<int> taken = std::move(nc.v());
  CHECK(taken.size() == 3);
  CHECK(taken[2] == 3);
}
