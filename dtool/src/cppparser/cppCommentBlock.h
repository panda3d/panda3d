// Filename: cppCommentBlock.h
// Created by:  drose (15Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CPPCOMMENTBLOCK_H
#define CPPCOMMENTBLOCK_H

#include "dtoolbase.h"

#include "cppFile.h"

#include <list>

///////////////////////////////////////////////////////////////////
//       Class : CPPCommentBlock
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
