/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_small_vector.cxx
 * @author rdb
 * @date 2026-07-02
 */

#include "small_vector.h"

#include "catch_amalgamated.hpp"

#include <iterator>
#include <memory>
#include <sstream>
#include <string>

namespace {
  /**
   * An element type that records every special-member operation and keeps a
   * running count of live instances, so a test can assert that a small_vector
   * constructs and destructs exactly the objects it should and leaks none.
   *
   * A moved-from instance has its value poisoned to -1, which lets a test
   * detect a stale read of a source that should have been relocated.
   */
  struct Tracked {
    static int _default_ctor;
    static int _value_ctor;
    static int _copy_ctor;
    static int _move_ctor;
    static int _copy_assign;
    static int _move_assign;
    static int _dtor;
    static int _live;

    static void reset() {
      _default_ctor = _value_ctor = _copy_ctor = _move_ctor = 0;
      _copy_assign = _move_assign = _dtor = _live = 0;
    }

    // Total number of objects ever brought into existence, by any means.
    static int get_num_constructed() {
      return _default_ctor + _value_ctor + _copy_ctor + _move_ctor;
    }

    int _value;
    bool _moved_from;

    Tracked() : _value(0), _moved_from(false) { ++_default_ctor; ++_live; }
    Tracked(int v) : _value(v), _moved_from(false) { ++_value_ctor; ++_live; }

    Tracked(const Tracked &o) : _value(o._value), _moved_from(false) {
      ++_copy_ctor;
      ++_live;
    }
    Tracked(Tracked &&o) noexcept : _value(o._value), _moved_from(false) {
      o._moved_from = true;
      o._value = -1;
      ++_move_ctor;
      ++_live;
    }
    Tracked &operator =(const Tracked &o) {
      _value = o._value;
      _moved_from = false;
      ++_copy_assign;
      return *this;
    }
    Tracked &operator =(Tracked &&o) noexcept {
      _value = o._value;
      _moved_from = false;
      o._moved_from = true;
      o._value = -1;
      ++_move_assign;
      return *this;
    }
    ~Tracked() { ++_dtor; --_live; }
  };

  int Tracked::_default_ctor = 0;
  int Tracked::_value_ctor = 0;
  int Tracked::_copy_ctor = 0;
  int Tracked::_move_ctor = 0;
  int Tracked::_copy_assign = 0;
  int Tracked::_move_assign = 0;
  int Tracked::_dtor = 0;
  int Tracked::_live = 0;

  // A vector with a known, small inline capacity of 2, so that tests can drive
  // the small->large transition at a precise, predictable size.
  typedef small_vector<Tracked, 2> Vec;

  // Extract the values into a plain vector for easy comparison.
  std::vector<int> values(const Vec &v) {
    std::vector<int> out;
    for (const Tracked &t : v) {
      out.push_back(t._value);
    }
    return out;
  }
}

TEST_CASE("small_vector is empty and inline on construction", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    CHECK(v.empty());
    CHECK(v.size() == 0);
    CHECK(v.capacity() == 2);
    CHECK(v.begin() == v.end());
    // No storage was allocated and no element constructed.
    CHECK(Tracked::get_num_constructed() == 0);
  }
  CHECK(Tracked::_live == 0);
}

// A small_vector with static storage duration.  Its constexpr default
// constructor means the compiler constant-initializes this before any dynamic
// initialization runs, which is the property that makes small_vector safe to
// use during static init (it cannot itself be caught in the static init order
// fiasco).  small_vector is not a literal type (it has a non-trivial
// destructor), so this cannot be checked with a constexpr variable or
// static_assert; we instead confirm the object is valid and usable.
small_vector<int, 4> global_sv;

TEST_CASE("small_vector is usable with static storage duration", "[dtoolutil]") {
  CHECK(global_sv.empty());
  CHECK(global_sv.size() == 0);
  CHECK(global_sv.capacity() == 4);

  global_sv.push_back(1);
  global_sv.push_back(2);
  CHECK(global_sv.size() == 2);
  global_sv.clear();
}

