// Filename: softCVS.cxx
// Created by:  drose (10Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "softCVS.h"

#include <filename.h>
#include <notify.h>
#include <vector_string.h>

#include <algorithm>

#ifdef WIN32_VC
// Windows uses a different API for scanning for files in a directory.
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#else
#include <sys/types.h>
#include <dirent.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftCVS::
SoftCVS() {
  _cvs_binary = "cvs";

  set_program_description
    ("softcvs scrubs over a SoftImage database that was recently copied "
     "into a CVS-controlled directory and prepares it for cvs updating.  "
     "It eliminates SoftImage's silly filename-based versioning system by "
     "renaming versioned filenames higher than 1-0 back to version 1-0 "
     "(thus overwriting the previous file version 1-0).  This allows CVS "
     "to manage the versioning rather than having to change the filename "
     "with each new version.  This program also automatically adds each "
     "new file to the CVS repository.\n\n"

     "You must run this from within the root of a SoftImage database "
     "directory; e.g. the directory that contains SCENES, PICTURES, MODELS, "
     "and so on.");

  clear_runlines();
  add_runline("[opts]");

  add_option
    ("i", "", 80,
     "Prompt the user for confirmation before every operation.",
     &SoftCVS::dispatch_none, &_interactive);

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

  // Begin the traversal.
  traverse(".");

  // Now consider adjusting the scene files.
  set<string>::iterator si;
  for (si = _scene_files.begin(); si != _scene_files.end(); ++si) {
    consider_scene_file(*si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::traverse
//       Access: Private
//  Description: Reads the directory indicated by prefix, looking for
//               files that are named something like *.2-0.ext,
//               and renames these to *.1-0.ext.
////////////////////////////////////////////////////////////////////
void SoftCVS::
traverse(const string &dirname) {
  // Get the list of files in the directory.
  vector_string files;

  DIR *root = opendir(dirname.c_str());
  if (root == (DIR *)NULL) {
    nout << "Unable to scan directory " << dirname << "\n";
  }

  struct dirent *d;
  d = readdir(root);
  while (d != (struct dirent *)NULL) {
    files.push_back(d->d_name);
    d = readdir(root);
  }
  closedir(root);

  // Now go through and identify files with version numbers, and
  // collect together those files that are different versions of the
  // same file.
  vector<SoftFilename> versions;
  vector_string::const_iterator fi;
  for (fi = files.begin(); fi != files.end(); ++fi) {
    const string &filename = (*fi);
    if (!filename.empty() && filename[0] != '.') {
      SoftFilename v(filename);
      if (v.has_version()) {
	versions.push_back(v);
      } else {
	// Maybe this is a subdirectory?
	Filename subdir = dirname + "/" + filename;
	if (subdir.is_directory()) {
	  traverse(subdir);
	}
      }
    }
  }

  if (!versions.empty()) {
    // We actually have some versioned filenames in this directory.
    // We'll therefore need to know the set of files that are CVS
    // elements.
    set<string> cvs_elements;
    bool in_cvs = false;
    if (!_no_cvs) {
      in_cvs = scan_cvs(dirname, cvs_elements);
    }

    // Now sort the versioned filenames in order so we can scan for
    // higher versions.
    sort(versions.begin(), versions.end());
    
    vector<SoftFilename>::iterator vi;
    vi = versions.begin();
    while (vi != versions.end()) {
      SoftFilename &file = (*vi);
      _versioned_files.insert(file.get_base());

      if (!file.is_1_0()) {
	// Here's a file that needs to be renamed.  But first, identify
	// all the other versions of the same file.
	vector<SoftFilename>::iterator start_vi;
	start_vi = vi;
	while (vi != versions.end() && (*vi).is_same_file(file)) {
	  ++vi;
	}
	
	if (rename_file(dirname, start_vi, vi)) {
	  if (in_cvs) {
	    consider_add_cvs(dirname, file.get_1_0_filename(), cvs_elements);
	  }

	  if (file.get_extension() == ".dsc") {
	    _scene_files.insert(dirname + "/" + file.get_1_0_filename());
	  }
	}
	
      } else {
	if (in_cvs) {
	  consider_add_cvs(dirname, file.get_filename(), cvs_elements);
	}

	if (file.get_extension() == ".dsc") {
	  _scene_files.insert(dirname + "/" + file.get_filename());
	}
	++vi;
      }
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
rename_file(const string &dirname,
	    vector<SoftFilename>::const_iterator begin, 
	    vector<SoftFilename>::const_iterator end) {
  int length = end - begin;
  nassertr(length > 0, false);

  string source_filename = (*begin).get_filename();
  string dest_filename = (*begin).get_1_0_filename();

  if (length > 2) {
    nout << source_filename << " supercedes:\n";
    vector<SoftFilename>::const_iterator p;
    for (p = begin + 1; p != end; ++p) {
      nout << "  " << (*p).get_filename() << "\n";
    }

  } else if (length == 2) {
    nout << source_filename << " supercedes " 
	 << (*(begin + 1)).get_filename() << ".\n";

  } else {
    if (_interactive) {
      nout << source_filename << " needs renaming.\n";
    } else {
      nout << source_filename << " renamed.\n";
    }
  }

  if (_interactive) {
    if (!prompt_yesno("Rename this file (y/n)? ")) {
      return false;
    }
  }

  // Now remove all of the "wrong" files.
  vector<SoftFilename>::const_iterator p;
  for (p = begin + 1; p != end; ++p) {
    Filename file = dirname + "/" + (*p).get_filename();
    if (!file.unlink()) {
      nout << "Unable to remove " << file << ".\n";
    }
  }

  // And rename the good one.
  Filename source = dirname + "/" + source_filename;
  Filename dest = dirname + "/" + dest_filename;
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
scan_cvs(const string &dirname, set<string> &cvs_elements) {
  Filename cvs_entries = dirname + "/CVS/Entries";
  if (!cvs_entries.exists()) {
    // Try to CVSify the directory.
    if (_interactive) {
      nout << "Directory " << dirname << " is not CVS-controlled.\n";
      if (!prompt_yesno("Add the directory to CVS (y/n)? ")) {
	return false;
      }
    }

    if (!cvs_add(dirname)) {
      return false;
    }
  }

  ifstream in;
  cvs_entries.set_text();
  if (!cvs_entries.open_read(in)) {
    cerr << "Unable to read CVS directory.\n";
    return true;
  }

  string line;
  getline(in, line);
  while (!in.fail() && !in.eof()) {
    if (!line.empty() && line[0] == '/') {
      size_t slash = line.find('/', 1);
      if (slash != string::npos) {
	string filename = line.substr(1, slash - 1);
	cvs_elements.insert(filename);
      }
    }

    getline(in, line);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::consider_add_cvs
//       Access: Private
//  Description: Considers adding the indicated file to the CVS
//               repository, if it is not already there.
////////////////////////////////////////////////////////////////////
void SoftCVS::
consider_add_cvs(const string &dirname, const string &filename,
		 const set<string> &cvs_elements) {
  if (cvs_elements.count(filename) != 0) {
    // Already in CVS!
    return;
  }

  string path = dirname + "/" + filename;

  if (_interactive) {
    if (!prompt_yesno("Add " + path + " to CVS (y/n)? ")) {
      return;
    }
  }
  
  cvs_add(path);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::consider_scene_file
//       Access: Private
//  Description: Checks to see if the indicated file is a scene file,
//               and that it contains references to a higher-version
//               filename.  If so, offers to adjust it.
////////////////////////////////////////////////////////////////////
void SoftCVS::
consider_scene_file(Filename path) {
  path.set_text();
  ifstream in;
  if (!path.open_read(in)) {
    nout << "Could not read " << path << ".\n";
    return;
  }
  
  // Scan the scene file into memory.
  ostringstream scene;
  if (!scan_scene_file(in, scene)) {
    // The scene file doesn't need to change.
    return;
  }
  
  // The scene file should change.
  if (_interactive) {
    nout << "Scene file " << path << " needs to be updated.\n";
    if (!prompt_yesno("Modify this file (y/n)? ")) {
      return;
    }
  }
  
  // Rewrite the scene file.
  in.close();
  path.unlink();
  ofstream out;
  if (!path.open_write(out)) {
    nout << "Could not write " << path << ".\n";
    return;
  }
  
  string data = scene.str();
  out.write(data.data(), data.length());
  
  if (out.fail()) {
    nout << "Error writing " << path << ".\n";
    return;
  }
  nout << "Updated scene file " << path << ".\n";
}


////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::scan_scene_file
//       Access: Private
//  Description: Copies a scene file from the input stream to the
//               output stream, looking for stale file references
//               (i.e. filenames whose version number is greater than
//               1-0).  If any such filenames are found, replaces them
//               with the equivalent 1-0 filename, and returns true;
//               otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
scan_scene_file(istream &in, ostream &out) {
  bool any_changed = false;
  int c;

  c = in.get();
  while (!in.eof() && !in.fail()) {
    // Skip whitespace.
    while (isspace(c) && !in.eof() && !in.fail()) {
      out.put(c);
      c = in.get();
    }

    // Now begin a word.
    string word;
    while (!isspace(c) && !in.eof() && !in.fail()) {
      word += c;
      c = in.get();
    }

    if (!word.empty()) {
      // Here's the name of a "versioned" element.  Should we rename
      // it?  Only if the version is not 1-0, and this kind of element
      // is versioned by filename.  (Some elements are not versioned
      // by filename; instead, they keep the same filename but store
      // multiple versions within themselves.  Trouble.)
      SoftFilename v(word);
      if (v.has_version() && !v.is_1_0() &&
	  _versioned_files.count(v.get_base()) != 0) {
	out << v.get_1_0_filename();
	any_changed = true;
      } else {
	out << word;
      }
    }
  }

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::cvs_add
//       Access: Private
//  Description: Invokes CVS to add the file to the repository.
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
cvs_add(const string &path) {
  string command = _cvs_binary + " add " + path;
  nout << command << "\n";
  int result = system(command.c_str());

  if (result != 0) {
    nout << "Failure invoking cvs.\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::prompt_yesno
//       Access: Private
//  Description: Asks the user a yes-or-no question.  Returns true if
//               the answer is yes, false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftCVS::
prompt_yesno(const string &message) {
  while (true) {
    string result = prompt(message);
    nassertr(!result.empty(), false);
    if (result.size() == 1) {
      if (tolower(result[0]) == 'y') {
	return true;
      } else if (tolower(result[0]) == 'n') {
	return false;
      }
    }

    nout << "*** Invalid response: " << result << "\n\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftCVS::prompt
//       Access: Private
//  Description: Issues a prompt to the user and waits for a typed
//               response.  Returns the response (which will not be
//               empty).
////////////////////////////////////////////////////////////////////
string SoftCVS::
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


int main(int argc, char *argv[]) {
  SoftCVS prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
