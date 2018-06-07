/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnotify.I
 * @author drose
 * @date 2000-02-28
 */

/**
 * Returns true if an assertion test has failed (and not been ignored) since
 * the last call to clear_assert_failed().
 *
 * When an assertion test fails, the assert handler may decide either to
 * abort, return, or ignore the assertion.  Naturally, if it decides to abort,
 * this flag is irrelevant.  If it chooses to ignore the assertion, the flag
 * is not set.  However, if the assert handler chooses to return out of the
 * function (the normal case), it will also set this flag to indicate that an
 * assertion failure has occurred.
 *
 * This will also be the behavior in the absence of a user-defined assert
 * handler.
 */
INLINE bool Notify::
has_assert_failed() const {
  return _assert_failed;
}

/**
 * Returns the error message that corresponds to the assertion that most
 * recently failed.
 */
INLINE const std::string &Notify::
get_assert_error_message() const {
  return _assert_error_message;
}

/**
 * Resets the assert_failed flag that is set whenever an assertion test fails.
 * See has_assert_failed().
 */
INLINE void Notify::
clear_assert_failed() {
  _assert_failed = false;
}
