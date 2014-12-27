// Filename: softCVS.cxx
// Created by:  drose (10Nov00)
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

#include "softCVS.h"

#include "pnotify.h"
#include "multifile.h"
#include "pystub.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SoftCVS::
SoftCVS() {
  _cvs_binary = "cvs";

  set_program_brief("prepare a SoftImage database directory for adding to CVS");
  set_program_description
    ("softcvs is designed to prepare a directory hierarchy "
     "representing a SoftImage database for adding to CVS.  "
     "First, it eliminates SoftImage's silly filename-based "
     "versioning system by renaming versioned filenames higher "
     "than 1-0 back to version 1-0.  Then, it rolls up all the "
     "files for each scene except the texture images into a Panda "
     "multifile, which is added to CVS; the texture images are "
     "directly added to CVS where they are.\n\n"

     "The reduction of hundreds of SoftImage files per scene down to one "
     "multifile and a handle of texture images should greatly improve "
     "the update and commit times of CVS.\n\n"

     "You must run this from within the root of a SoftImage database "
     "directory; e.g. the directory that contains SCENES, PICTURES, MODELS, "
     "and so on.");

  clear_runlines();
  add_runline("[opts]");

  add_option
    ("nc", "", 80,
     "Do not attempt to add newly-created files to CVS.  The default "
     "is to add them.",
     &SoftCVS::dispatch_none, &_no_cvs);

  add_option
    ("cvs", "cvs_binary", 80,
     "Specify how to run the cvs program for adding newly-created files.  "
     "The default is simply \"cvs\".",
     &SoftCVS::dispatch_string, NULL, &_cvs_binary);
}


