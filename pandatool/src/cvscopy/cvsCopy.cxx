// Filename: cvsCopy.cxx
// Created by:  drose (31Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "cvsCopy.h"
#include "cvsSourceDirectory.h"

#include "pnotify.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CVSCopy::
CVSCopy() {
  _model_dirname = ".";
  _key_filename = "Sources.pp";
  _cvs_binary = "cvs";
  _user_aborted = false;
  _model_dir = (CVSSourceDirectory *)NULL;
  _map_dir = (CVSSourceDirectory *)NULL;

  clear_runlines();
  add_runline("[opts] file [file ... ]");

  add_option
    ("f", "", 80,
     "Force the copy to happen without any input from the user.  If a file "
     "with the same name exists anywhere in the source hierarchy, it will "
     "be overwritten without prompting; if a file does not yet exist, it "
     "will be created in the directory named by -d or by -m, as appropriate.",
     &CVSCopy::dispatch_none, &_force);

  add_option
    ("i", "", 80,
     "The opposite of -f, this will prompt the user before each action.  "
     "The default is only to prompt the user when an action is ambiguous "
     "or unusual.",
     &CVSCopy::dispatch_none, &_interactive);

  add_option
    ("d", "dirname", 80,
     "Copy model files that are not already present somewhere in the tree "
     "to the indicated directory.  The default is the current directory.",
     &CVSCopy::dispatch_filename, &_got_model_dirname, &_model_dirname);

  add_option
    ("m", "dirname", 80,
     "Copy texture map files to the indicated directory.  The default "
     "is src/maps from the root directory.",
     &CVSCopy::dispatch_filename, &_got_map_dirname, &_map_dirname);

  add_option
    ("root", "dirname", 80,
     "Specify the root of the CVS source hierarchy.  The default is to "
     "use the ppremake convention of locating the directory above the -d "
     "directory that contains a file called Package.pp.",
     &CVSCopy::dispatch_filename, &_got_root_dirname, &_root_dirname);

  add_option
    ("key", "filename", 80,
     "Specify the name of the file that must exist in each directory for "
     "it to be considered part of the CVS source hierarchy.  The default "
     "is the ppremake convention, \"Sources.pp\".  Other likely candidates "
     "are \"CVS\" to search a CVS hierarchy, or \".\" to include "
     "all subdirectories indiscriminately.",
     &CVSCopy::dispatch_filename, NULL, &_key_filename);

  add_option
    ("nc", "", 80,
     "Do not attempt to add newly-created files to CVS.  The default "
     "is to add them.",
     &CVSCopy::dispatch_none, &_no_cvs);

  add_option
    ("cvs", "cvs_binary", 80,
     "Specify how to run the cvs program for adding newly-created files.  "
     "The default is simply \"cvs\".",
     &CVSCopy::dispatch_string, NULL, &_cvs_binary);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::import
//       Access: Public
//  Description: Checks for the given filename somewhere in the
//               directory hierarchy, and chooses a place to import
//               it.  Copies the file by calling copy_file().
//
//               Extra_data may be NULL or a pointer to some
//               user-defined structure; CVSCopy simply passes it
//               unchanged to copy_file().  It presumably gives the
//               class a hint as to how the file should be copied.
//               Suggested_dir is the suggested directory in which to
//               copy the file, if it does not already exist
//               elsewhere.
//
//               On success, returns the FilePath it was actually
//               copied to.  On failure, returns an invalid FilePath.
////////////////////////////////////////////////////////////////////
CVSSourceTree::FilePath CVSCopy::
import(const Filename &source, void *extra_data,
       CVSSourceDirectory *suggested_dir) {
  CopiedFiles::const_iterator ci;
  ci = _copied_files.find(source);
  if (ci != _copied_files.end()) {
    // We have already copied this file.
    return (*ci).second;
  }

  if (!source.exists()) {
    nout << "Source filename " << source << " does not exist!\n";
    return CVSSourceTree::FilePath();
  }

  string basename = filter_filename(source.get_basename());

  CVSSourceTree::FilePath path =
    _tree.choose_directory(basename, suggested_dir, _force, _interactive);
  nassertr(path.is_valid(), path);

  _copied_files[source] = path;
  Filename dest = path.get_fullpath();

  bool new_file = !dest.exists();
  if (!new_file && verify_file(source, dest, path._dir, extra_data)) {
    // The file is unchanged.
    nout << path.get_path() << " is unchanged.\n";

  } else {
    // The file has changed.
    nout << "Copying " << basename << " to " << path.get_path() << "\n";

    if (!copy_file(source, dest, path._dir, extra_data, new_file)) {
      if (!continue_after_error()) {
        return CVSSourceTree::FilePath();
      }
    } else {
      if (new_file) {
        cvs_add(dest);
      }
    }
  }

  return path;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::continue_after_error
//       Access: Public
//  Description: Prompts the user (unless -f was specified) if he
//               wants to continue the copy operation after some error
//               has occurred.  Returns true to continue, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
continue_after_error() {
  if (_force) {
    return true;
  }
  if (_user_aborted) {
    return false;
  }

  while (true) {
    string result = prompt("Error occurred during copy!  Continue (y/n)? ");
    nassertr(!result.empty(), false);
    if (result.size() == 1) {
      if (tolower(result[0]) == 'y') {
        return true;
      } else if (tolower(result[0]) == 'n') {
        _user_aborted = true;
        return false;
      }
    }

    nout << "*** Invalid response: " << result << "\n\n";
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
handle_args(Args &args) {
  if (args.empty()) {
    nout << "You must specify the file(s) to copy from on the command line.\n";
    return false;
  }

  for (Args::const_iterator ai = args.begin();
       ai != args.end();
       ++ai) {
    _source_files.push_back(Filename::from_os_specific(*ai));
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::post_command_line
//       Access: Protected, Virtual
//  Description: This is called after the command line has been
//               completely processed, and it gives the program a
//               chance to do some last-minute processing and
//               validation of the options and arguments.  It should
//               return true if everything is fine, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
post_command_line() {
  if (!scan_hierarchy()) {
    return false;
  }

  _model_dir = _tree.find_directory(_model_dirname);
  if (_model_dir == (CVSSourceDirectory *)NULL) {
    if (_got_model_dirname) {
      nout << "Warning: model directory " << _model_dirname
           << " is not within the source hierarchy.\n";
    }
  }

  if (_got_map_dirname) {
    _map_dir = _tree.find_directory(_map_dirname);

    if (_map_dir == (CVSSourceDirectory *)NULL) {
      nout << "Warning: map directory " << _map_dirname
           << " is not within the source hierarchy.\n";
    }

  } else {
    _map_dir = _tree.find_relpath("src/maps");

    if (_map_dir == (CVSSourceDirectory *)NULL) {
      nout << "Warning: no directory " << _tree.get_root_dirname()
           << "/src/maps.\n";
      _map_dir = _model_dir;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::verify_file
//       Access: Protected, Virtual
//  Description: Verifies that the file is identical and does not need
//               to be recopied.  Returns true if the files are
//               identical, false if they differ.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
verify_file(const Filename &, const Filename &,
            CVSSourceDirectory *, void *) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::verify_binary_file
//       Access: Protected
//  Description: Verifies that the file is identical and does not need
//               to be recopied.  Returns true if the files are
//               identical, false if they differ.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
verify_binary_file(Filename source, Filename dest) {
  if (source == dest) {
    return true;
  }

  source.set_binary();
  dest.set_binary();

  ifstream s, d;
  if (!source.open_read(s)) {
    return false;
  }
  if (!dest.open_read(d)) {
    return false;
  }

  int cs, cd;
  cs = s.get();
  cd = d.get();
  while (!s.eof() && !s.fail() && !d.eof() && !d.fail()) {
    if (cs != cd) {
      return false;
    }
    cs = s.get();
    cd = d.get();
  }

  if (s.fail() || d.fail()) {
    // If we had some read error, call the files different.
    return false;
  }

  // If we haven't reached the end of one of the files yet, that file
  // is longer than the other one, and the files are therefore
  // different.
  if (!s.eof() || !d.eof()) {
    return false;
  }

  // Otherwise, the files are the same.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::copy_binary_file
//       Access: Protected
//  Description: Copies a file without modifying it or scanning it in
//               any way.  This is particularly useful for copying
//               textures.  This is provided as a convenience function
//               for derived programs because so many model file
//               formats will also require copying textures or other
//               black-box files.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
copy_binary_file(Filename source, Filename dest) {
  if (source == dest) {
    return true;
  }

  source.set_binary();
  dest.set_binary();

  ifstream in;
  ofstream out;
  if (!source.open_read(in)) {
    nout << "Cannot read " << source << "\n";
    return false;
  }

  dest.unlink();
  if (!dest.open_write(out)) {
    nout << "Cannot write " << dest << "\n";
    return false;
  }

  int c;
  c = in.get();
  while (!in.eof() && !in.fail() && !out.fail()) {
    out.put(c);
    c = in.get();
  }

  if (!in.eof() && in.fail()) {
    nout << "Error reading " << source << "\n";
    return false;
  }
  if (out.fail()) {
    nout << "Error writing " << dest << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::cvs_add
//       Access: Protected
//  Description: Invokes CVS to add the indicated filename to the
//               repository, if the user so requested.  Returns true
//               if successful, false if there is an error.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
cvs_add(const Filename &filename) {
  if (_no_cvs) {
    return true;
  }

  if (!CVSSourceTree::temp_chdir(filename.get_dirname())) {
    nout << "Invalid directory: " << filename.get_dirname() << "\n";
    return false;
  }

  string command = _cvs_binary + " add -kb " + 
    protect_from_shell(filename.get_basename());
  nout << command << "\n";
  int result = system(command.c_str());

  CVSSourceTree::restore_cwd();

  if (result != 0) {
    nout << "Failure invoking cvs.\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::protect_from_shell
//       Access: Protected, Static
//  Description: Inserts escape characters into the indicated source
//               string to protect it from the shell, so that it may
//               be given on the command line.  Returns the modified
//               string.
////////////////////////////////////////////////////////////////////
string CVSCopy::
protect_from_shell(const string &source) {
  string result;

  for (string::const_iterator pi = source.begin(); pi != source.end(); ++pi) {
    switch (*pi) {
    case '\\':
    case ' ':
    case '\'':
    case '"':
    case '(':
    case ')':
    case '<':
    case '>':
    case '|':
    case '&':
    case '!':
    case '$':
    case '~':
    case '*':
    case '?':
    case '[':
    case ']':
    case ';':
      result += '\\';
      // fall through

    default:
      result += *pi;
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::filter_filename
//       Access: Protected, Virtual
//  Description: Given a source filename (including the basename only,
//               without a dirname), return the appropriate
//               corresponding filename within the source directory.
//               This may be used by derived classes to, for instance,
//               strip a version number from the filename.
////////////////////////////////////////////////////////////////////
string CVSCopy::
filter_filename(const string &source) {
  return source;
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::scan_hierarchy
//       Access: Private
//  Description: Starts the scan of the source hierarchy.  This
//               identifies all of the files in the source hierarchy
//               we're to copy these into, so we can guess where
//               referenced files should be placed.  Returns true if
//               everything is ok, false if there is an error.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
scan_hierarchy() {
  if (!_got_root_dirname) {
    // If we didn't get a root directory name, find the directory
    // above this one that contains the file "Package.pp".
    if (!scan_for_root(_model_dirname)) {
      return false;
    }
  }

  _tree.set_root(_root_dirname);
  nout << "Root is " << _tree.get_root_fullpath() << "\n";

  return _tree.scan(_key_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::scan_for_root
//       Access: Private
//  Description: Searches for the root of the source directory by
//               looking for the parent directory that contains
//               "Package.pp".  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool CVSCopy::
scan_for_root(const string &dirname) {
  Filename sources = dirname + "/Sources.pp";
  if (!sources.exists()) {
    nout << "Couldn't find " << sources << " in source directory.\n";
    return false;
  }
  Filename package = dirname + "/Package.pp";
  if (package.exists()) {
    // Here's the root!
    _root_dirname = dirname;
    return true;
  }

  return scan_for_root(dirname + "/..");
}

////////////////////////////////////////////////////////////////////
//     Function: CVSCopy::prompt
//       Access: Private
//  Description: Issues a prompt to the user and waits for a typed
//               response.  Returns the response (which will not be
//               empty).
////////////////////////////////////////////////////////////////////
string CVSCopy::
prompt(const string &message) {
  nout << flush;
  while (true) {
    cerr << message << flush;
    string response;
    getline(cin, response);

    // Remove leading and trailing whitespace.
    size_t p = 0;
    while (p < response.length() && isspace(response[p])) {
      p++;
    }

    size_t q = response.length();
    while (q > p && isspace(response[q - 1])) {
      q--;
    }

    if (q > p) {
      return response.substr(p, q - p);
    }
  }
}
