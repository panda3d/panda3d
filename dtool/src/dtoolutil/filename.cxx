// Filename: filename.cxx
// Created by:  drose (18Jan99)
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

#include "filename.h"
#include "dSearchPath.h"
#include "executionEnvironment.h"

#include <stdio.h>  // For rename() and tempnam()
#include <time.h>   // for clock() and time()
#include <sys/stat.h>
#include <algorithm>

#ifdef HAVE_UTIME_H
#include <utime.h>

// We assume we have these too.
#include <errno.h>
#include <fcntl.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

// It's true that dtoolbase.h includes this already, but we include
// this again in case we are building this file within ppremake.
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#ifdef WIN32
/* begin Win32-specific code */

#ifdef WIN32_VC
#include <direct.h>
#include <windows.h>
#endif

// We might have been linked with the Cygwin dll.  This is ideal if it
// is available, because it allows Panda to access all the Cygwin
// mount definitions if they are in use.  If the Cygwin dll is not
// available, we fall back to our own convention for converting
// pathnames.
#ifdef HAVE_CYGWIN
extern "C" void cygwin_conv_to_win32_path(const char *path, char *win32);
extern "C" void cygwin_conv_to_posix_path(const char *path, char *posix);
#endif

static string
front_to_back_slash(const string &str) {
  string result = str;
  string::iterator si;
  for (si = result.begin(); si != result.end(); ++si) {
    if ((*si) == '/') {
      (*si) = '\\';
    }
  }

  return result;
}

static string
back_to_front_slash(const string &str) {
  string result = str;
  string::iterator si;
  for (si = result.begin(); si != result.end(); ++si) {
    if ((*si) == '\\') {
      (*si) = '/';
    }
  }

  return result;
}

static const string &
get_panda_root() {
  static string panda_root;
  static bool got_panda_root = false;

  if (!got_panda_root) {
    const char *envvar = getenv("PANDA_ROOT");
    if (envvar != (const char *)NULL) {
      panda_root = front_to_back_slash(envvar);
    }

    if (panda_root.empty() || panda_root[panda_root.length() - 1] != '\\') {
      panda_root += '\\';
    }

    got_panda_root = true;
  }

  return panda_root;
}

static string
convert_pathname(const string &unix_style_pathname) {
  if (unix_style_pathname.empty()) {
    return string();
  }

  // To convert from a Unix-style pathname to a Windows-style
  // pathname, we need to change all forward slashes to backslashes.
  // We might need to add a prefix as well, since Windows pathnames
  // typically begin with a drive letter.

  // By convention, if the top directory name consists of just one
  // letter, we treat that as a drive letter and map the rest of the
  // filename accordingly.  On the other hand, if the top directory
  // name consists of more than one letter, we assume this is a file
  // within some predefined tree whose root is given by the
  // environment variable "PANDA_ROOT", or if that is not defined,
  // "CYGWIN_ROOT" (for backward compatibility).
  string windows_pathname;

  if (unix_style_pathname[0] != '/') {
    // It doesn't even start from the root, so we don't have to do
    // anything fancy--relative pathnames are the same in Windows as
    // in Unix, except for the direction of the slashes.
    windows_pathname = front_to_back_slash(unix_style_pathname);

  } else if (unix_style_pathname.length() > 3 &&
             isalpha(unix_style_pathname[1]) &&
             unix_style_pathname[2] == '/') {
    // This pathname begins with a slash and a single letter.  That
    // must be the drive letter.

    // We have to cast the result of toupper() to (char) to help some
    // compilers (e.g. Cygwin's gcc 2.95.3) happy; so that they do not
    // confuse this string constructor with one that takes two
    // iterators.
    windows_pathname =
      string(1, (char)toupper(unix_style_pathname[1])) + ":" +
      front_to_back_slash(unix_style_pathname.substr(2));

  } else {
    // It starts with a slash, but the first part is not a single
    // letter.

#ifdef HAVE_CYGWIN
    // Use Cygwin to convert it if possible.
    char result[4096] = "";
    cygwin_conv_to_win32_path(unix_style_pathname.c_str(), result);
    windows_pathname = result;
#else  // HAVE_CYGWIN
    // Without Cygwin, just prefix $PANDA_ROOT.
    windows_pathname = get_panda_root();
    windows_pathname += front_to_back_slash(unix_style_pathname.substr(1));
#endif  // HAVE_CYGWIN
  }

  return windows_pathname;
}

string
convert_dso_pathname(const string &unix_style_pathname) {
  // If the extension is .so, change it to .dll.
  size_t dot = unix_style_pathname.rfind('.');
  if (dot == string::npos ||
      unix_style_pathname.find('/', dot) != string::npos) {
    // No filename extension.
    return convert_pathname(unix_style_pathname);
  }
  if (unix_style_pathname.substr(dot) != ".so") {
    // Some other extension.
    return convert_pathname(unix_style_pathname);
  }

  string dll_basename = unix_style_pathname.substr(0, dot);

#ifdef _DEBUG
  // If we're building a debug version, all the dso files we link in
  // must be named file_d.dll.  This does prohibit us from linking in
  // external dso files, generated outside of the Panda build system,
  // that don't follow this _d convention.  Maybe we need a separate
  // convert_system_dso_pathname() function.

  // We can't simply check to see if the file exists, because this
  // might not be a full path to the dso filename--it might be
  // somewhere on the LD_LIBRARY_PATH, or on PATH, or any of a number
  // of nutty places.

  return convert_pathname(dll_basename + "_d.dll");
#else
  return convert_pathname(dll_basename + ".dll");
#endif
}

