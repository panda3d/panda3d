// Filename: ppSubroutine.h
// Created by:  drose (10Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPSUBROUTINE_H
#define PPSUBROUTINE_H

#include "ppremake.h"

#include <vector>
#include <map>

///////////////////////////////////////////////////////////////////
//       Class : PPSubroutine
// Description : This represents a named subroutine defined via the
//               #defsub .. #end sequence that may be invoked at any
//               time via #call.  All subroutine definitions are
//               global.
////////////////////////////////////////////////////////////////////
class PPSubroutine {
public:
  vector<string> _formals;
  vector<string> _lines;

public:
  static void define_sub(const string &name, PPSubroutine *sub);
  static const PPSubroutine *get_sub(const string &name);

  static void define_func(const string &name, PPSubroutine *sub);
  static const PPSubroutine *get_func(const string &name);

  typedef map<string, PPSubroutine *> Subroutines;
  static Subroutines _subroutines;
  static Subroutines _functions;
};

#endif

