/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineStream.I
 * @author drose
 * @date 2000-02-26
 */

/**
 *
 */
INLINE LineStream::
LineStream() : std::ostream(&_lsb) {
}

/**
 * Returns true if there is at least one line of text (or even a partial line)
 * available in the LineStream object.  If this returns true, the line may
 * then be retrieved via get_line().
 */
INLINE bool LineStream::
is_text_available() const {
  return _lsb.is_text_available();
}

/**
 * Extracts and returns the next line (or partial line) of text available in
 * the LineStream object.  Once the line has been extracted, you may call
 * has_newline() to determine whether or not there was an explicit newline
 * character written following this line.
 */
INLINE std::string LineStream::
get_line() {
  return _lsb.get_line();
}

/**
 * Returns true if the line of text most recently returned by get_line() was
 * written out with a terminating newline, or false if a newline character has
 * not yet been written to the LineStream.
 */
INLINE bool LineStream::
has_newline() const {
  return _lsb.has_newline();
}
