// Filename: ppMain.cxx
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppMain.h"
#include "ppScope.h"
#include "ppCommandFile.h"

#include <unistd.h>

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
  // First, read the package file.  We find this either in this
  // directory, or in some directory above us.
  string search = root + "/";
  if (root == ".") {
    search = "";
  }

  string package_file = search + PACKAGE_FILENAME;

  while (access(package_file.c_str(), F_OK) != 0) {
    // We continue to walk up directories as long as we see a source
    // file in each directory.  When we stop seeing source files, we
    // stop walking upstairs.
    string source_file = search + SOURCE_FILENAME;
    if (access(source_file.c_str(), F_OK) != 0) {
      cerr << "Could not find ppremake package file " << PACKAGE_FILENAME
	   << ".\n\n"
	   << "This file should be present in the top of the source directory tree;\n"
	   << "it defines implementation-specific variables to control the output\n"
	   << "of ppremake, as well as pointing out the installed location of\n"
	   << "important ppremake config files.\n\n";
      return false;
    }
    search += "../";
    package_file = search + PACKAGE_FILENAME;
  }

  _def_scope = new PPScope(&_named_scopes);
  _def_scope->define_variable("PACKAGEFILE", package_file);
  _def_scope->define_variable("TOPDIRPREFIX", search);
  _defs = new PPCommandFile(_def_scope);

  //  cerr << "Reading " << package_file << "\n";
  if (!_defs->read_file(package_file)) {
    cerr << "Error reading package file " << package_file << ".\n";
    return false;
  }

  PPScope::push_scope(_def_scope);

  if (!_tree.scan(search, &_named_scopes)) {
    return false;
  }

  _def_scope->define_variable("TREE", _tree.get_complete_subtree());

  if (_tree.count_source_files() == 0) {
    cerr << "Could not find any source definition files named " << SOURCE_FILENAME
	 << ".\n\n"
	 << "A file by this name should be present in each directory of the source\n"
	 << "hierarchy; it defines the source files and targets that should be\n"
	 << "built in each directory, as well as the relationships between the\n"
	 << "directories.\n\n";
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
  return r_process_all(&_tree);
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
  PPDirectoryTree *dir = _tree.find_dirname(dirname);
  if (dir == (PPDirectoryTree *)NULL) {
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
//     Function: PPMain::r_process_all
//       Access: Private
//  Description: The recursive implementation of process_all().
////////////////////////////////////////////////////////////////////
bool PPMain::
r_process_all(PPDirectoryTree *dir) {
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
p_process(PPDirectoryTree *dir) {
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
