// Filename: cppTypeParser.h
// Created by:  drose (14Dec99)
//
////////////////////////////////////////////////////////////////////

#ifndef CPPTYPEPARSER_H
#define CPPTYPEPARSER_H

#include <dtoolbase.h>

#include "cppPreprocessor.h"

class CPPType;
class CPPScope;

///////////////////////////////////////////////////////////////////
// 	 Class : CPPTypeParser
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

 
