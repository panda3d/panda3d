// Filename: cppTypeParser.h
// Created by:  drose (14Dec99)
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

#ifndef CPPTYPEPARSER_H
#define CPPTYPEPARSER_H

#include "dtoolbase.h"

#include "cppPreprocessor.h"

class CPPType;
class CPPScope;

///////////////////////////////////////////////////////////////////
//       Class : CPPTypeParser
// Description :
////////////////////////////////////////////////////////////////////
class CPPTypeParser : public CPPPreprocessor {
public:
  CPPTypeParser(CPPScope *current_scope, CPPScope *global_scope);
  ~CPPTypeParser();

  bool parse_type(const string &type);
  bool parse_type(const string &type, const CPPPreprocessor &filepos);

  void output(ostream &out) const;

  CPPScope *_current_scope;
  CPPScope *_global_scope;
  CPPType *_type;
};

inline ostream &
operator << (ostream &out, const CPPTypeParser &ep) {
  ep.output(out);
  return out;
}

#endif


