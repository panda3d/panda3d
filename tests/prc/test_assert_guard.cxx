/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_assert_guard.cxx
 * @author rdb
 * @date 2026-07-19
 */

#include "pnotify.h"

#include "catch_amalgamated.hpp"

/**
 * Routes a Panda assertion failure (nassertr and friends) into Catch2 as a
 * test failure at the point where it fired, rather than letting the test
 * pass with only a line on the notify output.  Returning true preserves the
 * normal no-abort behavior: the asserting function bails out with its
 * default return value.
 */
static bool
catch2_assert_handler(const char *expression, int line, const char *source_file) {
  FAIL_CHECK("Assertion failed: " << expression << " at line " << line << " of " << source_file);
  return true;
}

/**
 * Installs the assert handler for the duration of each test case, so that a
 * test that trips an assertion fails even if all of its CHECKs pass.  A test
 * that intentionally provokes an assertion can temporarily install its own
 * handler; this listener reinstates the guard for the next test case.
 */
class PandaAssertListener final : public Catch::EventListenerBase {
public:
  using Catch::EventListenerBase::EventListenerBase;

  void testCaseStarting(const Catch::TestCaseInfo &) override {
    Notify::ptr()->set_assert_handler(&catch2_assert_handler);
  }

  void testCaseEnded(const Catch::TestCaseStats &) override {
    Notify::ptr()->clear_assert_handler();
    Notify::ptr()->clear_assert_failed();
  }
};

CATCH_REGISTER_LISTENER(PandaAssertListener)
