// Filename: cppCommentBlock.h
// Created by:  drose (15Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CPPCOMMENTBLOCK_H
#define CPPCOMMENTBLOCK_H

#include <dtoolbase.h>

#include "cppFile.h"

#include <list>

///////////////////////////////////////////////////////////////////
// 	 Class : CPPCommentBlock
// Description : This represents a comment appearing in the source
//               code.  The CPPPreprocessor collects these, and saves
//               the complete list of comments encountered; it also
//               stores a list of the comment blocks appearing before
//               each declaration.
////////////////////////////////////////////////////////////////////
class CPPCommentBlock {
public:
  CPPFile _file;
  int _line_number;
  int _col_number;
  int _last_line;
  bool _c_style;
  string _comment;
};

typedef list<CPPCommentBlock *> CPPComments;

#endif
