// Filename: cppToken.h
// Created by:  drose (22Oct99)
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

#ifndef CPPTOKEN_H
#define CPPTOKEN_H

#include "dtoolbase.h"

#include "cppBisonDefs.h"

///////////////////////////////////////////////////////////////////
//       Class : CPPToken
// Description :
////////////////////////////////////////////////////////////////////
class CPPToken {
public:
  CPPToken(int token, int line_number = 0, int col_number = 0,
           const CPPFile &file = CPPFile(""),
           const string &str = string(),
           const YYSTYPE &lval = YYSTYPE());
  CPPToken(const CPPToken &copy);
  void operator = (const CPPToken &copy);

  static CPPToken eof();
  bool is_eof() const;

  void output(ostream &out) const;

  int _token;
  YYSTYPE _lval;
  YYLTYPE _lloc;
};

inline ostream &operator << (ostream &out, const CPPToken &token) {
  token.output(out);
  return out;
}


#endif
