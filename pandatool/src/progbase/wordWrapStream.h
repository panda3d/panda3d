// Filename: wordWrapStream.h
// Created by:  drose (28Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef WORDWRAPSTREAM_H
#define WORDWRAPSTREAM_H

#include <pandatoolbase.h>

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
class EXPCL_PANDA WordWrapStream : public ostream {
public:
  WordWrapStream(ProgramBase *program);

private:
  WordWrapStreamBuf _lsb;
};

#endif
