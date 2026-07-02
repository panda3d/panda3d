/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_patomic.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "patomic.h"

#include "catch_amalgamated.hpp"

TEST_CASE("patomic<T> read-modify-write returns the previous value", "[dtoolbase]") {
  patomic<int> a(10);
  CHECK(a.load() == 10);

  // Each fetch_* must return the value from *before* the operation.
  CHECK(a.fetch_add(5) == 10);
  CHECK(a.load() == 15);
  CHECK(a.fetch_sub(3) == 15);
  CHECK(a.load() == 12);

  a.store(0xf0);
  CHECK(a.fetch_and(0x3c) == 0xf0);
  CHECK(a.load() == 0x30);
  CHECK(a.fetch_or(0x0f) == 0x30);
  CHECK(a.load() == 0x3f);
  CHECK(a.fetch_xor(0xff) == 0x3f);
  CHECK(a.load() == 0xc0);

  // exchange returns the old value and installs the new one.
  CHECK(a.exchange(7) == 0xc0);
  CHECK(a.load() == 7);
}

TEST_CASE("patomic<T>::compare_exchange_strong follows the CAS contract", "[dtoolbase]") {
  patomic<int> a(100);

  int expected = 100;
  REQUIRE(a.compare_exchange_strong(expected, 200));
  CHECK(a.load() == 200);
  CHECK(expected == 100);  // unchanged on success

  // On failure the value is left alone and 'expected' is loaded with the
  // actual current value, so a retry loop can make progress.
  expected = 100;
  REQUIRE_FALSE(a.compare_exchange_strong(expected, 999));
  CHECK(a.load() == 200);
  CHECK(expected == 200);  // refreshed to the current value on failure
}

TEST_CASE("patomic_unsigned_lock_free default-constructs to zero", "[dtoolbase]") {
  // The class follows C++20 semantics, NOT C++11: a default-constructed atomic
  // must be initialized to zero.
  patomic_unsigned_lock_free a;
  CHECK(a.load() == 0u);

  patomic_unsigned_lock_free b(42u);
  CHECK(b.load() == 42u);

  CHECK(b.fetch_add(8u) == 42u);
  CHECK(b.load() == 50u);
  CHECK(b.fetch_sub(50u) == 50u);
  CHECK(b.load() == 0u);
}

TEST_CASE("patomic_signed_lock_free default-constructs to zero", "[dtoolbase]") {
  patomic_signed_lock_free a;
  CHECK(a.load() == 0);

  patomic_signed_lock_free b(-5);
  CHECK(b.load() == -5);

  // Signed arithmetic, including crossing zero.
  CHECK(b.fetch_add(10) == -5);
  CHECK(b.load() == 5);
  CHECK(b.fetch_sub(8) == 5);
  CHECK(b.load() == -3);
}

TEST_CASE("patomic_flag implements test-and-set", "[dtoolbase]") {
  // Default construction leaves the flag clear (C++20 semantics).
  patomic_flag flag;
  CHECK_FALSE(flag.test());

  // First test_and_set observes the old (clear) state and sets the flag.
  CHECK_FALSE(flag.test_and_set());
  CHECK(flag.test());

  // A second test_and_set now observes the set state.
  CHECK(flag.test_and_set());
  CHECK(flag.test());

  // clear() returns it to false; a subsequent test_and_set again returns the
  // old (now clear) value.
  flag.clear();
  CHECK_FALSE(flag.test());
  CHECK_FALSE(flag.test_and_set());
  CHECK(flag.test());
}

TEST_CASE("patomic_flag can be initialized set", "[dtoolbase]") {
  patomic_flag flag(true);
  CHECK(flag.test());
  CHECK(flag.test_and_set());
  flag.clear();
  CHECK_FALSE(flag.test());
}
