// Filename: tokenBoard.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97,98  
// Walt Disney Imagineering, Inc.
//
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TokenBoard::is_done_token() 
//       Access: Public
//  Description: Returns true if the indicated token id is on the 
//		 done queue, false otherwise.
////////////////////////////////////////////////////////////////////
template<class TokenType>
bool TokenBoard<TokenType>::
is_done_token(int id) {
  // First, empty the done list, copying them to really_done.  We have
  // to do this since we can only examine tokens on the head of the
  // done list, and the token we're looking for might not be at the
  // head.
  while (!_done.is_empty()) {
    _really_done.push_back(_done.extract());
  }

  // Now we can search really_done for our desired id.
  list< PT(TokenType) >::iterator found;
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
  while (!_done.is_empty()) {
    _really_done.push_back(_done.extract());
  }

  // Now we can search really_done for our desired id.
  list< PT(TokenType) >::iterator found;
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
