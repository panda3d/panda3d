// Filename: ppMain.cxx
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppMain.h"
#include "ppScope.h"
#include "ppCommandFile.h"
#include "ppDirectory.h"

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h> // for perror

////////////////////////////////////////////////////////////////////
//     Function: PPMain::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPMain::
PPMain(PPScope *global_scope) {
  _global_scope = global_scope;
  PPScope::push_scope(_global_scope);

  _def_scope = (PPScope *)NULL;
  _defs = (PPCommandFile *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PPMain::
~PPMain() {
  if (_def_scope != (PPScope *)NULL) {
    delete _def_scope;
  }
  if (_defs != (PPCommandFile *)NULL) {
    delete _defs;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::read_source
//       Access: Public
//  Description: Reads the directory hierarchy of Sources.pp files, at
//               the indicated directory and below.
////////////////////////////////////////////////////////////////////
bool PPMain::
read_source(const string &root) {
  // First, find the top of the source tree, as indicated by the
  // presence of a Package.pp file.
  string trydir = root;

  string package_file = trydir + "/" + PACKAGE_FILENAME;

  while (access(package_file.c_str(), F_OK) != 0) {
    // We continue to walk up directories as long as we see a source
    // file in each directory.  When we stop seeing source files, we
    // stop walking upstairs.
    string source_file = trydir + "/" + SOURCE_FILENAME;
    if (access(source_file.c_str(), F_OK) != 0) {
      cerr << "Could not find ppremake package file " << PACKAGE_FILENAME
	   << ".\n\n"
	   << "This file should be present in the top of the source directory tree;\n"
	   << "it defines implementation-specific variables to control the output\n"
	   << "of ppremake, as well as pointing out the installed location of\n"
	   << "important ppremake config files.\n\n";
      return false;
    }
    trydir += "/..";
    package_file = trydir + "/" + PACKAGE_FILENAME;
  }

  // Now cd to the source root and get the actual path.
  if (chdir(trydir.c_str()) < 0) {
    perror("chdir");
    return false;
  }

  string cwd = get_cwd();
  cerr << "Root is " << cwd << "\n";

  _def_scope = new PPScope(&_named_scopes);
  _def_scope->define_variable("PACKAGEFILE", package_file);
  _def_scope->define_variable("TOPDIR", cwd);
  _defs = new PPCommandFile(_def_scope);

  if (!_defs->read_file(PACKAGE_FILENAME)) {
    return false;
  }

  PPScope::push_scope(_def_scope);

  if (!_tree.scan_source(&_named_scopes)) {
    return false;
  }

  _def_scope->define_variable("TREE", _tree.get_complete_tree());

  if (_tree.count_source_files() == 0) {
    cerr << "Could not find any source definition files named " << SOURCE_FILENAME
	 << ".\n\n"
	 << "A file by this name should be present in each directory of the source\n"
	 << "hierarchy; it defines the source files and targets that should be\n"
	 << "built in each directory, as well as the relationships between the\n"
	 << "directories.\n\n";
    return false;
  }

  cerr << "Read " << _tree.count_source_files() << " " << SOURCE_FILENAME
       << " files.\n";

  if (!read_global_file()) {
    return false;
  }

  if (!_tree.scan_depends(&_named_scopes)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::process_all
//       Access: Public
//  Description: Does all the processing on all known directories.
//               See process().
////////////////////////////////////////////////////////////////////
bool PPMain::
process_all() {
  string cache_filename = _def_scope->expand_variable("DEPENDENCY_CACHE_FILENAME");

  if (cache_filename.empty()) {
    cerr << "Warning: no definition given for $[DEPENDENCY_CACHE_FILENAME].\n";
  } else {
    _tree.read_file_dependencies(cache_filename);
  }

  if (!r_process_all(_tree.get_root())) {
    return false;
  }

  if (!cache_filename.empty()) {
    _tree.update_file_dependencies(cache_filename);
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::process
//       Access: Public
//  Description: Does the processing associated with the source file
//               in the indicated subdirectory name.  This involves
//               reading in the template file and generating whatever
//               output the template file indicates.
////////////////////////////////////////////////////////////////////
bool PPMain::
process(const string &dirname) {
  PPDirectory *dir = _tree.find_dirname(dirname);
  if (dir == (PPDirectory *)NULL) {
    cerr << "Unknown directory: " << dirname << "\n";
    return false;
  }

  if (dir->get_source() == (PPCommandFile *)NULL) {
    cerr << "No source file in " << dirname << "\n";
    return false;
  }

  return p_process(dir);
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::report_depends
//       Access: Public
//  Description: Reports all the directories that the named directory
//               depends on.
////////////////////////////////////////////////////////////////////
void PPMain::
report_depends(const string &dirname) const {
  PPDirectory *dir = _tree.find_dirname(dirname);
  if (dir == (PPDirectory *)NULL) {
    cerr << "Unknown directory: " << dirname << "\n";
    return;
  }

  dir->report_depends();
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::report_needs
//       Access: Public
//  Description: Reports all the directories that depend on (need) the
//               named directory.
////////////////////////////////////////////////////////////////////
void PPMain::
report_needs(const string &dirname) const {
  PPDirectory *dir = _tree.find_dirname(dirname);
  if (dir == (PPDirectory *)NULL) {
    cerr << "Unknown directory: " << dirname << "\n";
    return;
  }

  dir->report_needs();
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::r_process_all
//       Access: Private
//  Description: The recursive implementation of process_all().
////////////////////////////////////////////////////////////////////
bool PPMain::
r_process_all(PPDirectory *dir) {
  if (dir->get_source() != (PPCommandFile *)NULL) {
    if (!p_process(dir)) {
      return false;
    }
  }

  int num_children = dir->get_num_children();
  for (int i = 0; i < num_children; i++) {
    if (!r_process_all(dir->get_child(i))) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::p_process
//       Access: Private
//  Description: The private implementation of process().
////////////////////////////////////////////////////////////////////
bool PPMain::
p_process(PPDirectory *dir) {
  current_output_directory = dir;
  _named_scopes.set_current(dir->get_dirname());
  PPCommandFile *source = dir->get_source();
  assert(source != (PPCommandFile *)NULL);

  PPScope *scope = source->get_scope();

  string template_filename = scope->expand_variable("TEMPLATE_FILE");
  if (template_filename.empty()) {
    cerr << "No definition given for $[TEMPLATE_FILE], cannot process.\n";
    return false;
  }

  PPCommandFile template_file(scope);
  if (!template_file.read_file(template_filename)) {
    cerr << "Error reading template file " << template_filename << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::read_global_file
//       Access: Private
//  Description: Reads in the Global.pp file after all sources files
//               have been read and sorted into dependency order.
////////////////////////////////////////////////////////////////////
bool PPMain::
read_global_file() {
  assert(_def_scope != (PPScope *)NULL);

  string global_filename = _def_scope->expand_variable("GLOBAL_FILE");
  if (global_filename.empty()) {
    cerr << "No definition given for $[GLOBAL_FILE], cannot process.\n";
    return false;
  }
  
  PPCommandFile global(_def_scope);
  if (!global.read_file(global_filename)) {
    cerr << "Error reading global definition file "
	 << global_filename << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPMain::get_cwd
//       Access: Private, Static
//  Description: Calls the system getcwd(), automatically allocating a
//               large enough string.
////////////////////////////////////////////////////////////////////
string PPMain::
get_cwd() {
  static size_t bufsize = 1024;
  static char *buffer = NULL;

  if (buffer == (char *)NULL) {
    buffer = new char[bufsize];
  }

  while (getcwd(buffer, bufsize) == (char *)NULL) {
    if (errno != ERANGE) {
      perror("getcwd");
      return string();
    }
    delete[] buffer;
    bufsize = bufsize * 2;
    buffer = new char[bufsize];
    assert(buffer != (char *)NULL);
  }

  return string(buffer);
}
