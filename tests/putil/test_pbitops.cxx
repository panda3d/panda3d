/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_pbitops.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "pbitops.h"

#include "catch_amalgamated.hpp"

// Portable, obviously-correct reference implementations.  These loop one bit
// at a time and never use a shift wider than the type.
namespace {
  template<class T>
  int ref_count_bits(T x) {
    int n = 0;
    for (unsigned int i = 0; i < sizeof(T) * 8; ++i) {
      n += (int)((x >> i) & T(1));
    }
    return n;
  }

  template<class T>
  int ref_lowest_on_bit(T x) {
    for (unsigned int i = 0; i < sizeof(T) * 8; ++i) {
      if ((x >> i) & T(1)) {
        return (int)i;
      }
    }
    return -1;
  }

  template<class T>
  int ref_highest_on_bit(T x) {
    // Scans downward with an early return rather than remembering the last
    // matching index in an upward scan: GCC 16.1 on aarch64 miscompiles the
    // latter at -O3 by vectorizing it as a conditional last-index reduction.
    for (int i = (int)(sizeof(T) * 8) - 1; i >= 0; --i) {
      if ((x >> i) & T(1)) {
        return i;
      }
    }
    return -1;
  }

  // Every bit at or below the highest set bit becomes 1.
  template<class T>
  T ref_flood_down(T x) {
    int h = ref_highest_on_bit(x);
    unsigned long long acc = 0;
    for (int i = 0; i <= h; ++i) {
      acc |= (1ULL << i);
    }
    return (T)acc;
  }

  // Every bit at or above the lowest set bit becomes 1.
  template<class T>
  T ref_flood_up(T x) {
    int l = ref_lowest_on_bit(x);
    if (l < 0) {
      return T(0);
    }
    unsigned long long acc = 0;
    for (unsigned int i = (unsigned int)l; i < sizeof(T) * 8; ++i) {
      acc |= (1ULL << i);
    }
    return (T)acc;
  }

  // Check every pbitops routine against the reference for one input value.
  template<class T>
  void check_all(T x) {
    CAPTURE(sizeof(T) * 8, (unsigned long long)x);
    CHECK(count_bits_in_word(x) == ref_count_bits(x));
    CHECK(get_lowest_on_bit(x) == ref_lowest_on_bit(x));
    CHECK(get_highest_on_bit(x) == ref_highest_on_bit(x));
    CHECK(flood_bits_down(x) == ref_flood_down(x));
    CHECK(flood_bits_up(x) == ref_flood_up(x));
    // get_next_higher_bit is defined as get_highest_on_bit + 1.
    CHECK(get_next_higher_bit(x) == ref_highest_on_bit(x) + 1);
  }
}

TEMPLATE_TEST_CASE("pbitops routines agree with a naive reference", "[putil]",
                   unsigned short, unsigned int,
                   unsigned long, unsigned long long) {
  const unsigned int width = sizeof(TestType) * 8;
  const TestType all_ones = (TestType)~TestType(0);

  SECTION("boundary values") {
    check_all<TestType>(0);
    check_all<TestType>(1);
    check_all<TestType>(2);
    check_all<TestType>(3);
    check_all<TestType>(all_ones);
    check_all<TestType>(all_ones - 1);
    check_all<TestType>((TestType)(all_ones >> 1));            // 0 followed by 1s
    check_all<TestType>((TestType)(TestType(1) << (width - 1)));  // only the MSB
  }

  SECTION("every single-bit value") {
    // Exercises the bit-scan intrinsics at each position, including the MSB.
    for (unsigned int i = 0; i < width; ++i) {
      check_all<TestType>((TestType)(TestType(1) << i));
    }
  }

  SECTION("adjacent bit pairs and alternating patterns") {
    for (unsigned int i = 0; i + 1 < width; ++i) {
      check_all<TestType>((TestType)((TestType(1) << i) | (TestType(1) << (i + 1))));
    }
    check_all<TestType>((TestType)0xaaaaaaaaaaaaaaaaULL);
    check_all<TestType>((TestType)0x5555555555555555ULL);
    check_all<TestType>((TestType)0xf0f0f0f0f0f0f0f0ULL);
  }
}

TEST_CASE("count_bits_in_word matches the num_bits_on table exhaustively", "[putil]") {
  // The 16-bit overload is a straight table lookup; walking all 65536 values
  // validates the entire precomputed num_bits_on[] table in one shot.
  for (int i = 0; i <= 0xffff; ++i) {
    unsigned short x = (unsigned short)i;
    REQUIRE(count_bits_in_word(x) == ref_count_bits(x));
    REQUIRE((int)num_bits_on[i] == ref_count_bits(x));
  }
}

TEST_CASE("get_lowest/highest_on_bit report -1 for an empty word", "[putil]") {
  CHECK(get_lowest_on_bit((unsigned short)0) == -1);
  CHECK(get_lowest_on_bit((unsigned int)0) == -1);
  CHECK(get_lowest_on_bit((unsigned long)0) == -1);
  CHECK(get_lowest_on_bit((unsigned long long)0) == -1);

  CHECK(get_highest_on_bit((unsigned short)0) == -1);
  CHECK(get_highest_on_bit((unsigned int)0) == -1);
  CHECK(get_highest_on_bit((unsigned long)0) == -1);
  CHECK(get_highest_on_bit((unsigned long long)0) == -1);
}

TEST_CASE("get_next_higher_bit finds the smallest enclosing power of two", "[putil]") {
  // Documented contract: the smallest n such that (1 << n) is larger than x.
  CHECK(get_next_higher_bit((unsigned int)0) == 0);   // 1 << 0 == 1 > 0
  CHECK(get_next_higher_bit((unsigned int)1) == 1);   // 1 << 1 == 2 > 1
  CHECK(get_next_higher_bit((unsigned int)5) == 3);   // 1 << 3 == 8 > 5
  CHECK(get_next_higher_bit((unsigned int)8) == 4);   // 1 << 4 == 16 > 8
  CHECK(get_next_higher_bit((unsigned int)255) == 8);
  CHECK(get_next_higher_bit((unsigned int)256) == 9);
}
