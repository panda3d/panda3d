/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppCommentBlock.h
 * @author drose
 * @date 2000-08-15
 */

#ifndef CPPCOMMENTBLOCK_H
#define CPPCOMMENTBLOCK_H

#include "dtoolbase.h"

#include "cppFile.h"

#include <list>

/**
 * This represents a comment appearing in the source code.  The
 * CPPPreprocessor collects these, and saves the complete list of comments
 * encountered; it also stores a list of the comment blocks appearing before
 * each declaration.
 */
class CPPCommentBlock {
public:
  CPPFile _file;
  int _line_number;
  int _col_number;
  int _last_line;
  bool _c_style;
  std::string _comment;
};

typedef std::list<CPPCommentBlock *> CPPComments;

#endif
