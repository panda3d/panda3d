// Filename: wordWrapStream.h
// Created by:  drose (28Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef WORDWRAPSTREAM_H
#define WORDWRAPSTREAM_H

#include "pandatoolbase.h"

#include "wordWrapStreamBuf.h"

////////////////////////////////////////////////////////////////////
//       Class : WordWrapStream
// Description : A special ostream that formats all of its output
//               through ProgramBase::show_text().  This allows the
//               program to easily word-wrap its output messages to
//               fit the terminal width.
//
//               By convention (inherited from show_text), a newline
//               written to the WordWrapStream indicates a paragraph
//               break, and is generally printed as a blank line.  To
//               force a line break without a paragraph break, use
//               '\r'.
////////////////////////////////////////////////////////////////////
class WordWrapStream : public ostream {
public:
  WordWrapStream(ProgramBase *program);

private:
  WordWrapStreamBuf _lsb;
};

#endif
