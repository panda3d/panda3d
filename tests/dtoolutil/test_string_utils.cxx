/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_string_utils.cxx
 * @author rdb
 * @date 2026-07-03
 */

#include "string_utils.h"
#include "vector_string.h"

#include "catch_amalgamated.hpp"

#include <cstdint>

using std::string;
using std::wstring;

TEST_CASE("cmp_nocase compares case-insensitively", "[dtoolutil]") {
  CHECK(cmp_nocase("panda3d", "panda3d") == 0);
  CHECK(cmp_nocase("abc", "ABC") == 0);
  CHECK(cmp_nocase("", "") == 0);
  CHECK(cmp_nocase("Panda3D", "panda3d") == 0);

  // Ordering is reported as the sign of the result, like strcmp.
  CHECK(cmp_nocase("abc", "abd") < 0);
  CHECK(cmp_nocase("abd", "abc") > 0);

  // A prefix sorts before the longer string.
  CHECK(cmp_nocase("ab", "abc") < 0);
  CHECK(cmp_nocase("abc", "ab") > 0);
}

TEST_CASE("cmp_nocase_uh treats hyphen and underscore as equal", "[dtoolutil]") {
  CHECK(cmp_nocase_uh("a-b", "a-b") == 0);
  CHECK(cmp_nocase_uh("a_b", "a-b") == 0);
  CHECK(cmp_nocase_uh("FOO_BAR", "foo-bar") == 0);
  CHECK(cmp_nocase_uh("a_b", "a_b") == 0);

  // Still order-sensitive on genuinely different characters.
  CHECK(cmp_nocase_uh("a_b", "a_c") < 0);
}

TEST_CASE("downcase and upcase convert ASCII letters", "[dtoolutil]") {
  CHECK(downcase("Hello, World! 123") == "hello, world! 123");
  CHECK(upcase("Hello, World! 123") == "HELLO, WORLD! 123");
  CHECK(downcase("") == "");
  CHECK(upcase("") == "");
}

TEST_CASE("extract_words splits on whitespace", "[dtoolutil]") {
  SECTION("simple words") {
    vector_string words;
    int n = extract_words("the quick brown fox", words);
    CHECK(n == 4);
    CHECK(words == vector_string({"the", "quick", "brown", "fox"}));
  }

  SECTION("leading, trailing and repeated whitespace are ignored") {
    vector_string words;
    int n = extract_words("  \t one\ttwo  \n three  ", words);
    CHECK(n == 3);
    CHECK(words == vector_string({"one", "two", "three"}));
  }

  SECTION("empty and whitespace-only strings yield no words") {
    vector_string words;
    CHECK(extract_words("", words) == 0);
    CHECK(extract_words("   \t\n ", words) == 0);
    CHECK(words.empty());
  }

  SECTION("results are appended to the existing vector") {
    vector_string words;
    words.push_back("existing");
    extract_words("a b", words);
    CHECK(words == vector_string({"existing", "a", "b"}));
  }
}

TEST_CASE("tokenize splits on delimiter characters", "[dtoolutil]") {
  SECTION("basic split") {
    vector_string words;
    tokenize("a,b,c", words, ",");
    CHECK(words == vector_string({"a", "b", "c"}));
  }

  SECTION("any character in the delimiter set splits") {
    vector_string words;
    tokenize("a,b;c", words, ",;");
    CHECK(words == vector_string({"a", "b", "c"}));
  }

  SECTION("repeated delimiters produce empty tokens") {
    vector_string words;
    tokenize("a,,b", words, ",");
    CHECK(words == vector_string({"a", "", "b"}));
  }

  SECTION("a leading delimiter produces a leading empty token") {
    vector_string words;
    tokenize(",a", words, ",");
    CHECK(words == vector_string({"", "a"}));
  }

  SECTION("discard_repeated_delimiters collapses runs") {
    vector_string words;
    tokenize("a,,,b", words, ",", true);
    CHECK(words == vector_string({"a", "b"}));
  }

  SECTION("a string with no delimiter is a single token") {
    vector_string words;
    tokenize("abc", words, ",");
    CHECK(words == vector_string({"abc"}));
  }
}

TEST_CASE("trim removes surrounding whitespace", "[dtoolutil]") {
  CHECK(trim("  hello  ") == "hello");
  CHECK(trim_left("  hello  ") == "hello  ");
  CHECK(trim_right("  hello  ") == "  hello");

  // Interior whitespace is preserved.
  CHECK(trim("  a b  ") == "a b");

  // Tabs and newlines count as whitespace.
  CHECK(trim("\t\n hi \n\t") == "hi");

  // Empty and all-whitespace strings collapse to empty.
  CHECK(trim("") == "");
  CHECK(trim("     ") == "");
  CHECK(trim_left("     ") == "");
  CHECK(trim_right("     ") == "");
}

