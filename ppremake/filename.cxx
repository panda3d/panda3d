// Filename: filename.cxx
// Created by:  drose (19Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "filename.h"
#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: PPScope::is_fullpath
//  Description: Returns true if the given pathname appears to be a
//               fully-specified pathname.  This means it begins with
//               a slash for unix_platform, and it begins with a slash
//               or backslash, with an optional drive leterr, for
//               windows_platform.
////////////////////////////////////////////////////////////////////
bool
is_fullpath(const string &pathname) {
  if (pathname.empty()) {
    return false;
  }

  if (pathname[0] == '/') {
    return true;
  }

  if (windows_platform) {
    if (pathname.length() > 2 && 
	isalpha(pathname[0]) && pathname[1] == ':') {
      // A drive-letter prefix.
      return (pathname[2] == '/' || pathname[2] == '\\');
    }
    // No drive-letter prefix.
    return (pathname[0] == '\\');
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::to_os_filename
//  Description: Changes forward slashes to backslashes, but only if
//               windows_platform is set.  Otherwise returns the
//               string unchanged.
////////////////////////////////////////////////////////////////////
string
to_os_filename(string pathname) {
  if (windows_platform) {
    string::iterator si;
    for (si = pathname.begin(); si != pathname.end(); ++si) {
      if ((*si) == '/') {
	(*si) = '\\';
      }
    }
  }

  return pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: PPScope::to_unix_filename
//  Description: Changes backslashes to forward slashes, but only if
//               windows_platform is set.  Otherwise returns the
//               string unchanged.
////////////////////////////////////////////////////////////////////
string
to_unix_filename(string pathname) {
  if (windows_platform) {
    string::iterator si;
    for (si = pathname.begin(); si != pathname.end(); ++si) {
      if ((*si) == '\\') {
	(*si) = '/';
      }
    }
  }

  return pathname;
}
