// Filename: cppToken.h
// Created by:  drose (22Oct99)
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
