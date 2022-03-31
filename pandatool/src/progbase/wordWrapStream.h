/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wordWrapStream.h
 * @author drose
 * @date 2000-06-28
 */

#ifndef WORDWRAPSTREAM_H
#define WORDWRAPSTREAM_H

#include "pandatoolbase.h"

#include "wordWrapStreamBuf.h"

/**
 * A special ostream that formats all of its output through
 * ProgramBase::show_text().  This allows the program to easily word-wrap its
 * output messages to fit the terminal width.
 *
 * By convention (inherited from show_text), a newline written to the
 * WordWrapStream indicates a paragraph break, and is generally printed as a
 * blank line.  To force a line break without a paragraph break, use '\r'.
 */
class WordWrapStream : public std::ostream {
public:
  WordWrapStream(ProgramBase *program);

private:
  WordWrapStreamBuf _lsb;
};

#endif