TEST_CASE("small_vector push_back grows across the inline boundary", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;

    v.push_back(Tracked(10));
    v.push_back(Tracked(20));
    // Still inline: capacity unchanged, nothing relocated.
    CHECK(v.size() == 2);
    CHECK(v.capacity() == 2);
    CHECK(Tracked::_move_ctor == 2);  // the two temporaries moved in
    CHECK(values(v) == std::vector<int>({10, 20}));

    // This push exceeds inline capacity and forces a heap allocation, which
    // relocates the two existing elements by move.
    v.push_back(Tracked(30));
    CHECK(v.size() == 3);
    CHECK(v.capacity() == 4);            // doubled from 2
    CHECK(values(v) == std::vector<int>({10, 20, 30}));

    // Doubling again at the next boundary.
    v.push_back(Tracked(40));
    v.push_back(Tracked(50));
    CHECK(v.size() == 5);
    CHECK(v.capacity() == 8);
    CHECK(values(v) == std::vector<int>({10, 20, 30, 40, 50}));

    // small_vector relocates by construct+destruct, never by assignment.
    CHECK(Tracked::_copy_assign == 0);
    CHECK(Tracked::_move_assign == 0);
    // Every object ever constructed that is no longer live has been destructed.
    CHECK(Tracked::_live == (int)v.size());
    CHECK(Tracked::get_num_constructed() - Tracked::_dtor == (int)v.size());
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector emplace_back constructs in place", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    Tracked &ref = v.emplace_back(7);
    CHECK(ref._value == 7);
    CHECK(&ref == &v.back());
    CHECK(Tracked::_value_ctor == 1);
    CHECK(Tracked::_copy_ctor == 0);
    CHECK(Tracked::_move_ctor == 0);
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector element access", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    for (int i = 0; i < 5; ++i) {
      v.push_back(Tracked(i * 100));
    }
    CHECK(v.front()._value == 0);
    CHECK(v.back()._value == 400);
    CHECK(v[3]._value == 300);
    CHECK(v.at(2)._value == 200);
    CHECK(v.data() == &v[0]);

    v[1]._value = 999;
    CHECK(v.at(1)._value == 999);
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector iterators cover forward and reverse", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    for (int i = 1; i <= 4; ++i) {
      v.push_back(Tracked(i));
    }

    SECTION("forward range-for preserves order") {
      std::vector<int> seen;
      for (const Tracked &t : v) {
        seen.push_back(t._value);
      }
      CHECK(seen == std::vector<int>({1, 2, 3, 4}));
    }

    SECTION("reverse iteration") {
      std::vector<int> seen;
      for (auto it = v.rbegin(); it != v.rend(); ++it) {
        seen.push_back(it->_value);
      }
      CHECK(seen == std::vector<int>({4, 3, 2, 1}));
    }

    SECTION("end - begin equals size") {
      CHECK((size_t)(v.end() - v.begin()) == v.size());
      CHECK((size_t)(v.cend() - v.cbegin()) == v.size());
    }
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector copy makes an independent deep copy", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec a;
    a.push_back(Tracked(1));
    a.push_back(Tracked(2));
    a.push_back(Tracked(3));  // now large (capacity 4)

    SECTION("copy constructor") {
      Vec b(a);
      CHECK(values(b) == values(a));
      CHECK(b.data() != a.data());   // separate storage
      b[0]._value = 42;
      CHECK(a[0]._value == 1);        // mutation did not alias
    }

    SECTION("copy assignment") {
      Vec b;
      b.push_back(Tracked(99));
      b = a;
      CHECK(values(b) == std::vector<int>({1, 2, 3}));
      CHECK(b.data() != a.data());
      // Copy assignment clears then reconstructs; no element assignment used.
      CHECK(Tracked::_copy_assign == 0);
      CHECK(Tracked::_move_assign == 0);
    }
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector move steals from the source and empties it", "[dtoolutil]") {
  Tracked::reset();

  SECTION("move constructor, large source steals the pointer") {
    {
      Vec a;
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));  // large: capacity 4
      }
      const Tracked *old_data = a.data();
      int moves_before = Tracked::_move_ctor;

      Vec b(std::move(a));
      CHECK(values(b) == std::vector<int>({0, 1, 2, 3}));
      CHECK(b.data() == old_data);              // pointer was stolen, not copied
      CHECK(Tracked::_move_ctor == moves_before);  // no per-element move
      CHECK(a.size() == 0);
      CHECK(a.capacity() == 2);                 // reset to inline
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("move constructor, small source relocates each element") {
    {
      Vec a;
      a.push_back(Tracked(5));
      a.push_back(Tracked(6));  // inline
      int moves_before = Tracked::_move_ctor;

      Vec b(std::move(a));
      CHECK(values(b) == std::vector<int>({5, 6}));
      CHECK(Tracked::_move_ctor == moves_before + 2);  // both relocated
      CHECK(a.size() == 0);
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("move assignment") {
    {
      Vec a;
      a.push_back(Tracked(1));
      a.push_back(Tracked(2));
      a.push_back(Tracked(3));  // large

      Vec b;
      b.push_back(Tracked(7));  // this element must be destructed on assign
      b = std::move(a);

      CHECK(values(b) == std::vector<int>({1, 2, 3}));
      CHECK(a.size() == 0);
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("self move-assignment is a harmless no-op") {
    {
      Vec a;
      a.push_back(Tracked(1));
      a.push_back(Tracked(2));
      a.push_back(Tracked(3));

      Vec &alias = a;
      a = std::move(alias);
      CHECK(values(a) == std::vector<int>({1, 2, 3}));
    }
    CHECK(Tracked::_live == 0);
  }
}

TEST_CASE("small_vector clear destructs elements but keeps storage", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    for (int i = 0; i < 4; ++i) {
      v.push_back(Tracked(i));  // large, capacity 4
    }
    size_t cap = v.capacity();

    v.clear();
    CHECK(v.empty());
    CHECK(v.size() == 0);
    CHECK(v.capacity() == cap);    // storage retained
    CHECK(Tracked::_live == 0);   // all four destructed

    // Storage is reusable without reallocation.
    v.push_back(Tracked(99));
    CHECK(v.capacity() == cap);
    CHECK(v.back()._value == 99);
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector shrink_to_fit releases spare storage", "[dtoolutil]") {
  Tracked::reset();

  SECTION("shrinks large storage back to inline when it fits") {
    {
      Vec v;
      for (int i = 0; i < 4; ++i) {
        v.push_back(Tracked(i));   // large, capacity 4
      }
      v.pop_back();
      v.pop_back();                // size 2, still capacity 4
      CHECK(v.capacity() == 4);

      v.shrink_to_fit();
      CHECK(v.size() == 2);
      CHECK(v.capacity() == 2);    // back to inline
      CHECK(values(v) == std::vector<int>({0, 1}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("shrinks to an exact-fit heap block when still above inline size") {
    {
      Vec v;
      for (int i = 0; i < 5; ++i) {
        v.push_back(Tracked(i));   // capacity 8, size 5
      }
      CHECK(v.capacity() == 8);
      v.shrink_to_fit();
      CHECK(v.capacity() == 5);
      CHECK(values(v) == std::vector<int>({0, 1, 2, 3, 4}));
    }
    CHECK(Tracked::_live == 0);
  }
}

TEST_CASE("small_vector reserve grows capacity without changing contents", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    v.push_back(Tracked(1));
    v.push_back(Tracked(2));

    v.reserve(10);
    CHECK(v.capacity() == 10);
    CHECK(v.size() == 2);
    CHECK(values(v) == std::vector<int>({1, 2}));

    // A reserve that does not exceed capacity does nothing.
    const Tracked *data = v.data();
    v.reserve(5);
    CHECK(v.capacity() == 10);
    CHECK(v.data() == data);
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector resize grows and shrinks", "[dtoolutil]") {
  Tracked::reset();

  SECTION("shrinking destructs the tail") {
    {
      Vec v;
      for (int i = 0; i < 5; ++i) {
        v.push_back(Tracked(i));
      }
      v.resize(2);
      CHECK(v.size() == 2);
      CHECK(values(v) == std::vector<int>({0, 1}));
      CHECK(Tracked::_live == 2);
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("growing by one appends the fill value") {
    {
      Vec v;
      v.push_back(Tracked(1));
      v.resize(2, Tracked(8));
      CHECK(values(v) == std::vector<int>({1, 8}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("growing by several fills every new slot with the value") {
    {
      Vec v;
      v.resize(4, Tracked(8));
      // Every appended element should equal the requested fill value.
      CHECK(values(v) == std::vector<int>({8, 8, 8, 8}));
    }
    CHECK(Tracked::_live == 0);
  }
}

TEST_CASE("small_vector insert shifts elements right", "[dtoolutil]") {
  Tracked::reset();

  SECTION("insert in the middle without reallocation") {
    {
      Vec v;
      v.push_back(Tracked(1));
      v.push_back(Tracked(3));
      v.reserve(4);
      Vec::iterator it = v.insert(v.begin() + 1, Tracked(2));
      CHECK(it->_value == 2);
      CHECK(values(v) == std::vector<int>({1, 2, 3}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("insert at begin, mid, and end") {
    {
      Vec v;
      v.push_back(Tracked(2));
      v.insert(v.begin(), Tracked(1));          // front
      v.insert(v.end(), Tracked(4));            // back
      v.insert(v.begin() + 2, Tracked(3));      // middle
      CHECK(values(v) == std::vector<int>({1, 2, 3, 4}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("insert that forces a reallocation") {
    {
      Vec v;
      v.push_back(Tracked(1));
      v.push_back(Tracked(2));   // full inline (capacity 2)
      Vec::iterator it = v.insert(v.begin(), Tracked(0));
      CHECK(it->_value == 0);
      CHECK(v.capacity() >= 3);
      CHECK(values(v) == std::vector<int>({0, 1, 2}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("insert a moved rvalue") {
    {
      Vec v;
      v.push_back(Tracked(1));
      Tracked t(2);
      v.insert(v.end(), std::move(t));
      CHECK(t._moved_from);
      CHECK(values(v) == std::vector<int>({1, 2}));
    }
    CHECK(Tracked::_live == 0);
  }
}

TEST_CASE("small_vector erase removes elements and returns the next", "[dtoolutil]") {
  Tracked::reset();

  SECTION("erase a single middle element") {
    {
      Vec v;
      for (int i = 0; i < 4; ++i) {
        v.push_back(Tracked(i));
      }
      Vec::iterator it = v.erase(v.begin() + 1);
      CHECK(it->_value == 2);      // element after the erased one
      CHECK(values(v) == std::vector<int>({0, 2, 3}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("erase the last element") {
    {
      Vec v;
      for (int i = 0; i < 3; ++i) {
        v.push_back(Tracked(i));
      }
      Vec::iterator it = v.erase(v.end() - 1);
      CHECK(it == v.end());
      CHECK(values(v) == std::vector<int>({0, 1}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("erase from the front of large storage relocates into inline") {
    {
      Vec v;
      for (int i = 0; i < 4; ++i) {
        v.push_back(Tracked(i));  // large, capacity 4
      }
      // Erasing from the front leaves size 3 (> inline 2): stays on the heap.
      Vec::iterator it = v.erase(v.begin());
      CHECK(values(v) == std::vector<int>({1, 2, 3}));
      CHECK(v.capacity() == 4);
      CHECK(it == v.begin());
      CHECK(it->_value == 1);

      // Erasing again leaves size 2 (== inline): the vector moves its contents
      // back into inline storage and frees the heap block.
      v.erase(v.begin());
      CHECK(values(v) == std::vector<int>({2, 3}));
      CHECK(v.capacity() == 2);
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("erase an empty range is a no-op") {
    {
      Vec v;
      v.push_back(Tracked(1));
      v.push_back(Tracked(2));
      Vec::iterator it = v.erase(v.begin() + 1, v.begin() + 1);
      CHECK(it == v.begin() + 1);
      CHECK(values(v) == std::vector<int>({1, 2}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("erase a range in the middle") {
    {
      Vec v;
      for (int i = 0; i < 5; ++i) {
        v.push_back(Tracked(i));
      }
      Vec::iterator it = v.erase(v.begin() + 1, v.begin() + 4);
      CHECK(values(v) == std::vector<int>({0, 4}));
      CHECK(it == v.begin() + 1);   // element following the removed range
      CHECK(it->_value == 4);
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("erase a leading range of large storage relocates into inline") {
    {
      Vec v;
      for (int i = 0; i < 5; ++i) {
        v.push_back(Tracked(i));  // capacity 8, size 5
      }
      v.erase(v.begin(), v.begin() + 3);   // size 2 == inline
      CHECK(values(v) == std::vector<int>({3, 4}));
      CHECK(v.capacity() == 2);
    }
    CHECK(Tracked::_live == 0);
  }
}

TEST_CASE("small_vector initializer_list construction", "[dtoolutil]") {
  Tracked::reset();

  SECTION("list that fits inline") {
    {
      Vec v({Tracked(1), Tracked(2)});
      CHECK(v.size() == 2);
      CHECK(v.capacity() == 2);
      CHECK(values(v) == std::vector<int>({1, 2}));
    }
    CHECK(Tracked::_live == 0);
  }

  SECTION("list that overflows to the heap") {
    {
      Vec v({Tracked(1), Tracked(2), Tracked(3), Tracked(4)});
      CHECK(v.size() == 4);
      CHECK(v.capacity() >= 4);
      CHECK(values(v) == std::vector<int>({1, 2, 3, 4}));
    }
    CHECK(Tracked::_live == 0);
  }
}

TEST_CASE("small_vector pop_back destructs the last element", "[dtoolutil]") {
  Tracked::reset();
  {
    Vec v;
    v.push_back(Tracked(1));
    v.push_back(Tracked(2));
    CHECK(Tracked::_live == 2);
    v.pop_back();
    CHECK(v.size() == 1);
    CHECK(Tracked::_live == 1);
    CHECK(v.back()._value == 1);
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector supports move-only element types", "[dtoolutil]") {
  // A move-only element exercises the paths that must never fall back to a
  // copy: push_back(T&&), emplace_back, resize(size_type), the move
  // constructor and the reallocation-relocation loop.
  typedef small_vector<std::unique_ptr<int>, 2> PtrVec;
  PtrVec v;
  v.emplace_back(new int(1));
  v.push_back(std::unique_ptr<int>(new int(2)));
  v.emplace_back(new int(3));   // forces reallocation, relocating by move
  v.emplace_back(new int(4));

  REQUIRE(v.size() == 4);
  CHECK(*v[0] == 1);
  CHECK(*v[1] == 2);
  CHECK(*v[2] == 3);
  CHECK(*v[3] == 4);

  PtrVec moved(std::move(v));
  CHECK(moved.size() == 4);
  CHECK(*moved[3] == 4);
  CHECK(v.size() == 0);

  moved.erase(moved.begin());
  REQUIRE(moved.size() == 3);
  CHECK(*moved[0] == 2);

  moved.resize(5);
  REQUIRE(moved.size() == 5);
  CHECK(*moved[0] == 2);
  CHECK(*moved[2] == 4);
  CHECK(moved[3] == nullptr);
  CHECK(moved[4] == nullptr);

  moved.resize(1);
  REQUIRE(moved.size() == 1);
  CHECK(*moved[0] == 2);
}

TEST_CASE("small_vector with zero inline capacity behaves as a plain vector", "[dtoolutil]") {
  Tracked::reset();
  {
    small_vector<Tracked, 0> v;
    CHECK(v.capacity() == 0);
    CHECK(v.empty());

    v.push_back(Tracked(1));   // must allocate immediately
    CHECK(v.size() == 1);
    CHECK(v.capacity() >= 1);
    v.push_back(Tracked(2));
    v.push_back(Tracked(3));
    CHECK(v[0]._value == 1);
    CHECK(v[2]._value == 3);
  }
  CHECK(Tracked::_live == 0);
}

TEST_CASE("small_vector constructs from an iterator range", "[dtoolutil]") {
  const uint32_t values[6] = {1, 2, 3, 4, 5, 6};

  // Within the inline capacity.
  small_vector<uint32_t, 4> small(values, values + 3);
  REQUIRE(small.size() == 3);
  CHECK(small[0] == 1);
  CHECK(small[2] == 3);

  // Spilling to the heap.
  small_vector<uint32_t, 4> large(values, values + 6);
  REQUIRE(large.size() == 6);
  for (uint32_t i = 0; i < 6; ++i) {
    CHECK(large[i] == i + 1);
  }

  // Empty range.
  small_vector<uint32_t, 4> empty(values, values);
  CHECK(empty.empty());
}

TEST_CASE("small_vector assign replaces the contents", "[dtoolutil]") {
  small_vector<uint32_t, 4> vec {1, 2, 3};

  const uint32_t values[6] = {9, 8, 7, 6, 5, 4};
  vec.assign(values, values + 6);
  REQUIRE(vec.size() == 6);
  CHECK(vec[0] == 9);
  CHECK(vec[5] == 4);

  // Assigning a shorter range shrinks it again.
  vec.assign(values, values + 2);
  REQUIRE(vec.size() == 2);
  CHECK(vec[0] == 9);
  CHECK(vec[1] == 8);

  // Assigning from another small_vector's range.
  small_vector<uint32_t, 4> other {11, 22, 33};
  vec.assign(other.begin() + 1, other.end());
  REQUIRE(vec.size() == 2);
  CHECK(vec[0] == 22);
  CHECK(vec[1] == 33);
}

TEST_CASE("small_vector inserts an iterator range", "[dtoolutil]") {
  small_vector<uint32_t, 4> vec {1, 2, 7, 8};

  // Insert in the middle, forcing a spill to the heap.
  const uint32_t mid[4] = {3, 4, 5, 6};
  auto it = vec.insert(vec.begin() + 2, mid, mid + 4);
  CHECK(*it == 3);
  REQUIRE(vec.size() == 8);
  for (uint32_t i = 0; i < 8; ++i) {
    CHECK(vec[i] == i + 1);
  }

  // Insert at the end.
  const uint32_t tail[2] = {9, 10};
  vec.insert(vec.end(), tail, tail + 2);
  REQUIRE(vec.size() == 10);
  CHECK(vec[8] == 9);
  CHECK(vec[9] == 10);

  // Insert at the beginning.
  const uint32_t head[1] = {0};
  vec.insert(vec.begin(), head, head + 1);
  REQUIRE(vec.size() == 11);
  for (uint32_t i = 0; i < 11; ++i) {
    CHECK(vec[i] == i);
  }

  // An empty range is a no-op.
  it = vec.insert(vec.begin() + 5, head, head);
  CHECK(it == vec.begin() + 5);
  REQUIRE(vec.size() == 11);

  // A single-element range insert without a spill.
  small_vector<uint32_t, 4> small {1, 3};
  const uint32_t two[1] = {2};
  small.insert(small.begin() + 1, two, two + 1);
  REQUIRE(small.size() == 3);
  CHECK(small[0] == 1);
  CHECK(small[1] == 2);
  CHECK(small[2] == 3);
}

TEST_CASE("small_vector inserts a range aliasing itself", "[dtoolutil]") {
  // Without a spill, where the shift would otherwise overwrite the source.
  small_vector<std::string, 8> vec {"alpha", "bravo", "charlie", "delta"};
  auto it = vec.insert(vec.begin() + 1, vec.begin() + 2, vec.begin() + 4);
  CHECK(*it == "charlie");
  REQUIRE(vec.size() == 6);
  CHECK(vec[0] == "alpha");
  CHECK(vec[1] == "charlie");
  CHECK(vec[2] == "delta");
  CHECK(vec[3] == "bravo");
  CHECK(vec[4] == "charlie");
  CHECK(vec[5] == "delta");

  // With a spill to the heap, which would otherwise free the source.
  small_vector<std::string, 4> spill {"one", "two", "three", "four"};
  it = spill.insert(spill.begin() + 2, spill.begin() + 1, spill.begin() + 3);
  CHECK(*it == "two");
  REQUIRE(spill.size() == 6);
  CHECK(spill[0] == "one");
  CHECK(spill[1] == "two");
  CHECK(spill[2] == "two");
  CHECK(spill[3] == "three");
  CHECK(spill[4] == "three");
  CHECK(spill[5] == "four");
}

TEST_CASE("small_vector supports single-pass input iterators", "[dtoolutil]") {
  // std::istream_iterator is a genuine single-pass input iterator; the range
  // may only be traversed once.
  std::istringstream ctor_stream("1 2 3 4 5");
  std::istream_iterator<uint32_t> ctor_begin(ctor_stream), stream_end;
  small_vector<uint32_t, 4> vec(ctor_begin, stream_end);
  REQUIRE(vec.size() == 5);
  for (uint32_t i = 0; i < 5; ++i) {
    CHECK(vec[i] == i + 1);
  }

  std::istringstream assign_stream("9 8 7");
  vec.assign(std::istream_iterator<uint32_t>(assign_stream), stream_end);
  REQUIRE(vec.size() == 3);
  CHECK(vec[0] == 9);
  CHECK(vec[1] == 8);
  CHECK(vec[2] == 7);

  std::istringstream insert_stream("100 200");
  auto it = vec.insert(vec.begin() + 1,
                       std::istream_iterator<uint32_t>(insert_stream),
                       stream_end);
  CHECK(*it == 100);
  REQUIRE(vec.size() == 5);
  CHECK(vec[0] == 9);
  CHECK(vec[1] == 100);
  CHECK(vec[2] == 200);
  CHECK(vec[3] == 8);
  CHECK(vec[4] == 7);

  std::istringstream empty_stream("");
  it = vec.insert(vec.begin() + 2,
                  std::istream_iterator<uint32_t>(empty_stream),
                  stream_end);
  CHECK(it == vec.begin() + 2);
  REQUIRE(vec.size() == 5);
}

TEST_CASE("small_vector empty range insert preserves non-trivial elements", "[dtoolutil]") {
  small_vector<std::string, 4> vec {"alpha", "bravo", "charlie"};
  const std::string empty[1] = {"unused"};

  auto it = vec.insert(vec.begin() + 1, empty, empty);
  CHECK(it == vec.begin() + 1);
  REQUIRE(vec.size() == 3);
  CHECK(vec[0] == "alpha");
  CHECK(vec[1] == "bravo");
  CHECK(vec[2] == "charlie");
}
