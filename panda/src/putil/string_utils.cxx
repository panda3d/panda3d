// Filename: string_utils.cxx
// Created by:  drose (18Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "string_utils.h"

#include <ctype.h>

// Case-insensitive string comparison, from Stroustrup's C++ third edition.
// Works like strcmp().
int
cmp_nocase(const string &s, const string &s2) {
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end()) {
    if (toupper(*p) != toupper(*p2)) {
      return (toupper(*p) < toupper(*p2)) ? -1 : 1;
    }
    ++p;
    ++p2;
  }
  
  return (s2.size() == s.size()) ? 0 :
    (s.size() < s2.size()) ? -1 : 1;  // size is unsigned
}

INLINE int 
toupper_uh(int ch) {
  return (ch == '_') ? '-' : toupper(ch);
}


int
cmp_nocase_uh(const string &s, const string &s2) {
  string::const_iterator p = s.begin();
  string::const_iterator p2 = s2.begin();

  while (p != s.end() && p2 != s2.end()) {
    if (toupper_uh(*p) != toupper_uh(*p2)) {
      return (toupper_uh(*p) < toupper_uh(*p2)) ? -1 : 1;
    }
    ++p;
    ++p2;
  }
  
  return (s2.size() == s.size()) ? 0 :
    (s.size() < s2.size()) ? -1 : 1;  // size is unsigned
}



string
downcase(const string &s) {
  string result;
  string::const_iterator p;
  for (p = s.begin(); p != s.end(); ++p) {
    result += tolower(*p);
  }
  return result;
}

