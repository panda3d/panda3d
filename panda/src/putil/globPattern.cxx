// Filename: globPattern.cxx
// Created by:  drose (30May00)
// 
////////////////////////////////////////////////////////////////////

#include "globPattern.h"

////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::matches_substr
//       Access: Private
//  Description: The recursive implementation of matches().  This
//               returns true if the pattern substring [pi, pend)
//               matches the candidate substring [ci, cend), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GlobPattern::
matches_substr(string::const_iterator pi, string::const_iterator pend,
               string::const_iterator ci, string::const_iterator cend) const {
  // If we run out of pattern or candidate string, it's a match only
  // if they both ran out at the same time.
  if (pi == pend || ci == cend) {
    // A special exception: we allow ci to reach the end before pi,
    // only if pi is one character before the end and that last
    // character is '*'.
    if ((ci == cend) && (pi + 1 == pend) && (*pi) == '*') {
      return true;
    }
    return (pi == pend && ci == cend);
  }

  switch (*pi) {
    
  case '*':
    // A '*' in the pattern string means to match any sequence of zero
    // or more characters in the candidate string.  This means we have
    // to recurse twice: either consume one character of the candidate
    // string and continue to try matching the *, or stop trying to
    // match the * here.
    return
      matches_substr(pi, pend, ci + 1, cend) ||
      matches_substr(pi + 1, pend, ci, cend);

  case '?':
    // A '?' in the pattern string means to match exactly one
    // character in the candidate string.  That's easy.
    return matches_substr(pi + 1, pend, ci + 1, cend);

  case '[':
    // An open square bracket begins a set.
    ++pi;
    if ((*pi) == '!') {
      ++pi;
      if (matches_set(pi, pend, *ci)) {
        return false;
      }
    } else {
      if (!matches_set(pi, pend, *ci)) {
        return false;
      }
    }
    if (pi == pend) {
      // Oops, there wasn't a closing square bracket.
      return false;
    }
    return matches_substr(pi + 1, pend, ci + 1, cend);
    
  case '\\':
    // A backslash escapes the next special character.
    ++pi;
    if (pi == pend) {
      return false;
    }
    // fall through.

  default:
    // Anything else means to match exactly that.
    if ((*pi) != (*ci)) {
      return false;
    }
    return matches_substr(pi + 1, pend, ci + 1, cend);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GlobPattern::matches_set
//       Access: Private
//  Description: Called when an unescaped open square bracked is
//               scanned, this is called with pi positioned after the
//               opening square bracket, scans the set sequence,
//               leaving pi positioned on the closing square bracket,
//               and returns true if the indicated character matches
//               the set of characters indicated, false otherwise.
////////////////////////////////////////////////////////////////////
bool GlobPattern::
matches_set(string::const_iterator &pi, string::const_iterator pend,
            char ch) const {
  bool matched = false;

  while (pi != pend && (*pi) != ']') {
    if ((*pi) == '\\') {
      // Backslash escapes the next character.
      ++pi;
      if (pi == pend) {
        return false;
      }
    }

    if (ch == (*pi)) {
      matched = true;
    }

    // Maybe it's an a-z style range?
    char start = (*pi);
    ++pi;
    if (pi != pend && (*pi) == '-') {
      ++pi;
      if (pi != pend && (*pi) != ']') {
        // Yes, we have a range: start-end.

        if ((*pi) == '\\') {
          // Backslash escapes.
          ++pi;
          if (pi == pend) {
            return false;
          }
        }

        char end = (*pi);
        ++pi;

        if (ch >= start && ch <= end) {
          matched = true;
        }
      } else {
        // This was a - at the end of the string.
        if (ch == '-') {
          matched = true;
        }
      }
    }
  }

  return matched;
}


  