string
convert_executable_pathname(const string &unix_style_pathname) {
  // If the extension is not .exe, append .exe.
  size_t dot = unix_style_pathname.rfind('.');
  if (dot == string::npos ||
      unix_style_pathname.find('/', dot) != string::npos) {
    // No filename extension.
    return convert_pathname(unix_style_pathname + ".exe");
  }
  if (unix_style_pathname.substr(dot) != ".exe") {
    // Some other extension.
    return convert_pathname(unix_style_pathname + ".exe");
  }

  return convert_pathname(unix_style_pathname);
}
#endif //WIN32

////////////////////////////////////////////////////////////////////
//     Function: Filename::Constructor
//       Access: Public
//  Description: This constructor composes the filename out of a
//               directory part and a basename part.  It will insert
//               an intervening '/' if necessary.
////////////////////////////////////////////////////////////////////
Filename::
Filename(const Filename &dirname, const Filename &basename) {
  if (dirname.empty()) {
    (*this) = basename;
  } else {
    string dirpath = dirname.get_fullpath();
    if (dirpath[dirpath.length() - 1] == '/') {
      (*this) = dirpath + basename.get_fullpath();
    } else {
      (*this) = dirpath + "/" + basename.get_fullpath();
    }
  }
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::from_os_specific
//       Access: Public, Static
//  Description: This named constructor returns a Panda-style filename
//               (that is, using forward slashes, and no drive letter)
//               based on the supplied filename string that describes
//               a filename in the local system conventions (for
//               instance, on Windows, it may use backslashes or begin
//               with a drive letter and a colon).
//
//               Use this function to create a Filename from an
//               externally-given filename string.  Use
//               to_os_specific() again later to reconvert it back to
//               the local operating system's conventions.
//
//               This function will do the right thing even if the
//               filename is partially local conventions and partially
//               Panda conventions; e.g. some backslashes and some
//               forward slashes.
////////////////////////////////////////////////////////////////////
Filename Filename::
from_os_specific(const string &os_specific, Filename::Type type) {
#ifdef WIN32
  string result = back_to_front_slash(os_specific);
  const string &panda_root = get_panda_root();

  // If the initial prefix is the same as panda_root, remove it.
  if (!panda_root.empty() && panda_root.length() < result.length()) {
    bool matches = true;
    size_t p;
    for (p = 0; p < panda_root.length() && matches; p++) {
      char c = tolower(panda_root[p]);
      if (c == '\\') {
        c = '/';
      }
      matches = (c == tolower(result[p]));
    }

    if (matches) {
      // The initial prefix matches!  Replace the initial bit with a
      // leading slash.
      result = result.substr(panda_root.length());
      assert(!result.empty());
      if (result[0] != '/') {
        result = '/' + result;
      }
      Filename filename(result);
      filename.set_type(type);
      return filename;
    }
  }

  // All right, the initial prefix was not under panda_root.  But
  // maybe it begins with a drive letter.
  if (result.size() >= 3 && isalpha(result[0]) &&
      result[1] == ':' && result[2] == '/') {
    result[1] = tolower(result[0]);
    result[0] = '/';
  }

  Filename filename(result);
  filename.set_type(type);
  return filename;
#else  // WIN32
  // Generic Unix-style filenames--no conversion necessary.
  Filename filename(os_specific);
  filename.set_type(type);
  return filename;
#endif  // WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::expand_from
//       Access: Public, Static
//  Description: Returns the same thing as from_os_specific(), but
//               embedded environment variable references
//               (e.g. "$DMODELS/foo.txt") are expanded out.
////////////////////////////////////////////////////////////////////
Filename Filename::
expand_from(const string &os_specific, Filename::Type type) {
  return from_os_specific(ExecutionEnvironment::expand_string(os_specific),
                          type);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::temporary
//       Access: Public
//  Description: Generates a temporary filename within the indicated
//               directory, using the indicated prefix.  If the
//               directory is empty, a system-defined directory is
//               chosen instead.
//
//               The generated filename did not exist when the
//               Filename checked, but since it does not specifically
//               create the file, it is possible that another process
//               could simultaneously create a file by the same name.
////////////////////////////////////////////////////////////////////
Filename Filename::
temporary(const string &dirname, const string &prefix, Type type) {
  if (dirname.empty()) {
    // If we are not given a dirname, use the system tempnam()
    // function to create a system-defined temporary filename.
    char *name = tempnam(NULL, prefix.c_str());
    Filename result(name);
    free(name);
    result.set_type(type);
    return result;
  }

  // If we *are* given a dirname, then use our own algorithm to make
  // up a filename within that dirname.  We do that because the system
  // tempnam() (for instance, under Windows) may ignore the dirname.

  Filename result(dirname, "");
  result.set_type(type);
  do {
    // We take the time of day and multiply it by the process time.
    // This will give us a very large number, of which we take the
    // bottom 24 bits and generate a 6-character hex code.
    int hash = (clock() * time(NULL)) & 0xffffff;
    char hex_code[10];
    sprintf(hex_code, "%06x", hash);
    result.set_basename(prefix + hex_code);
  } while (result.exists());

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::set_fullpath
//       Access: Public
//  Description: Replaces the entire filename: directory, basename,
//               extension.  This can also be achieved with the
//               assignment operator.
////////////////////////////////////////////////////////////////////
void Filename::
set_fullpath(const string &s) {
  (*this) = s;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::set_dirname
//       Access: Public
//  Description: Replaces the directory part of the filename.  This is
//               everything in the filename up to, but not including
//               the rightmost slash.
////////////////////////////////////////////////////////////////////
void Filename::
set_dirname(const string &s) {
  if (s.empty()) {
    // Remove the directory prefix altogether.
    _filename.replace(0, _basename_start, "");

    int length_change = - ((int)_basename_start);

    _dirname_end = 0;
    _basename_start += length_change;
    _basename_end += length_change;
    _extension_start += length_change;

  } else {
    // Replace the existing directory prefix, or insert a new one.

    // We build the string ss to include the terminal slash.
    string ss;
    if (s[s.length()-1] == '/') {
      ss = s;
    } else {
      ss = s+'/';
    }

    int length_change = ss.length() - _basename_start;

    _filename.replace(0, _basename_start, ss);

    _dirname_end = ss.length() - 1;

    // An exception: if the dirname string was the single slash, the
    // dirname includes that slash.
    if (ss.length() == 1) {
      _dirname_end = 1;
    }

    _basename_start += length_change;

    if (_basename_end != string::npos) {
      _basename_end += length_change;
      _extension_start += length_change;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::set_basename
//       Access: Public
//  Description: Replaces the basename part of the filename.  This is
//               everything in the filename after the rightmost slash,
//               including any extensions.
////////////////////////////////////////////////////////////////////
void Filename::
set_basename(const string &s) {
  _filename.replace(_basename_start, string::npos, s);
  locate_extension();
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::set_fullpath_wo_extension
//       Access: Public
//  Description: Replaces the full filename--directory and basename
//               parts--except for the extension.
////////////////////////////////////////////////////////////////////
void Filename::
set_fullpath_wo_extension(const string &s) {
  int length_change = s.length() - _basename_end;

  _filename.replace(0, _basename_end, s);

  if (_basename_end != string::npos) {
    _basename_end += length_change;
    _extension_start += length_change;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::set_basename_wo_extension
//       Access: Public
//  Description: Replaces the basename part of the filename, without
//               the file extension.
////////////////////////////////////////////////////////////////////
void Filename::
set_basename_wo_extension(const string &s) {
  int length_change = s.length() - (_basename_end - _basename_start);

  if (_basename_end == string::npos) {
    _filename.replace(_basename_start, string::npos, s);

  } else {
    _filename.replace(_basename_start, _basename_end - _basename_start, s);

    _basename_end += length_change;
    _extension_start += length_change;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::set_extension
//       Access: Public
//  Description: Replaces the file extension.  This is everything after
//               the rightmost dot, if there is one, or the empty
//               string if there is not.
////////////////////////////////////////////////////////////////////
void Filename::
set_extension(const string &s) {
  if (s.empty()) {
    // Remove the extension altogether.
    if (_basename_end != string::npos) {
      _filename.replace(_basename_end, string::npos, "");
      _basename_end = string::npos;
      _extension_start = string::npos;
    }

  } else if (_basename_end == string::npos) {
    // Insert an extension where there was none before.
    _basename_end = _filename.length();
    _extension_start = _filename.length() + 1;
    _filename += '.' + s;

  } else {
    // Replace an existing extension.
    _filename.replace(_extension_start, string::npos, s);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::standardize
//       Access: Public
//  Description: Converts the filename to standard form by replacing
//               consecutive slashes with a single slash, removing a
//               trailing slash if present, and backing up over ../
//               sequences within the filename where possible.
////////////////////////////////////////////////////////////////////
void Filename::
standardize() {
  assert(!_filename.empty());
  if (_filename == ".") {
    // Don't change a single dot; this refers to the current directory.
    return;
  }

  vector<string> components;

  // Pull off the components of the filename one at a time.
  bool global = (_filename[0] == '/');

  size_t p = 0;
  while (p < _filename.length() && _filename[p] == '/') {
    p++;
  }
  while (p < _filename.length()) {
    size_t slash = _filename.find('/', p);
    string component = _filename.substr(p, slash - p);
    if (component == ".") {
      // Ignore /./.
    } else if (component == ".." && !components.empty() &&
               !(components.back() == "..")) {
      // Back up.
      components.pop_back();
    } else {
      components.push_back(component);
    }

    p = slash;
    while (p < _filename.length() && _filename[p] == '/') {
      p++;
    }
  }

  // Now reassemble the filename.
  string result;
  if (global) {
    result = "/";
  }
  if (!components.empty()) {
    result += components[0];
    for (int i = 1; i < (int)components.size(); i++) {
      result += "/" + components[i];
    }
  }

  (*this) = result;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::make_absolute
//       Access: Public
//  Description: Converts the filename to a fully-qualified pathname
//               from the root (if it is a relative pathname), and
//               then standardizes it (see standardize()).
//
//               This is sometimes a little problematic, since it may
//               convert the file to its 'true' absolute pathname,
//               which could be an ugly NFS-named file, irrespective
//               of symbolic links
//               (e.g. /.automount/dimbo/root/usr2/fit/people/drose
//               instead of /fit/people/drose); besides being ugly,
//               filenames like this may not be consistent across
//               multiple different platforms.
////////////////////////////////////////////////////////////////////
void Filename::
make_absolute() {
  make_absolute(ExecutionEnvironment::get_cwd());
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::make_absolute
//       Access: Public
//  Description: Converts the filename to a fully-qualified filename
//               from the root (if it is a relative filename), and
//               then standardizes it (see standardize()).  This
//               flavor accepts a specific starting directory that the
//               filename is known to be relative to.
////////////////////////////////////////////////////////////////////
void Filename::
make_absolute(const Filename &start_directory) {
  if (is_local()) {
    (*this) = Filename(start_directory, _filename);
  }

  standardize();
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::make_canonical
//       Access: Public
//  Description: Converts this filename to a canonical name by
//               replacing the directory part with the fully-qualified
//               directory part.  This is done by changing to that
//               directory and calling getcwd().
//
//               This has the effect of (a) converting relative paths
//               to absolute paths (but see make_absolute() if this is
//               the only effect you want), and (b) always resolving a
//               given directory name to the same string, even if
//               different symbolic links are traversed, and (c)
//               changing nice symbolic-link paths like
//               /fit/people/drose to ugly NFS automounter names like
//               /hosts/dimbo/usr2/fit/people/drose.  This can be
//               troubling, but sometimes this is exactly what you
//               want, particularly if you're about to call
//               make_relative_to() between two filenames.
//
//               The return value is true if successful, or false on
//               failure (usually because the directory name does not
//               exist or cannot be chdir'ed into).
////////////////////////////////////////////////////////////////////
bool Filename::
make_canonical() {
  if (empty()) {
    // An empty filename is a special case.  This doesn't name
    // anything.
    return false;
  }

  // Temporarily save the current working directory.
  Filename cwd = ExecutionEnvironment::get_cwd();

  if (is_directory()) {
    // If the filename itself represents a directory and not a
    // filename, cd to the named directory, not the one above it.
    string dirname = to_os_specific();

    if (chdir(dirname.c_str()) < 0) {
      return false;
    }
    (*this) = ExecutionEnvironment::get_cwd();

  } else {
    // Otherwise, if the filename represents a regular file (or
    // doesn't even exist), cd to the directory above.
    Filename dir(get_dirname());

    if (dir.empty()) {
      // No dirname means the file is in this directory.
      set_dirname(cwd);
      return true;
    }

    string dirname = dir.to_os_specific();
    if (chdir(dirname.c_str()) < 0) {
      return false;
    }
    set_dirname(ExecutionEnvironment::get_cwd().get_fullpath());
  }

  // Now restore the current working directory.
  string osdir = cwd.to_os_specific();
  if (chdir(osdir.c_str()) < 0) {
    cerr << "Error!  Cannot change back to " << cwd << "\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::to_os_specific
//       Access: Public
//  Description: Converts the filename from our generic Unix-like
//               convention (forward slashes starting with the root at
//               '/') to the corresponding filename in the local
//               operating system (slashes in the appropriate
//               direction, starting with the root at C:\, for
//               instance).  Returns the string representing the
//               converted filename, but does not change the Filename
//               itself.
//
//               See also from_os_specific().
////////////////////////////////////////////////////////////////////
string Filename::
to_os_specific() const {
  if (empty()) {
    return string();
  }
  Filename standard(*this);
  standard.standardize();

#ifdef WIN32
  switch (get_type()) {
  case T_dso:
    return convert_dso_pathname(standard.get_fullpath());
  case T_executable:
    return convert_executable_pathname(standard.get_fullpath());
  default:
    return convert_pathname(standard.get_fullpath());
  }
#else // WIN32
  return standard;
#endif // WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::exists
//       Access: Public
//  Description: Returns true if the filename exists on the disk,
//               false otherwise.  If the type is indicated to be
//               executable, this also tests that the file has execute
//               permission.
////////////////////////////////////////////////////////////////////
bool Filename::
exists() const {
  string os_specific = to_os_specific();

#ifdef WIN32_VC
  bool exists = false;

  DWORD results = GetFileAttributes(os_specific.c_str());
  if (results != -1) {
    exists = true;
  }

#else  // WIN32_VC
  struct stat this_buf;
  bool exists = false;

  if (stat(os_specific.c_str(), &this_buf) == 0) {
    exists = true;
  }
#endif

  return exists;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_regular_file
//       Access: Public
//  Description: Returns true if the filename exists and is the
//               name of a regular file (i.e. not a directory or
//               device), false otherwise.
////////////////////////////////////////////////////////////////////
bool Filename::
is_regular_file() const {
  string os_specific = to_os_specific();

#ifdef WIN32_VC
  bool isreg = false;

  DWORD results = GetFileAttributes(os_specific.c_str());
  if (results != -1) {
    isreg = ((results & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE)) == 0);
  }

#else  // WIN32_VC
  struct stat this_buf;
  bool isreg = false;

  if (stat(os_specific.c_str(), &this_buf) == 0) {
    isreg = S_ISREG(this_buf.st_mode);
  }
#endif

  return isreg;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_directory
//       Access: Public
//  Description: Returns true if the filename exists and is a
//               directory name, false otherwise.
////////////////////////////////////////////////////////////////////
bool Filename::
is_directory() const {
  string os_specific = to_os_specific();

#ifdef WIN32_VC
  bool isdir = false;

  DWORD results = GetFileAttributes(os_specific.c_str());
  if (results != -1) {
    isdir = (results & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }
#else  // WIN32_VC
  struct stat this_buf;
  bool isdir = false;

  if (stat(os_specific.c_str(), &this_buf) == 0) {
    isdir = S_ISDIR(this_buf.st_mode);
  }
#endif

  return isdir;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_executable
//       Access: Public
//  Description: Returns true if the filename exists and is
//               executable
////////////////////////////////////////////////////////////////////
bool Filename::
is_executable() const {
#ifdef WIN32_VC
  // no access() in windows, but to our advantage executables can only
  // end in .exe or .com
  string extension = get_extension();
  if (extension == "exe" || extension == "com") {
    return exists();
  }

#else /* WIN32_VC */
  string os_specific = to_os_specific();
  if (access(os_specific.c_str(), X_OK) == 0) {
    return true;
  }
#endif /* WIN32_VC */

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::compare_timestamps
//       Access: Public
//  Description: Returns a number less than zero if the file named by
//               this object is older than the given file, zero if
//               they have the same timestamp, or greater than zero if
//               this one is newer.
//
//               If this_missing_is_old is true, it indicates that a
//               missing file will be treated as if it were older than
//               any other file; otherwise, a missing file will be
//               treated as if it were newer than any other file.
//               Similarly for other_missing_is_old.
////////////////////////////////////////////////////////////////////
int Filename::
compare_timestamps(const Filename &other,
                   bool this_missing_is_old,
                   bool other_missing_is_old) const {
  string os_specific = to_os_specific();
  string other_os_specific = other.to_os_specific();

#ifdef WIN32_VC
  struct _stat this_buf;
  bool this_exists = false;

  if (_stat(os_specific.c_str(), &this_buf) == 0) {
    this_exists = true;
  }

  struct _stat other_buf;
  bool other_exists = false;

  if (_stat(other_os_specific.c_str(), &other_buf) == 0) {
    other_exists = true;
  }
#else  // WIN32_VC
  struct stat this_buf;
  bool this_exists = false;

  if (stat(os_specific.c_str(), &this_buf) == 0) {
    this_exists = true;
  }

  struct stat other_buf;
  bool other_exists = false;

  if (stat(other_os_specific.c_str(), &other_buf) == 0) {
    other_exists = true;
  }
#endif

  if (this_exists && other_exists) {
    // Both files exist, return the honest time comparison.
    return (int)this_buf.st_mtime - (int)other_buf.st_mtime;

  } else if (!this_exists && !other_exists) {
    // Neither file exists.
    if (this_missing_is_old == other_missing_is_old) {
      // Both files are either "very old" or "very new".
      return 0;
    }
    if (this_missing_is_old) {
      // This file is "very old", the other is "very new".
      return -1;
    } else {
      // This file is "very new", the other is "very old".
      return 1;
    }

  } else if (!this_exists) {
    // This file doesn't, the other one does.
    return this_missing_is_old ? -1 : 1;

  } else { // !other_exists
    assert(!other_exists);

    // This file exists, the other one doesn't.
    return other_missing_is_old ? 1 : -1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::resolve_filename
//       Access: Public
//  Description: Searches the given search path for the filename.  If
//               it is found, updates the filename to the full
//               pathname found and returns true; otherwise, returns
//               false.
////////////////////////////////////////////////////////////////////
bool Filename::
resolve_filename(const DSearchPath &searchpath,
                 const string &default_extension) {
  string found;

  if (is_local()) {
    found = searchpath.find_file(get_fullpath());

    if (found.empty()) {
      // We didn't find it with the given extension; can we try the
      // default extension?
      if (get_extension().empty() && !default_extension.empty()) {
        Filename try_ext = *this;
        try_ext.set_extension(default_extension);
        found = searchpath.find_file(try_ext.get_fullpath());
      }
    }

  } else {
    if (exists()) {
      // The full pathname exists.  Return true.
      return true;

    } else {
      // The full pathname doesn't exist with the given extension;
      // does it exist with the default extension?
      if (get_extension().empty() && !default_extension.empty()) {
        Filename try_ext = *this;
        try_ext.set_extension(default_extension);
        if (try_ext.exists()) {
          found = try_ext;
        }
      }
    }
  }

  if (!found.empty()) {
    (*this) = found;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::make_relative_to
//       Access: Public
//  Description: Adjusts this filename, which must be a
//               fully-specified pathname beginning with a slash, to
//               make it a relative filename, relative to the
//               fully-specified directory indicated (which must also
//               begin with, and may or may not end with, a slash--a
//               terminating slash is ignored).
//
//               This only performs a string comparsion, so it may be
//               wise to call make_canonical() on both filenames
//               before calling make_relative_to().
//
//               If allow_backups is false, the filename will only be
//               adjusted to be made relative if it is already
//               somewhere within or below the indicated directory.
//               If allow_backups is true, it will be adjusted in all
//               cases, even if this requires putting a series of ../
//               characters before the filename--unless it would have
//               to back all the way up to the root.
//
//               Returns true if the file was adjusted, false if it
//               was not.
////////////////////////////////////////////////////////////////////
bool Filename::
make_relative_to(Filename directory, bool allow_backups) {
  if (_filename.empty() || directory.empty() ||
      _filename[0] != '/' || directory[0] != '/') {
    return false;
  }

  standardize();
  directory.standardize();

  if (directory == "/") {
    // Don't be silly.
    return false;
  }

  string rel_to_file = directory.get_fullpath() + "/.";

  size_t common = get_common_prefix(rel_to_file);
  if (common < 2) {
    // Oh, never mind.
    return false;
  }

  string result;
  int slashes = count_slashes(rel_to_file.substr(common));
  if (slashes > 0 && !allow_backups) {
    // Too bad; the file's not under the indicated directory.
    return false;
  }

  for (int i = 0; i < slashes; i++) {
    result += "../";
  }
  result += _filename.substr(common);
  (*this) = result;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::find_on_searchpath
//       Access: Public
//  Description: Performs the reverse of the resolve_filename()
//               operation: assuming that the current filename is
//               fully-specified pathname (i.e. beginning with '/'),
//               look on the indicated search path for a directory
//               under which the file can be found.  When found,
//               adjust the Filename to be relative to the indicated
//               directory name.
//
//               Returns the index of the directory on the searchpath
//               at which the file was found, or -1 if it was not
//               found.
////////////////////////////////////////////////////////////////////
int Filename::
find_on_searchpath(const DSearchPath &searchpath) {
  if (_filename.empty() || _filename[0] != '/') {
    return -1;
  }

  int num_directories = searchpath.get_num_directories();
  for (int i = 0; i < num_directories; i++) {
    if (make_relative_to(searchpath.get_directory(i), false)) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::scan_directory
//       Access: Public
//  Description: Attempts to open the named filename as if it were a
//               directory and looks for the non-hidden files within
//               the directory.  Fills the given vector up with the
//               sorted list of filenames that are local to this
//               directory.
//
//               It is the user's responsibility to ensure that the
//               contents vector is empty before making this call;
//               otherwise, the new files will be appended to it.
//
//               Returns true on success, false if the directory could
//               not be read for some reason.
////////////////////////////////////////////////////////////////////
bool Filename::
scan_directory(vector_string &contents) const {
#if defined(HAVE_DIRENT_H)
  size_t orig_size = contents.size();

  string dirname;
  if (empty()) {
    dirname = ".";
  } else {
    dirname = _filename;
  }
  DIR *root = opendir(dirname.c_str());
  if (root == (DIR *)NULL) {
    return false;
  }

  struct dirent *d;
  d = readdir(root);
  while (d != (struct dirent *)NULL) {
    if (d->d_name[0] != '.') {
      contents.push_back(d->d_name);
    }
    d = readdir(root);
  }
  closedir(root);

  sort(contents.begin() + orig_size, contents.end());
  return true;

#elif defined(WIN32_VC)
  // Use FindFirstFile()/FindNextFile() to walk through the list of
  // files in a directory.
  size_t orig_size = contents.size();

  string match;
  if (empty()) {
    match = "*.*";
  } else {
    match = to_os_specific() + "\\*.*";
  }
  WIN32_FIND_DATA find_data;

  HANDLE handle = FindFirstFile(match.c_str(), &find_data);
  if (handle == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_NO_MORE_FILES) {
      // No matching files is not an error.
      return true;
    }
    return false;
  }

  do {
    string filename = find_data.cFileName;
    if (filename != "." && filename != "..") {
      contents.push_back(filename);
    }
  } while (FindNextFile(handle, &find_data));

  bool scan_ok = (GetLastError() == ERROR_NO_MORE_FILES);
  FindClose(handle);

  sort(contents.begin() + orig_size, contents.end());
  return scan_ok;
  
#else
  // Don't know how to scan directories!
  return false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::open_read
//       Access: Public
//  Description: Opens the indicated ifstream for reading the file, if
//               possible.  Returns true if successful, false
//               otherwise.  This requires the setting of the
//               set_text()/set_binary() flags to open the file
//               appropriately as indicated; it is an error to call
//               open_read() without first calling one of set_text()
//               or set_binary().
////////////////////////////////////////////////////////////////////
bool Filename::
open_read(ifstream &stream) const {
  assert(is_text() || is_binary());

  ios_openmode open_mode = ios::in;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

  string os_specific = to_os_specific();
  stream.clear();
  stream.open(os_specific.c_str(), open_mode);
  return (!stream.fail());
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::open_write
//       Access: Public
//  Description: Opens the indicated ifstream for writing the file, if
//               possible.  Returns true if successful, false
//               otherwise.  This requires the setting of the
//               set_text()/set_binary() flags to open the file
//               appropriately as indicated; it is an error to call
//               open_read() without first calling one of set_text()
//               or set_binary().
//
//               If truncate is true, the file is truncated to zero
//               length upon opening it, if it already exists.
//               Otherwise, the file is kept at its original length.
////////////////////////////////////////////////////////////////////
bool Filename::
open_write(ofstream &stream, bool truncate) const {
  assert(is_text() || is_binary());

  ios_openmode open_mode = ios::out;

  if (truncate) {
    open_mode |= ios::trunc;

  } else {
    // Some systems insist on having ios::in set to prevent the file
    // from being truncated when we open it.  Makes ios::trunc kind of
    // pointless, doesn't it?  On the other hand, setting ios::in also
    // seems to imply ios::nocreate (!), so we should only set this if
    // the file already exists.
    if (exists()) {
      open_mode |= ios::in;
    }
  }

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

  stream.clear();
  string os_specific = to_os_specific();
#ifdef HAVE_OPEN_MASK
  stream.open(os_specific.c_str(), open_mode, 0666);
#else
  stream.open(os_specific.c_str(), open_mode);
#endif

  return (!stream.fail());
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::open_append
//       Access: Public
//  Description: Opens the indicated ifstream for writing the file, if
//               possible.  Returns true if successful, false
//               otherwise.  This requires the setting of the
//               set_text()/set_binary() flags to open the file
//               appropriately as indicated; it is an error to call
//               open_read() without first calling one of set_text()
//               or set_binary().
////////////////////////////////////////////////////////////////////
bool Filename::
open_append(ofstream &stream) const {
  assert(is_text() || is_binary());

  ios_openmode open_mode = ios::app;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

  stream.clear();
  string os_specific = to_os_specific();
#ifdef HAVE_OPEN_MASK
  stream.open(os_specific.c_str(), open_mode, 0666);
#else
  stream.open(os_specific.c_str(), open_mode);
#endif

  return (!stream.fail());
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::open_read_write
//       Access: Public
//  Description: Opens the indicated fstream for read/write access to
//               the file, if possible.  Returns true if successful,
//               false otherwise.  This requires the setting of the
//               set_text()/set_binary() flags to open the file
//               appropriately as indicated; it is an error to call
//               open_read() without first calling one of set_text()
//               or set_binary().
////////////////////////////////////////////////////////////////////
bool Filename::
open_read_write(fstream &stream) const {
  assert(is_text() || is_binary());

  ios_openmode open_mode = ios::in | ios::out;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

  stream.clear();
  string os_specific = to_os_specific();
#ifdef HAVE_OPEN_MASK
  stream.open(os_specific.c_str(), open_mode, 0666);
#else
  stream.open(os_specific.c_str(), open_mode);
#endif

  return (!stream.fail());
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::touch
//       Access: Public
//  Description: Updates the modification time of the file to the
//               current time.  If the file does not already exist, it
//               will be created.  Returns true if successful, false
//               if there is an error.
////////////////////////////////////////////////////////////////////
bool Filename::
touch() const {
#ifdef WIN32_VC
  // In Windows, we have to use the Windows API to do this reliably.

  // First, guarantee the file exists (and also get its handle).
  string os_specific = to_os_specific();
  HANDLE fhandle;
  fhandle = CreateFile(os_specific.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fhandle == INVALID_HANDLE_VALUE) {
    return false;
  }

  // Now update the file time and date.
  SYSTEMTIME sysnow;
  FILETIME ftnow;
  GetSystemTime(&sysnow);
  if (!SystemTimeToFileTime(&sysnow, &ftnow)) {
    CloseHandle(fhandle);
    return false;
  }
  
  if (!SetFileTime(fhandle, NULL, NULL, &ftnow)) {
    CloseHandle(fhandle);
    return false;
  }

  CloseHandle(fhandle);
  return true;

#elif defined(HAVE_UTIME_H)
  // Most Unix systems can do this explicitly.

  string os_specific = to_os_specific();
#ifdef HAVE_CYGWIN
  // In the Cygwin case, it seems we need to be sure to use the
  // Cygwin-style name; some broken utime() implementation.  That's
  // almost the same thing as the original Panda-style name, but not
  // exactly, so we first convert the Panda name to a Windows name,
  // then convert it back to Cygwin, to ensure we get it exactly right
  // by Cygwin rules.
  {
    char result[4096] = "";
    cygwin_conv_to_posix_path(os_specific.c_str(), result);
    os_specific = result;
  }
#endif  // HAVE_CYGWIN
  int result = utime(os_specific.c_str(), NULL);
  if (result < 0) {
    if (errno == ENOENT) {
      // So the file doesn't already exist; create it.
      int fd = creat(os_specific.c_str(), 0666);
      if (fd < 0) {
        perror(os_specific.c_str());
        return false;
      }
      close(fd);
      return true;
    }
    perror(os_specific.c_str());
    return false;
  }
  return true;
#else  // WIN32, HAVE_UTIME_H
  // Other systems may not have an explicit control over the
  // modification time.  For these systems, we'll just temporarily
  // open the file in append mode, then close it again (it gets closed
  // when the ofstream goes out of scope).
  ofstream file;
  return open_append(file);
#endif  // WIN32, HAVE_UTIME_H
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::unlink
//       Access: Public
//  Description: Permanently deletes the file associated with the
//               filename, if possible.  Returns true if successful,
//               false if failure (for instance, because the file did
//               not exist, or because permissions were inadequate).
////////////////////////////////////////////////////////////////////
bool Filename::
unlink() const {
  string os_specific = to_os_specific();
  return (::unlink(os_specific.c_str()) == 0);
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::rename_to
//       Access: Public
//  Description: Renames the file to the indicated new filename.  If
//               the new filename is in a different directory, this
//               will perform a move.  Returns true if successful,
//               false if failure.
////////////////////////////////////////////////////////////////////
bool Filename::
rename_to(const Filename &other) const {
  string os_specific = to_os_specific();
  string other_os_specific = other.to_os_specific();
  return (rename(os_specific.c_str(),
                 other_os_specific.c_str()) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::make_dir
//       Access: Public
//  Description: Creates all the directories in the path to the file
//               specified in the filename, except for the basename
//               itself.  This assumes that the Filename contains the
//               name of a file, not a directory name; it ensures that
//               the directory containing the file exists.
//
//               However, if the filename ends in a slash, it assumes
//               the Filename represents the name of a directory, and
//               creates all the paths.
////////////////////////////////////////////////////////////////////
bool Filename::
make_dir() const {
  if (empty()) {
    return false;
  }
  Filename path = *this;
  path.standardize();
  string dirname;
  if (_filename[_filename.length() - 1] == '/') {
    // The Filename ends in a slash; it represents a directory.
    dirname = path.get_fullpath();

  } else {
    // The Filename does not end in a slash; it represents a file.
    dirname = path.get_dirname();
  }

  // First, make sure everything up to the last path is known.  We
  // don't care too much if any of these fail; maybe they failed
  // because the directory was already there.
  size_t slash = dirname.find('/');
  while (slash != string::npos) {
    Filename component(dirname.substr(0, slash));
    string os_specific = component.to_os_specific();
#ifndef WIN32_VC
    mkdir(os_specific.c_str(), 0777);
#else
    mkdir(os_specific.c_str());
#endif
    slash = dirname.find('/', slash + 1);
  }

  // Now make the last one, and check the return value.
  Filename component(dirname);
  string os_specific = component.to_os_specific();
#ifndef WIN32_VC
  int result = mkdir(os_specific.c_str(), 0777);
#else
  int result = mkdir(os_specific.c_str());
#endif

  return (result == 0);
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::locate_basename
//       Access: Private
//  Description: After the string has been reassigned, search for the
//               slash marking the beginning of the basename, and set
//               _dirname_end and _basename_start correctly.
////////////////////////////////////////////////////////////////////
void Filename::
locate_basename() {
  // Scan for the last slash, which marks the end of the directory
  // part.
  if (_filename.empty()) {
    _dirname_end = 0;
    _basename_start = 0;

  } else {

    string::size_type slash = _filename.rfind('/');
    if (slash != string::npos) {
      _basename_start = slash + 1;
      _dirname_end = _basename_start;

      // One exception: in case there are multiple slashes in a row,
      // we want to treat them as a single slash.  The directory
      // therefore actually ends at the first of these; back up a bit.
      while (_dirname_end > 0 && _filename[_dirname_end-1] == '/') {
        _dirname_end--;
      }

      // Another exception: if the dirname was nothing but slashes, it
      // was the root directory, or / itself.  In this case the dirname
      // does include the terminal slash (of course).
      if (_dirname_end == 0) {
        _dirname_end = 1;
      }

    } else {
      _dirname_end = 0;
      _basename_start = 0;
    }
  }

  // Now:

  // _dirname_end is the last slash character, or 0 if there are no
  // slash characters.

  // _basename_start is the character after the last slash character,
  // or 0 if there are no slash characters.
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::locate_extension
//       Access: Private
//  Description: Once the end of the directory prefix has been found,
//               and _dirname_end and _basename_start are set
//               correctly, search for the dot marking the beginning
//               of the extension, and set _basename_end and
//               _extension_start correctly.
////////////////////////////////////////////////////////////////////
void Filename::
locate_extension() {
  // Now scan for the last dot after that slash.
  if (_filename.empty()) {
    _basename_end = string::npos;
    _extension_start = string::npos;

  } else {
    string::size_type dot = _filename.length() - 1;

    while (dot+1 > _basename_start && _filename[dot] != '.') {
      --dot;
    }

    if (dot+1 > _basename_start) {
      _basename_end = dot;
      _extension_start = dot + 1;
    } else {
      _basename_end = string::npos;
      _extension_start = string::npos;
    }
  }

  // Now:

  // _basename_end is the last dot, or npos if there is no dot.

  // _extension_start is the character after the last dot, or npos if
  // there is no dot.
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::get_common_prefix
//       Access: Private
//  Description: Returns the length of the longest common initial
//               substring of this string and the other one that ends
//               in a slash.  This is the lowest directory common to
//               both filenames.
////////////////////////////////////////////////////////////////////
size_t Filename::
get_common_prefix(const string &other) const {
  size_t len = 0;

  // First, get the length of the common initial substring.
  while (len < length() && len < other.length() &&
         _filename[len] == other[len]) {
    len++;
  }

  // Now insist that it ends in a slash.
  while (len > 0 && _filename[len-1] != '/') {
    len--;
  }

  return len;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::count_slashes
//       Access: Private, Static
//  Description: Returns the number of non-consecutive slashes in the
//               indicated string, not counting a terminal slash.
////////////////////////////////////////////////////////////////////
int Filename::
count_slashes(const string &str) {
  int count = 0;
  string::const_iterator si;
  si = str.begin();

  while (si != str.end()) {
    if (*si == '/') {
      count++;

      // Skip consecutive slashes.
      ++si;
      while (*si == '/') {
        ++si;
      }
      if (si == str.end()) {
        // Oops, that was a terminal slash.  Don't count it.
        count--;
      }

    } else {
      ++si;
    }
  }

  return count;
}

