/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_stl_compares.cxx
 * @author rdb
 * @date 2026-07-03
 */

#include "stl_compares.h"

#include "catch_amalgamated.hpp"

#include <string>
#include <string_view>

using std::string;

namespace {
  // A trivial object exposing compare_to(), for compare_to<> /
  // indirect_compare_to<>.
  struct Item {
    int _v;
    int compare_to(const Item &other) const {
      return (_v < other._v) ? -1 : ((_v > other._v) ? 1 : 0);
    }
  };

  // A trivial object exposing get_name(), for indirect_compare_names<>.
  struct Named {
    string _name;
    const string &get_name() const { return _name; }
  };

  // A trivial object exposing get_hash() and operator ==, for method_hash<> etc.
  struct Hashable {
    size_t _h;
    size_t get_hash() const { return _h; }
    bool operator == (const Hashable &other) const { return _h == other._h; }
    bool operator < (const Hashable &other) const { return _h < other._h; }
  };
}

TEST_CASE("floating_point_threshold buckets values within the threshold", "[dtoolbase]") {
  // With a threshold of 1.0, values are grouped by round-to-nearest bucket.
  floating_point_threshold<double> lt(1.0);

  // Values that round to the same bucket compare equal (neither sorts first).
  CHECK_FALSE(lt(10.2, 10.3));
  CHECK_FALSE(lt(10.3, 10.2));

  // A value is never less than itself.
  CHECK_FALSE(lt(10.2, 10.2));

  // Values in different buckets order by magnitude.
  CHECK(lt(10.2, 11.2));
  CHECK_FALSE(lt(11.2, 10.2));

  // The 0.5 rounding boundary: 10.6 rounds up to bucket 11.
  CHECK(lt(10.2, 10.6));
}

TEST_CASE("compare_to functor orders via the object's compare_to()", "[dtoolbase]") {
  compare_to<Item> lt;
  Item a{1}, b{2}, a2{1};

  CHECK(lt(a, b));
  CHECK_FALSE(lt(b, a));
  CHECK_FALSE(lt(a, a2));

  CHECK(lt.is_equal(a, a2));
  CHECK_FALSE(lt.is_equal(a, b));
}

TEST_CASE("indirect_less dereferences pointers and guards identity", "[dtoolbase]") {
  indirect_less<const int *> lt;
  int x = 1, y = 2, x2 = 1;

  CHECK(lt(&x, &y));
  CHECK_FALSE(lt(&y, &x));

  // Equal values in distinct objects: neither sorts first.
  CHECK_FALSE(lt(&x, &x2));

  // The same pointer is never less than itself (identity short-circuit).
  CHECK_FALSE(lt(&x, &x));
}

TEST_CASE("indirect_compare_to orders and compares via pointed-to compare_to()", "[dtoolbase]") {
  indirect_compare_to<const Item *> cmp;
  Item a{1}, b{2}, a2{1};

  CHECK(cmp(&a, &b));
  CHECK_FALSE(cmp(&b, &a));
  CHECK_FALSE(cmp(&a, &a));

  // is_equal is true for the identical pointer without dereferencing...
  CHECK(cmp.is_equal(&a, &a));
  // ...and for distinct pointers whose objects compare equal.
  CHECK(cmp.is_equal(&a, &a2));
  CHECK_FALSE(cmp.is_equal(&a, &b));
}

TEST_CASE("indirect_compare_names orders by get_name()", "[dtoolbase]") {
  indirect_compare_names<const Named *> cmp;
  Named apple{"apple"}, banana{"banana"}, apple2{"apple"};

  CHECK(cmp(&apple, &banana));
  CHECK_FALSE(cmp(&banana, &apple));
  CHECK_FALSE(cmp(&apple, &apple));

  CHECK(cmp.is_equal(&apple, &apple));
  CHECK(cmp.is_equal(&apple, &apple2));
  CHECK_FALSE(cmp.is_equal(&apple, &banana));
}

TEST_CASE("floating_point_hash keeps hashing and ordering consistent", "[dtoolbase]") {
  // The comparator and the hash must agree on which values are equivalent,
  // otherwise the type is unusable as a hash-map key.
  floating_point_hash<double> h(1.0);

  // Same bucket: equal under the comparator and equal hash.
  CHECK_FALSE(h(10.2, 10.3));
  CHECK_FALSE(h(10.3, 10.2));
  CHECK(h(10.2) == h(10.3));

  // Different bucket: ordered, and (with overwhelming likelihood) distinct hash.
  CHECK(h(10.2, 11.2));
  CHECK(h(10.2) != h(11.2));

  // Hashing is deterministic.
  CHECK(h(10.2) == h(10.2));
}

TEST_CASE("string_hash hashes strings deterministically", "[dtoolbase]") {
  string_hash h;

  CHECK(h(string("panda3d")) == h(string("panda3d")));
  CHECK(h(string("panda3d")) != h(string("Panda3D")));

  // The empty string is handled without dereferencing past the end.
  CHECK(h(string()) == h(string("")));
}

TEST_CASE("string_hash provides a transparent comparator", "[dtoolbase]") {
  // is_transparent lets heterogeneous lookups (e.g. by string_view) work.
  string_hash cmp;

  CHECK(cmp(string("abc"), string("abd")));
  CHECK_FALSE(cmp(string("abd"), string("abc")));

  // Heterogeneous comparison against a string_view.
  CHECK(cmp(string("abc"), std::string_view("abd")));
  CHECK_FALSE(cmp(std::string_view("abd"), string("abc")));
}

TEST_CASE("method_hash and indirect variants delegate to get_hash()", "[dtoolbase]") {
  Hashable obj{0xabcd};

  method_hash<Hashable> mh;
  CHECK(mh(obj) == 0xabcd);

  indirect_method_hash<const Hashable *, std::less<const Hashable *>> imh;
  CHECK(imh(&obj) == 0xabcd);

  indirect_equals_hash<const Hashable *> ieh;
  Hashable same{0xabcd}, diff{0x1234};
  CHECK(ieh(&obj) == 0xabcd);
  CHECK(ieh.is_equal(&obj, &obj));       // identity
  CHECK(ieh.is_equal(&obj, &same));      // distinct pointers, equal value
  CHECK_FALSE(ieh.is_equal(&obj, &diff));
}

TEST_CASE("integer_hash::add_hash is deterministic and mixes distinct keys", "[dtoolbase]") {
  // add_hash of the same key from the same seed is stable.
  CHECK(int_hash::add_hash(0, 42) == int_hash::add_hash(0, 42));

  // Distinct small integers produce distinct hashes (Jenkins mixing).
  size_t h0 = int_hash::add_hash(0, 0);
  size_t h1 = int_hash::add_hash(0, 1);
  size_t h2 = int_hash::add_hash(0, 2);
  CHECK(h0 != h1);
  CHECK(h1 != h2);
  CHECK(h0 != h2);

  // The running seed is folded in, so the same key from a different seed differs.
  CHECK(int_hash::add_hash(0, 42) != int_hash::add_hash(1, 42));
}

TEST_CASE("pointer_hash hashes pointer values deterministically", "[dtoolbase]") {
  int a = 0, b = 0;

  CHECK(pointer_hash::add_hash(0, &a) == pointer_hash::add_hash(0, &a));
  CHECK(pointer_hash::add_hash(0, &a) != pointer_hash::add_hash(0, &b));
  CHECK(pointer_hash::add_hash(0, nullptr) == pointer_hash::add_hash(0, nullptr));
}
