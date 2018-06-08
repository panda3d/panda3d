/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file programBase.I
 * @author drose
 * @date 2000-06-28
 */

/**
 * Formats the indicated text to stderr with the known _terminal_width.
 */
INLINE void ProgramBase::
show_text(const std::string &text) {
  show_text("", 0, text);
}
