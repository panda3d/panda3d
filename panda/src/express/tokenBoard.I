// Filename: tokenBoard.I
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

////////////////////////////////////////////////////////////////////
//     Function: TokenBoard::is_done_token()
//       Access: Public
//  Description: Returns true if the indicated token id is on the
//               done queue, false otherwise.
////////////////////////////////////////////////////////////////////
template<class TokenType>
bool TokenBoard<TokenType>::
is_done_token(int id) {
  // First, empty the done list, copying them to really_done.  We have
  // to do this since we can only examine tokens on the head of the
  // done list, and the token we're looking for might not be at the
  // head.
  while (!_done.empty()) {
    _really_done.push_back(_done.front());
    _done.pop_front();
  }

  // Now we can search really_done for our desired id.
  TYPENAME plist< PT(TokenType) >::iterator found;
  found = find_if(_really_done.begin(), _really_done.end(),
                  TokenMatch<TokenType>(id));

  return (found != _really_done.end());
}

////////////////////////////////////////////////////////////////////
//     Function: TokenBoard::get_done_token
//       Access: Public
//  Description: Locates the token by the given id in the list of done
//               tokens, removes it from the list, and returns its
//               pointer (which should be deleted by the calling
//               function).  Returns NULL if the token was not on the
//               done list.
////////////////////////////////////////////////////////////////////
template<class TokenType>
PT(TokenType) TokenBoard<TokenType>::
get_done_token(int id) {
  // First, empty the done list, copying them to really_done.  We have
  // to do this since we can only examine tokens on the head of the
  // done list, and the token we're looking for might not be at the
  // head.
  while (!_done.empty()) {
    _really_done.push_back(_done.front());
    _done.pop_front();
  }

  // Now we can search really_done for our desired id.
  TYPENAME plist< PT(TokenType) >::iterator found;
  found = find_if(_really_done.begin(), _really_done.end(),
                  TokenMatch<TokenType>(id));

  if (found == _really_done.end()) {
    return NULL;
  } else {
    PT(TokenType) tok = *found;
    _really_done.erase(found);
    return tok;
  }
}
