// Filename: chatHelpers.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "chatHelpers.h"

////////////////////////////////////////////////////////////////////
//     Function: add_line_wraps 
//  Description: Take an input string and format it so that lines
//               will wrap before they reach line_length (unless
//               there is no whitespace).
////////////////////////////////////////////////////////////////////
string add_line_wraps(const string& str, int line_length) {
  string new_str;
  string old_str = str;

  if (str.length() <= line_length) 
    return old_str;

  int length_so_far = 0;
  string::size_type ws;

  do {
    ws = old_str.find(' ');
    string sub_str;
    if (ws != string::npos) {
      sub_str = old_str.substr(0, ws+1);
      old_str = old_str.substr(ws+1, string::npos);
    } else
      sub_str = old_str;
    int sub_str_len = sub_str.length();
    if (length_so_far + sub_str_len > line_length) {
      new_str += '\n';
      length_so_far = 0;
    }
    new_str += sub_str;
    length_so_far += sub_str_len;
  } while (ws != string::npos);

  return new_str;
}
