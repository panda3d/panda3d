// Filename: filename.C
// Created by:  drose (18Jan99)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////

#include "filename.h"
#include "dSearchPath.h"

#include <stdio.h>  // For rename()
#include <sys/stat.h>


#if defined(WIN32)
/* begin Win32-specific code */

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
    // This is a pathname that begins with a single letter.  That must
    // be the drive letter.
    windows_pathname = 
      toupper(unix_style_pathname[1]) + ":" +
      front_to_back_slash(unix_style_pathname.substr(2));

  } else {
    // It does not begin with a single letter, so prefix "PANDA_ROOT".

    static string panda_root;
    static bool got_panda_root = false;

    if (!got_panda_root) {
      const char *envvar = getenv("PANDA_ROOT");
      if (envvar == (const char *)NULL) {
	envvar = getenv("CYGWIN_ROOT");
      }

      if (envvar != (const char *)NULL) {
	panda_root = front_to_back_slash(envvar);
      }

      if (!panda_root.empty() && panda_root[panda_root.length() - 1] != '\\') {
	panda_root += '\\';
      }

      got_panda_root = true;
    }

    windows_pathname = panda_root + front_to_back_slash(unix_style_pathname);
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
  (*this) = dirname.get_fullpath() + "/" + basename.get_fullpath();
  _flags = 0;
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

    int length_change = -_basename_start;

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
////////////////////////////////////////////////////////////////////
string Filename::
to_os_specific() const {
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
#ifdef WIN32_VC
  // Windows puts underscores in front of most of the function and
  // structure names it borrowed from Unix.  Embrace and extend.
  struct _stat this_buf;
  bool exists = false;

  if (_stat(to_os_specific().c_str(), &this_buf) == 0) {
    exists = true;
  }
#else  // WIN32_VC
  struct stat this_buf;
  bool exists = false;

  if (stat(to_os_specific().c_str(), &this_buf) == 0) {
    exists = true;
  }
#endif

  return exists;
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

#ifdef WIN32_VC
  struct _stat this_buf;
  bool this_exists = false;

  if (_stat(to_os_specific().c_str(), &this_buf) == 0) {
    this_exists = true;
  }

  struct _stat other_buf;
  bool other_exists = false;

  if (_stat(other.to_os_specific().c_str(), &other_buf) == 0) {
    other_exists = true;
  }
#else  // WIN32_VC
  struct stat this_buf;
  bool this_exists = false;

  if (stat(to_os_specific().c_str(), &this_buf) == 0) {
    this_exists = true;
  }

  struct stat other_buf;
  bool other_exists = false;

  if (stat(other.to_os_specific().c_str(), &other_buf) == 0) {
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

  int open_mode = ios::in;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

  stream.open(to_os_specific().c_str(), open_mode);
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
////////////////////////////////////////////////////////////////////
bool Filename::
open_write(ofstream &stream) const {
  assert(is_text() || is_binary());

  int open_mode = ios::out;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

#ifdef WIN32_VC
  stream.open(to_os_specific().c_str(), open_mode);
#else
  stream.open(to_os_specific().c_str(), open_mode, 0666);
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

  int open_mode = ios::app;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

#ifdef WIN32_VC
  stream.open(to_os_specific().c_str(), open_mode);
#else
  stream.open(to_os_specific().c_str(), open_mode, 0666);
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

  int open_mode = ios::in | ios::out;

#ifdef HAVE_IOS_BINARY
  // For some reason, some systems (like Irix) don't define
  // ios::binary.
  if (!is_text()) {
    open_mode |= ios::binary;
  }
#endif

#ifdef WIN32_VC
  stream.open(to_os_specific().c_str(), open_mode);
#else
  stream.open(to_os_specific().c_str(), open_mode, 0666);
#endif

  return (!stream.fail());
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
  return (::unlink(to_os_specific().c_str()) == 0);
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
  return (rename(to_os_specific().c_str(), 
		 other.to_os_specific().c_str()) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::mkdir
//       Access: Public
//  Description: Creates all the directories in the path to the file
//		 specified in the filename (useful for writing).
////////////////////////////////////////////////////////////////////
bool Filename::
make_dir() const {
  size_t p = 0;
  while (p < _filename.length()) {
    size_t slash = _filename.find('/', p);
    if (slash != string::npos) {
      string component = _filename.substr(0, slash);
      if (!(component == ".") || 
          !(component == "..")) {
        mkdir(component.c_str(), 0xffff);
      }
    }
    p = slash;
    while (p < _filename.length() && _filename[p] == '/')
      p++;
  }
  return true;
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

