/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_ordered_vector.cxx
 * @author drose
 * @date 2002-02-20
 */

#include "ordered_vector.h"
#include "vector_int.h"

#include "catch_amalgamated.hpp"

typedef ov_multiset<int> myvec;

TEST_CASE("ov_multiset keeps its elements in sorted order", "[express]") {
  myvec v;
  v.insert(v.end(), 3);
  v.insert(v.end(), 5);
  v.insert(v.end(), 4);
  v.insert(v.end(), 4);
  v.insert(v.end(), 2);
  v.insert(v.end(), 5);
  v.insert(v.end(), 2);

  // A multiset keeps duplicates, so all seven elements are retained, sorted.
  REQUIRE(v.size() == 7);
  CHECK(std::vector<int>(v.begin(), v.end())
        == std::vector<int>({2, 2, 3, 4, 4, 5, 5}));
}

TEST_CASE("ov_multiset::swap exchanges contents", "[express]") {
  myvec a, b;
  a.insert(a.end(), 1);
  a.insert(a.end(), 2);

  a.swap(b);
  CHECK(a.empty());
  REQUIRE(b.size() == 2);
  CHECK(b[0] == 1);
  CHECK(b[1] == 2);
}

TEST_CASE("ov_multiset::count reports the multiplicity of a key", "[express]") {
  myvec v;
  for (int x : {2, 2, 3, 4, 4, 5, 5}) {
    v.insert(v.end(), x);
  }

  CHECK(v.count(1) == 0);
  CHECK(v.count(2) == 2);
  CHECK(v.count(3) == 1);
  CHECK(v.count(4) == 2);
  CHECK(v.count(5) == 2);
  CHECK(v.count(6) == 0);
}

TEST_CASE("ov_multiset::equal_range bounds the run of a key", "[express]") {
  myvec v;
  for (int x : {2, 2, 3, 4, 4, 5, 5}) {
    v.insert(v.end(), x);
  }

  auto range = v.equal_range(4);
  // The two 4s sit at indices 3 and 4.
  CHECK(range.first - v.begin() == 3);
  CHECK(range.second - v.begin() == 5);

  // A key that is not present yields an empty range at its insertion point.
  auto missing = v.equal_range(1);
  CHECK(missing.first == missing.second);
  CHECK(missing.first == v.begin());

  auto past_end = v.equal_range(6);
  CHECK(past_end.first == past_end.second);
  CHECK(past_end.first == v.end());
}

TEST_CASE("ov_multiset::erase removes every matching element", "[express]") {
  myvec v;
  for (int x : {2, 2, 3, 4, 4, 5, 5}) {
    v.insert(v.end(), x);
  }

  size_t removed = v.erase(4);
  CHECK(removed == 2);
  REQUIRE(v.size() == 5);
  CHECK(std::vector<int>(v.begin(), v.end())
        == std::vector<int>({2, 2, 3, 5, 5}));
  CHECK(v.count(4) == 0);

  // Erasing a key that is not present is a no-op.
  CHECK(v.erase(4) == 0);
  CHECK(v.size() == 5);
}
