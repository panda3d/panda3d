// Filename: tokenBoard.h
// Created by:  mike (09Jan97)
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

#ifndef TOKENBOARD_H
#define TOKENBOARD_H

#include "pandabase.h"
#include "plist.h"
#include "circBuffer.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//      Struct : TokenMatch
// Description : This is an STL predicate function object that is used
//               to find a particular id in a list of token pointers.
//               It returns true when the token's id matches the
//               numeric id supplied to the constructor.
////////////////////////////////////////////////////////////////////
template<class TokenType>
class TokenMatch {
public:
  TokenMatch(int id) {
    _want_id = id;
  }
  bool operator()(PT(TokenType) tok) const {
    return (int)tok->_id == _want_id;
  }
  int _want_id;
};

const int MAX_TOKENBOARD_REQUESTS = 100;

////////////////////////////////////////////////////////////////////
//       Class : TokenBoard
// Description :
////////////////////////////////////////////////////////////////////
template<class TokenType>
class TokenBoard {
public:
  bool is_done_token(int id);
  PT(TokenType) get_done_token(int id);

  // waiting holds the list of requests sent to the DBASE process, not
  // yet handled.
  CircBuffer<PT(TokenType), MAX_TOKENBOARD_REQUESTS> _waiting;

  // done holds the list of requests handled by the DBASE process, but
  // not yet discovered by APP.  Probably this queue will only have
  // one item at a time on it.
  CircBuffer<PT(TokenType), MAX_TOKENBOARD_REQUESTS> _done;

  // really_done holds the requests extracted from done.  These are
  // extracted into the local list so we can safely search for and
  // remove a particular token from the middle of the list (we can
  // only remove from the head of a circular buffer).
  plist< PT(TokenType) > _really_done;
};

#include "tokenBoard.I"

#endif