////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void SoftCVS::
run() {
  // First, check for the scenes directory.  If it doesn't exist, we
  // must not be in the root of a soft database.
  Filename scenes = "SCENES/.";
  if (!scenes.exists()) {
    nout << "No SCENES directory found; you are not in the root of a "
      "SoftImage database.\n";
    exit(1);
  }

  // Also, if we're expecting to use CVS, make sure the CVS directory
  // exists.
  Filename cvs_entries = "CVS/Entries";
  if (!_no_cvs && !cvs_entries.exists()) {
    nout << "You do not appear to be within a CVS-controlled source "
      "directory.\n";
    exit(1);
  }

  // Scan all the files in the database.
  traverse_root();

  // Collapse out the higher-versioned scene files.
  collapse_scene_files();

  // Now determine which element files are actually referenced by at
  // least one of the scene files.
  if (!get_scenes()) {
    exit(1);
  }

  // Finally, remove all the element files that are no longer
  // referenced by any scenes.
  remove_unused_elements();

  // Now do all the cvs adding and removing we need.
  if (!_no_cvs) {
    cvs_add_or_remove("remove", _cvs_remove);
    cvs_add_or_remove("add -kb", _cvs_add);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::traverse_root
//       Access: Private
//  Description: Reads all of the toplevel directory names,
//               e.g. SCENES, MATERIALS, etc., and traverses them.
////////////////////////////////////////////////////////////////////
void SoftCVS::
traverse_root() {
  Filename root(".");

  // Get the list of subdirectories.
  vector_string subdirs;
  if (!root.scan_directory(subdirs)) {
    nout << "Unable to scan directory.\n";
    return;
  }

  vector_string::const_iterator di;
  for (di = subdirs.begin(); di != subdirs.end(); ++di) {
    Filename subdir = (*di);
    if (subdir.is_directory() && subdir != "CVS") {
      traverse_subdir(subdir);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::traverse_subdir
//       Access: Private
//  Description: Reads the directory indicated by prefix and
//               identifies all of the SoftImage files stored there.
////////////////////////////////////////////////////////////////////
void SoftCVS::
traverse_subdir(const Filename &directory) {
  // Get the list of files in the directory.
  vector_string files;
  if (!directory.scan_directory(files)) {
    nout << "Unable to scan directory " << directory << "\n";
    return;
  }

  // We need to know the set of files in this directory that are CVS
  // elements.
  pset<string> cvs_elements;
  bool in_cvs = false;
  if (!_no_cvs) {
    in_cvs = scan_cvs(directory, cvs_elements);
  }

  bool is_scenes = false;
  bool keep_all = false;
  bool wants_cvs = false;

  // Now make some special-case behavior based on the particular
  // SoftImage subdirectory we're in.
  string dirname = directory.get_basename();
  if (dirname == "SCENES") {
    is_scenes = true;

  } else if (dirname == "CAMERAS") {
    // We don't want anything in the cameras directory.  These may
    // change arbitrarily and have no bearing on the model or
    // animation that we will extract, so avoid them altogether.
    return;

  } else if (dirname == "PICTURES") {
    // In the pictures directory, we must keep everything, since the
    // scene files don't explicitly reference these but they're still
    // important.  Textures that are no longer used will pile up; we
    // leave this as the user's problem.

    // We not only keep the textures, but we also move them into CVS,
    // since (again) they're not part of the scene files and thus
    // won't get added to the multifiles.  Also, some textures are
    // shared between different scenes, and it would be wasteful to
    // add them to each scene multifile; furthermore, some scenes are
    // used for animation only, and we don't want to modify these
    // multifiles when the textures change.

    keep_all = true;
    wants_cvs = !_no_cvs;
  }

  vector_string::const_iterator fi;
  for (fi = files.begin(); fi != files.end(); ++fi) {
    const string &filename = (*fi);
    if (filename == "CVS") {
      // This special filename is not to be considered.

    } else if (filename == "Chapter.rsrc") {
      // This special filename should not be considered, except to add
      // it to the multifiles.
      _global_files.push_back(Filename(directory, filename));

    } else {
      SoftFilename soft(directory, filename);

      if (cvs_elements.count(filename) != 0) {
        // This file is known to be in CVS.
        soft.set_in_cvs(true);
      }

      if (keep_all) {
        soft.increment_use_count();
      }
      if (wants_cvs && !in_cvs) {
        // Try to CVSify the directory.
        cvs_add(directory);
        in_cvs = true;
      }
      soft.set_wants_cvs(wants_cvs);

      if (is_scenes && soft.has_version() && soft.get_extension() == ".dsc") {
        _scene_files.push_back(soft);
      } else {
        _element_files.insert(soft);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::collapse_scene_files
//       Access: Private
//  Description: Walks through the list of scene files found, and
//               renames the higher-versioned ones to version 1-0,
//               removing the intervening versions.
////////////////////////////////////////////////////////////////////
void SoftCVS::
collapse_scene_files() {
  // Get a copy of the scene files vector so we can modify it.  Also
  // empty out the _scene_files at the same time so we can fill it up
  // again.
  SceneFiles versions;
  versions.swap(_scene_files);

  // And sort them into order so we can easily compare higher and
  // lower versions.
  sort(versions.begin(), versions.end());

  SceneFiles::iterator vi;
  vi = versions.begin();
  while (vi != versions.end()) {
    SoftFilename &file = (*vi);

    if (!file.is_1_0()) {
      // Here's a file that needs to be renamed.  But first, identify
      // all the other versions of the same file.
      SceneFiles::iterator start_vi;
      start_vi = vi;
      while (vi != versions.end() && (*vi).is_same_file(file)) {
        ++vi;
      }

      rename_file(start_vi, vi);

    } else {
      ++vi;
    }

    file.make_1_0();
    _scene_files.push_back(file);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::get_scenes
//       Access: Private
//  Description: Walks through the list of scene files and looks for
//               the set of element files referenced by each one,
//               updating multifile accordingly.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
get_scenes() {
  bool okflag = true;

  // We will be added the multifiles to CVS if they're not already
  // added, so we have to know which files are in CVS already.
  pset<string> cvs_elements;
  if (!_no_cvs) {
    scan_cvs(".", cvs_elements);
  }

  SceneFiles::const_iterator vi;
  for (vi = _scene_files.begin(); vi != _scene_files.end(); ++vi) {
    const SoftFilename &sf = (*vi);
    Filename file(sf.get_dirname(), sf.get_filename());

    file.set_text();
    ifstream in;
    if (!file.open_read(in)) {
      nout << "Unable to read " << file << "\n";
    } else {
      nout << "Scanning " << file << "\n";

      Multifile multifile;
      Filename multifile_name = sf.get_base() + "mf";

      if (!multifile.open_read_write(multifile_name)) {
        nout << "Unable to open " << multifile_name << " for updating.\n";
        okflag = false;

      } else {
        if (!scan_scene_file(in, multifile)) {
          okflag = false;
        }

        // Add all the global files to the multifile too.  These
        // probably can't take compression (since in SoftImage they're
        // just the Chapter.rsrc files, each very tiny).
        vector_string::const_iterator gi;
        for (gi = _global_files.begin(); gi != _global_files.end(); ++gi) {
          if (multifile.update_subfile((*gi), (*gi), 0).empty()) {
            nout << "Unable to add " << (*gi) << "\n";
            okflag = false;
          }
        }

        // Also add the scene file itself.
        if (multifile.update_subfile(file, file, 6).empty()) {
          nout << "Unable to add " << file << "\n";
          okflag = false;
        }

        bool flushed = false;
        if (multifile.needs_repack()) {
          flushed = multifile.repack();
        } else {
          flushed = multifile.flush();
        }
        if (!flushed) {
          nout << "Failed to write " << multifile_name << ".\n";
          okflag = false;
        } else {
          nout << "Wrote " << multifile_name << ".\n";

          if (!_no_cvs && cvs_elements.count(multifile_name) == 0) {
            // Add the multifile to CVS.
            _cvs_add.push_back(multifile_name);
          }
        }
      }
    }
  }

  return okflag;
}


////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::remove_unused_elements
//       Access: Private
//  Description: Remove all the element files that weren't referenced
//               by any scene file.  Also plan to cvs add all those
//               that were referenced.
////////////////////////////////////////////////////////////////////
void SoftCVS::
remove_unused_elements() {
  ElementFiles::const_iterator fi;
  for (fi = _element_files.begin(); fi != _element_files.end(); ++fi) {
    const SoftFilename &sf = (*fi);
    Filename file(sf.get_dirname(), sf.get_filename());

    if (sf.get_use_count() == 0) {
      nout << file << " is unused.\n";

      if (!file.unlink()) {
        nout << "Unable to remove " << file << ".\n";

      } else if (sf.get_in_cvs()) {
        _cvs_remove.push_back(file);
      }

    } else if (sf.get_wants_cvs() && !sf.get_in_cvs()) {
      _cvs_add.push_back(file);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::rename_file
//       Access: Private
//  Description: Renames the first file in the indicated list to a
//               version 1-0 filename, superceding all the other files
//               in the list.  Returns true if the file is renamed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
rename_file(SoftCVS::SceneFiles::iterator begin,
            SoftCVS::SceneFiles::iterator end) {
  int length = end - begin;
  nassertr(length > 0, false);

  SoftFilename &orig = (*begin);

  string dirname = orig.get_dirname();
  string source_filename = orig.get_filename();
  string dest_filename = orig.get_1_0_filename();

  if (length > 2) {
    nout << source_filename << " supercedes:\n";
    SceneFiles::const_iterator p;
    for (p = begin + 1; p != end; ++p) {
      nout << "  " << (*p).get_filename() << "\n";
    }

  } else if (length == 2) {
    nout << source_filename << " supercedes "
         << (*(begin + 1)).get_filename() << ".\n";

  } else {
    nout << source_filename << " renamed.\n";
  }

  // Now remove all of the "wrong" files.

  SceneFiles::const_iterator p;
  for (p = begin + 1; p != end; ++p) {
    Filename file((*p).get_dirname(), (*p).get_filename());
    if (!file.unlink()) {
      nout << "Unable to remove " << file << ".\n";
    }
  }

  // And rename the good one.
  Filename source(dirname, source_filename);
  Filename dest(dirname, dest_filename);

  if (!source.rename_to(dest)) {
    nout << "Unable to rename " << source << " to " << dest_filename << ".\n";
    exit(1);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::scan_cvs
//       Access: Private
//  Description: Scans the CVS repository in the indicated directory
//               to determine which files are already versioned
//               elements.  Returns true if the directory is
//               CVS-controlled, false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
scan_cvs(const string &dirname, pset<string> &cvs_elements) {
  Filename cvs_entries = dirname + "/CVS/Entries";
  if (!cvs_entries.exists()) {
    return false;
  }

  ifstream in;
  cvs_entries.set_text();
  if (!cvs_entries.open_read(in)) {
    nout << "Unable to read CVS directory.\n";
    return true;
  }

  string line;
  getline(in, line);
  while (!in.fail() && !in.eof()) {
    if (!line.empty() && line[0] == '/') {
      size_t slash = line.find('/', 1);
      if (slash != string::npos) {
        string filename = line.substr(1, slash - 1);

        if (line.substr(slash + 1, 2) == "-1") {
          // If the first number after the slash is -1, the file used
          // to be here but was recently cvs removed.  It counts as no
          // longer being an element.
        } else {
          cvs_elements.insert(filename);
        }
      }
    }

    getline(in, line);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::scan_scene_file
//       Access: Private
//  Description: Reads a scene file, looking for references to element
//               files.  For each reference found, increments the
//               appropriate element file's reference count.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
scan_scene_file(istream &in, Multifile &multifile) {
  bool okflag = true;
  
  int c = in.get();
  while (!in.eof() && !in.fail()) {
    // Skip whitespace.
    while (isspace(c) && !in.eof() && !in.fail()) {
      c = in.get();
    }

    // Now begin a word.
    string word;
    while (!isspace(c) && !in.eof() && !in.fail()) {
      word += c;
      c = in.get();
    }

    if (!word.empty()) {
      SoftFilename v("", word);

      // Increment the use count on all matching elements of the multiset.
      pair<ElementFiles::iterator, ElementFiles::iterator> range;
      range = _element_files.equal_range(v);

      ElementFiles::iterator ei;
      for (ei = range.first; ei != range.second; ++ei) {
        // We cheat and get a non-const reference to the filename out
        // of the set.  We can safely do this because incrementing the
        // use count won't change its position in the set.
        SoftFilename &sf = (SoftFilename &)(*ei);
        sf.increment_use_count();

        Filename file(sf.get_dirname(), sf.get_filename());
        if (multifile.update_subfile(file, file, 6).empty()) {
          nout << "Unable to add " << file << "\n";
          okflag = false;
        }
      }
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::cvs_add
//       Access: Private
//  Description: Invokes CVS to add just the named file to the
//               repository.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
cvs_add(const string &path) {
  string command = _cvs_binary + " add -kb " + path;
  nout << command << "\n";
  int result = system(command.c_str());

  if (result != 0) {
    nout << "Failure invoking cvs.\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::cvs_add_or_remove
//       Access: Private
//  Description: Invokes CVS to add (or remove) all of the files in
//               the indicated vector.  Returns true on success, false
//               on failure.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
cvs_add_or_remove(const string &cvs_command, const vector_string &paths) {
  static const int max_command = 4096;

  if (!paths.empty()) {
    string command = _cvs_binary + " " + cvs_command;
    vector_string::const_iterator pi;
    pi = paths.begin();
    while (pi != paths.end()) {
      const string &path = (*pi);

      if ((int)command.length() + 1 + (int)path.length() >= max_command) {
        // Fire off the command now.
        nout << command << "\n";
        int result = system(command.c_str());

        if (result != 0) {
          nout << "Failure invoking cvs.\n";
          return false;
        }

        command = _cvs_binary + " " + cvs_command;
      }

      command += ' ';
      command += path;

      ++pi;
    }
    nout << command << "\n";
    int result = system(command.c_str());

    if (result != 0) {
      nout << "Failure invoking cvs.\n";
      return false;
    }
  }
  return true;
}


int main(int argc, char *argv[]) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  SoftCVS prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
