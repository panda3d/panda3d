// Filename: chanparse.I
// Created by:  cary (06Feb99)
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

#include "notify.h"

INLINE void ChanEatFrontWhite(std::string& S) {
  size_t i = S.find_first_not_of(" \t\r\f\n");
  if ((i != std::string::npos) && (i != 0))
    S.erase(0, i);
}

INLINE void ChanCheckScoping(std::string& S) {
  if (S[0] != '(') {
    // error, unscoped text usually indicates a problem
    nout << "error, '" << S << "' does not start with a (" << endl;
    S.erase(0, S.find_first_of("("));
  }
}

INLINE void ChanDescope(std::string& S) {
  int i = ChanMatchingParen(S);
  nassertv(i != -1);
  S = S.substr(1, i-2);
  ChanEatFrontWhite(S);
}

INLINE std::string ChanReadNextWord(std::string& S) {
  int i = S.find_first_of(" \t\r\f\n");
  std::string stmp = S.substr(0, i);
  S.erase(0, i);
  ChanEatFrontWhite(S);
  return stmp;
}

INLINE int ChanReadNextInt(std::string& S) {
  int i = S.find_first_of(" \t\r\f\n");
  std::string stmp = S.substr(0, i);
  S.erase(0, i);
  ChanEatFrontWhite(S);
  istringstream st(stmp.c_str());
  st >> i;
  return i;
}

INLINE bool ChanReadNextBool(std::string& S) {
  int i = S.find_first_of(" \t\r\f\n");
  std::string stmp = S.substr(0, i);
  S.erase(0, i);
  ChanEatFrontWhite(S);
  bool ret = false;
  if ((stmp.length() != 2) || (stmp[0] != '#')) {
    // error, this isn't gonna be either #t or #f
    nout << "error, '" << stmp << "' is not either #t or #f" << endl;
    ret = false;
  }
  if (stmp[1] == 't')
    ret = true;
  else if (stmp[1] != 'f') {
    // error this isn't either #t or #f
    nout << "error, '" << stmp << "' is not either #t or #f" << endl;
    ret = false;
  }
  return ret;
}

// #f or int
INLINE int ChanReadNextIntB(std::string& S) {
  int i = S.find_first_of(" \t\r\f\n");
  std::string stmp = S.substr(0, i);
  S.erase(0, i);
  ChanEatFrontWhite(S);
  i = -1;
  if (stmp[0] == '#')
    if ((stmp.length() != 2)||(stmp[1] != 'f')) {
      // error, not #f.. #t is not allowed in this case
      nout << "error, '" << stmp << "' is not allowed, only #f or an int"
           << endl;
      i = -1;
    }
  else {
    istringstream st(stmp.c_str());
    st >> i;
  }
  return i;
}

INLINE float ChanReadNextFloat(std::string& S) {
  int i = S.find_first_of(" \t\r\f\n");
  std::string stmp = S.substr(0, i);
  S.erase(0, i);
  ChanEatFrontWhite(S);
  istringstream st(stmp.c_str());
  float ret;
  st >> ret;
  return ret;
}