TEST_CASE("string_to_int parses leading integers and reports the tail", "[dtoolutil]") {
  SECTION("tail flavor") {
    string tail;
    CHECK(string_to_int("123abc", tail) == 123);
    CHECK(tail == "abc");

    // A negative value with no trailing garbage.
    CHECK(string_to_int("-42", tail) == -42);
    CHECK(tail.empty());

    // Leading whitespace is skipped by strtol.
    CHECK(string_to_int("   17", tail) == 17);
    CHECK(tail.empty());

    // No integer at all: the whole string is the tail.
    CHECK(string_to_int("xyz", tail) == 0);
    CHECK(tail == "xyz");
  }

  SECTION("bool flavor is true only for a clean parse") {
    int result = -999;
    CHECK(string_to_int("256", result));
    CHECK(result == 256);

    CHECK_FALSE(string_to_int("256 ", result));   // trailing space
    CHECK_FALSE(string_to_int("12.5", result));    // trailing ".5"
    CHECK_FALSE(string_to_int("abc", result));     // not a number
  }
}

TEST_CASE("string_to_double parses floating-point values", "[dtoolutil]") {
  SECTION("tail flavor") {
    string tail;
    CHECK(string_to_double("3.5rest", tail) == Catch::Approx(3.5));
    CHECK(tail == "rest");
  }

  SECTION("bool flavor") {
    double d = 0.0;
    CHECK(string_to_double("2.5", d));
    CHECK(d == Catch::Approx(2.5));

    CHECK(string_to_double("1e3", d));
    CHECK(d == Catch::Approx(1000.0));

    CHECK_FALSE(string_to_double("2.5x", d));
    CHECK_FALSE(string_to_double("notanumber", d));
  }

  SECTION("float and stdfloat flavors round-trip") {
    float f = 0.0f;
    CHECK(string_to_float("1.25", f));
    CHECK(f == Catch::Approx(1.25f));

    PN_stdfloat s = 0;
    CHECK(string_to_stdfloat("-0.5", s));
    CHECK(s == Catch::Approx((PN_stdfloat)-0.5));
  }
}

TEST_CASE("format_string renders integers exactly", "[dtoolutil]") {
  CHECK(format_string(0) == "0");
  CHECK(format_string(42) == "42");
  CHECK(format_string(-42) == "-42");
  CHECK(format_string(2147483647) == "2147483647");
  // The negation is computed in unsigned space, so INT_MIN is handled.
  CHECK(format_string((int)INT32_MIN) == "-2147483648");

  CHECK(format_string(0u) == "0");
  CHECK(format_string(4294967295u) == "4294967295");

  CHECK(format_string((int64_t)0) == "0");
  CHECK(format_string((int64_t)-1) == "-1");
  CHECK(format_string((int64_t)INT64_MAX) == "9223372036854775807");
  CHECK(format_string((int64_t)INT64_MIN) == "-9223372036854775808");
}

TEST_CASE("format_string renders bools and strings", "[dtoolutil]") {
  CHECK(format_string(true) == "true");
  CHECK(format_string(false) == "false");
  CHECK(format_string(std::string_view("hello")) == "hello");
  CHECK(format_string(std::string_view("")) == "");
}

TEST_CASE("format_string round-trips floating-point via string_to_double", "[dtoolutil]") {
  // The exact text pdtoa produces is an implementation detail; what matters is
  // that it parses back to the same value.
  for (double value : {0.0, 1.0, -1.0, 3.14159265358979, 1.0e-10, 2.5e8}) {
    double back = 0.0;
    string text = format_string(value);
    REQUIRE(string_to_double(text, back));
    CHECK(back == Catch::Approx(value));
  }

  float back = 0.0f;
  string ftext = format_string(1.5f);
  REQUIRE(string_to_float(ftext, back));
  CHECK(back == Catch::Approx(1.5f));
}

TEST_CASE("wide-character trim and tokenize", "[dtoolutil]") {
  CHECK(trim(std::wstring_view(L"  wide  ")) == wstring(L"wide"));
  CHECK(trim_left(std::wstring_view(L"  wide")) == wstring(L"wide"));
  CHECK(trim_right(std::wstring_view(L"wide  ")) == wstring(L"wide"));

  pvector<wstring> words;
  tokenize(std::wstring_view(L"a,b,c"), words, std::wstring_view(L","));
  REQUIRE(words.size() == 3);
  CHECK(words[0] == wstring(L"a"));
  CHECK(words[2] == wstring(L"c"));
}
