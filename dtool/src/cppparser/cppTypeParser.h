// Filename: cppTypeParser.h
// Created by:  drose (14Dec99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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


