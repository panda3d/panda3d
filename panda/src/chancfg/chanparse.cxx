// Filename: chanparse.cxx
// Created by:  cary (02Feb99)
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

#include "chanparse.h"
#include <notify.h>

const int ChanFileEOF = -1;

static bool file_done;

INLINE std::string ChanReadLine(istream& is) {
  if (is.eof() || is.fail())
#ifdef BROKEN_EXCEPTIONS
    {
      file_done = true;
      return "";
    }
#else
    throw ChanFileEOF;  // all done
#endif /* BROKEN_EXCEPTIONS */
  std::string S;
  std::getline(is, S);
  size_t i = S.find_first_not_of(" \t\f\r\n");
  if ((i == std::string::npos) || (S[i] == ';'))
    return ChanReadLine(is);  // all white space or comment
  if (i != 0)
    S.erase(0, i);  // remove leading white space
  i = S.find_first_of(";");
  if (i != std::string::npos)
    S.erase(i, std::string::npos);  // remove trailing comment
  return S;
}

int ChanMatchingParen(std::string S) {
  int i = 1, j = 1;
  std::string::iterator k = S.begin();

  ++k;
  while ((k != S.end()) && (i != 0)) {
    switch (*k) {
    case '(':
      ++i;
      break;
    case ')':
      --i;
      break;
    }
    ++j;
    ++k;
  }
  if ((k == S.end()) && (i != 0))
    j = -1;
  return j;
}

void ChanParse(istream& is, ChanParseFunctor* p) {
  file_done = false;

  std::string S;
#ifdef BROKEN_EXCEPTIONS
  S = ChanReadLine(is);
  if (file_done) {
    nout << "No data in file to parse" << endl;
    return;
  }
#else
  try {
    S = ChanReadLine(is);
  }
  catch (...) {
    nout << "No data in file to parse" << endl;
    return;  // comment that there was no data in file to parse
  }
#endif /* BROKEN_EXCEPTIONS */

  // now the main loop
#ifdef BROKEN_EXCEPTIONS
  {
    int i;
    while (true) {
      if ((!S.empty()) && (S[0] != '(')) {
        // error, should have leading paren
        nout << "error, no leading paren in '" << S << "'" << endl;
        S.erase(0, S.find_first_of("("));
      }
      while ((S.empty())||(i = ChanMatchingParen(S)) == -1) {
        S += " ";
        S += ChanReadLine(is);
        if (file_done) {
          if (!S.empty() && (!(S == " ")))
            // error, trailing text usually indicates an error in the file
            nout << "error, trailing text in file S = '" << S << "'" << endl;
          return;
        }
        ChanEatFrontWhite(S);
      }
      (*p)(S.substr(1, i-2));
      S.erase(0, i);
      ChanEatFrontWhite(S);
    }
  }
#else
  try {
    int i;
    while (true) {
      if ((!S.empty()) && (S[0] != '(')) {
        // error, should have leading paren
        nout << "error, no leading paren in '" << S << "'" << endl;
        S.erase(0, S.find_first_of("("));
      }
      while ((S.empty())||(i = ChanMatchingParen(S)) == -1) {
        S += " ";
        S += ChanReadLine(is);
        ChanEatFrontWhite(S);
      }
      (*p)(S.substr(1, i-2));
      S.erase(0, i);
      ChanEatFrontWhite(S);
    }
  }
  catch (...) {
    if (!S.empty() && (!(S == " ")))
      // error, trailing text usually indicates an error in the file
      nout << "error, trailing text in file S = '" << S << "'" << endl;
    return;
  }
#endif /* BROKEN_EXCEPTIONS */
}

ChanParseFunctor::~ChanParseFunctor() {
  return;
}

void ChanParseFunctor::operator()(std::string) {
  // error, should never be in here
  return;
}
