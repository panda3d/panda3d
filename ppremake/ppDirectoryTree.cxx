// Filename: ppDirectoryTree.cxx
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppDirectoryTree.h"
#include "ppScope.h"
#include "ppNamedScopes.h"
#include "ppCommandFile.h"
#include "tokenize.h"

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>

PPDirectoryTree *current_output_directory = (PPDirectoryTree *)NULL;

// An STL object to sort directories in order by dependency and then
// by name, used in get_child_dirnames().
class SortDirectoriesByDependencyAndName {
public:
  bool operator () (const PPDirectoryTree *a, const PPDirectoryTree *b) const {
    if (a->get_depends_index() != b->get_depends_index()) {
      return a->get_depends_index() < b->get_depends_index();
    }
    return a->get_dirname() < b->get_dirname();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::Constructor
//       Access: Public
//  Description: Creates the root level of the PPDirectoryTree.
////////////////////////////////////////////////////////////////////
PPDirectoryTree::
PPDirectoryTree() {
  _scope = (PPScope *)NULL;
  _source = (PPCommandFile *)NULL;
  _parent = (PPDirectoryTree *)NULL;
  _depth = 0;
  _depends_index = 0;
  _computing_depends_index = false;
  _dirnames = new Dirnames;

  _dirname = "top";
  (*_dirnames).insert(Dirnames::value_type(_dirname, this));
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::Destructor
//       Access: Public
//  Description: When a tree root destructs, all of its children are
//               also destroyed.
////////////////////////////////////////////////////////////////////
PPDirectoryTree::
~PPDirectoryTree() {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    delete (*ci);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::Constructor
//       Access: Protected
//  Description: Creates a new directory level that automatically adds
//               itself to its parent's children list.
////////////////////////////////////////////////////////////////////
PPDirectoryTree::
PPDirectoryTree(const string &dirname, PPDirectoryTree *parent) :
  _dirname(dirname),
  _parent(parent)
{
  assert(_parent != (PPDirectoryTree *)NULL);
  _scope = (PPScope *)NULL;
  _source = (PPCommandFile *)NULL;
  _parent->_children.push_back(this);
  _depth = _parent->_depth + 1;
  _depends_index = 0;
  _computing_depends_index = false;
  _dirnames = _parent->_dirnames;

  bool inserted = 
    (*_dirnames).insert(Dirnames::value_type(_dirname, this)).second;
  if (!inserted) {
    cerr << "Warning: multiple directories encountered named "
	 << _dirname << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::scan
//       Access: Public
//  Description: Reads in the complete hierarchy of source files.
//               prefix is the pathname to the directory on disk,
//               ending in slash.
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
scan(const string &prefix, PPNamedScopes *named_scopes) {
  if (!r_scan(prefix)) {
    return false;
  }

  if (!read_source_file(prefix, named_scopes)) {
    return false;
  }

  if (!read_depends_file(named_scopes)) {
    return false;
  }

  if (!resolve_dependencies()) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::count_source_files
//       Access: Public
//  Description: Returns the number of directories within the tree
//               that actually have a Sources.pp file that was read.
////////////////////////////////////////////////////////////////////
int PPDirectoryTree::
count_source_files() const {
  int count = 0;
  if (_source != (PPCommandFile *)NULL) {
    count++;
  }

  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    count += (*ci)->count_source_files();
  }

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_dirname
//       Access: Public
//  Description: Returns the name of this particular directory level.
////////////////////////////////////////////////////////////////////
const string &PPDirectoryTree::
get_dirname() const {
  return _dirname;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_depends_index
//       Access: Public
//  Description: Returns the dependency index associated with this
//               directory.  It is generally true that if directory A
//               depends on B, then A.get_depends_index() >
//               B.get_depends_index().
////////////////////////////////////////////////////////////////////
int PPDirectoryTree::
get_depends_index() const {
  return _depends_index;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_path
//       Access: Public
//  Description: Returns the relative path from the root to this
//               particular directory.  This does not include the root
//               name itself, and does not include a trailing slash.
////////////////////////////////////////////////////////////////////
string PPDirectoryTree::
get_path() const {
  if (_parent == (PPDirectoryTree *)NULL) {
    return ".";
  }
  if (_parent->_parent == (PPDirectoryTree *)NULL) {
    return _dirname;
  }
  return _parent->get_path() + "/" + _dirname;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_rel_to
//       Access: Public
//  Description: Returns the relative path to the other directory from
//               this one.  This does not include a trailing slash.
////////////////////////////////////////////////////////////////////
string PPDirectoryTree::
get_rel_to(const PPDirectoryTree *other) const {
  const PPDirectoryTree *a = this;
  const PPDirectoryTree *b = other;

  if (a == b) {
    return ".";
  }

  string prefix, postfix;
  while (a->_depth > b->_depth) {
    prefix += "../";
    a = a->_parent;
    assert(a != (PPDirectoryTree *)NULL);
  }

  while (b->_depth > a->_depth) {
    postfix = b->_dirname + "/" + postfix;
    b = b->_parent;
    assert(b != (PPDirectoryTree *)NULL);
  }

  while (a != b) {
    prefix += "../";
    postfix = b->_dirname + "/" + postfix;
    a = a->_parent;
    b = b->_parent;
    assert(a != (PPDirectoryTree *)NULL);
    assert(b != (PPDirectoryTree *)NULL);
  }

  string result = prefix + postfix;
  assert(!result.empty());
  return result.substr(0, result.length() - 1);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::find_dirname
//       Access: Public
//  Description: Searches for the first subdirectory found with the
//               matching dirname.  This is just the name of the
//               directory itself, not the relative path to the
//               directory.
////////////////////////////////////////////////////////////////////
PPDirectoryTree *PPDirectoryTree::
find_dirname(const string &dirname) {
  assert(_dirnames != (Dirnames *)NULL);
  Dirnames::const_iterator di;
  di = _dirnames->find(dirname);
  if (di != _dirnames->end()) {
    return (*di).second;
  }

  // No such dirname; too bad.
  return (PPDirectoryTree *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_source
//       Access: Public
//  Description: Returns the source file associated with this level
//               of the directory hierarchy.  This *might* be NULL.
////////////////////////////////////////////////////////////////////
PPCommandFile *PPDirectoryTree::
get_source() const {
  return _source;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_num_children
//       Access: Public
//  Description: Returns the number of subdirectories below this
//               level.
////////////////////////////////////////////////////////////////////
int PPDirectoryTree::
get_num_children() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_child
//       Access: Public
//  Description: Returns the nth subdirectory below this level.
////////////////////////////////////////////////////////////////////
PPDirectoryTree *PPDirectoryTree::
get_child(int n) const {
  assert(n >= 0 && n < (int)_children.size());
  return _children[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_child_dirnames
//       Access: Public
//  Description: Returns a single string listing the names of all the
//               subdirectories of this level, delimited by spaces.
//
//               The list is sorted in dependency order such that a
//               directory is listed after the other directories it
//               might depend on.
////////////////////////////////////////////////////////////////////
string PPDirectoryTree::
get_child_dirnames() const {
  Children copy_children = _children;
  sort(copy_children.begin(), copy_children.end(),
       SortDirectoriesByDependencyAndName());

  vector<string> words;
  Children::const_iterator ci;
  for (ci = copy_children.begin(); ci != copy_children.end(); ++ci) {
    words.push_back((*ci)->get_dirname());
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_complete_subtree
//       Access: Public
//  Description: Returns a single string listing the relative path
//               from the source root to each source directory at this
//               level and below, delimited by spaces.
////////////////////////////////////////////////////////////////////
string PPDirectoryTree::
get_complete_subtree() const {
  Children copy_children = _children;
  sort(copy_children.begin(), copy_children.end(),
       SortDirectoriesByDependencyAndName());

  vector<string> words;
  words.push_back(get_path());

  Children::const_iterator ci;
  for (ci = copy_children.begin(); ci != copy_children.end(); ++ci) {
    words.push_back((*ci)->get_complete_subtree());
  }

  string result = repaste(words, " ");
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::r_scan
//       Access: Private
//  Description: The recursive implementation of scan().
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
r_scan(const string &prefix) {
  string root_name = ".";
  if (!prefix.empty()) {
    root_name = prefix.substr(0, prefix.length() - 1);
  }
  
  DIR *root = opendir(root_name.c_str());
  if (root == (DIR *)NULL) {
    cerr << "Unable to scan directory " << root_name << "\n";
    return false;
  }

  struct dirent *d;
  d = readdir(root);
  while (d != (struct dirent *)NULL) {
    string dirname = d->d_name;

    if (!dirname.empty() && dirname[0] != '.') {
      // Do we have a source file in this subdirectory (if it is a
      // subdirectory)?
      string next_prefix = prefix + dirname + "/";
      string source_filename = next_prefix + SOURCE_FILENAME;
      if (access(source_filename.c_str(), F_OK) == 0) {
	PPDirectoryTree *subtree = new PPDirectoryTree(dirname, this);

	if (!subtree->r_scan(next_prefix)) {
	  return false;
	}
      }
    }

    d = readdir(root);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::read_source_file
//       Access: Private
//  Description: Recursively reads in the source file at each level,
//               if defined.
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
read_source_file(const string &prefix, PPNamedScopes *named_scopes) {
  string source_filename = prefix + SOURCE_FILENAME;

  ifstream in(source_filename.c_str());
  if (in) {
    //    cerr << "Reading " << source_filename << "\n";

    named_scopes->set_current(_dirname);
    _scope = named_scopes->make_scope("");
    
    _scope->define_variable("SOURCEFILE", SOURCE_FILENAME);
    _scope->define_variable("DIRNAME", _dirname);
    _scope->define_variable("DIRPREFIX", prefix);
    _scope->define_variable("PATH", get_path());
    _scope->define_variable("SUBDIRS", get_child_dirnames());
    _scope->define_variable("SUBTREE", get_complete_subtree());
    _scope->set_directory(this);
    
    _source = new PPCommandFile(_scope);
    
    if (!_source->read_stream(in)) {
      cerr << "Error when reading " << source_filename << "\n";
      return false;
    }
  }
    
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if (!(*ci)->read_source_file(prefix + (*ci)->get_dirname() + "/",
				  named_scopes)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::read_depends_file
//       Access: Private
//  Description: Recursively reads in the dependency definition file
//               for each source file.
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
read_depends_file(PPNamedScopes *named_scopes) {
  if (_scope != (PPScope *)NULL) {
    // Read the depends file, so we can determine the relationship
    // between this source file and all of the other source files.

    string depends_filename = _scope->expand_variable("DEPENDS_FILE");
    if (depends_filename.empty()) {
      cerr << "No definition given for $[DEPENDS_FILE], cannot process.\n";
      return false;
    }
    
    named_scopes->set_current(_dirname);
    PPCommandFile depends(_scope);
    if (!depends.read_file(depends_filename)) {
      cerr << "Error reading dependency definition file "
	   << depends_filename << ".\n";
      return false;
    }
    
    // This should have defined the variable DEPENDS, which lists the
    // various dirnames this source file depends on.

    vector<string> dirnames;
    tokenize_whitespace(_scope->expand_variable("DEPENDS"), dirnames);

    vector<string>::const_iterator ni;
    for (ni = dirnames.begin(); ni != dirnames.end(); ++ni) {
      const string &dirname = (*ni);
      PPDirectoryTree *dir = find_dirname(dirname);
      if (dir == (PPDirectoryTree *)NULL) {
	cerr << "Could not find dependent dirname " << dirname << "\n";
      } else {
	if (dir != this) {
	  _i_depend_on.insert(dir);
	  dir->_depends_on_me.insert(this);
	}
      }
    }
  }
    
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if (!(*ci)->read_depends_file(named_scopes)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::resolve_dependencies
//       Access: Private
//  Description: Visits each directory and assigns a correct
//               _depends_index to each one, such that if directory A
//               depends on directory B then A._depends_index >
//               B._depends_index.
//
//               This also detects cycles in the directory dependency
//               graph.
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
resolve_dependencies() {
  if (!compute_depends_index()) {
    return false;
  }

  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if (!(*ci)->resolve_dependencies()) {
      return false;
    }
  }

  // Now that we've resolved all of our children's dependencies,
  // redefine our SUBDIRS and SUBTREE variables to put things in the
  // right order.
  if (_scope != (PPScope *)NULL) {
    _scope->define_variable("SUBDIRS", get_child_dirnames());
    _scope->define_variable("SUBTREE", get_complete_subtree());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::compute_depends_index
//       Access: Private
//  Description: Computes the dependency score for a particular
//               directory.  See resolve_dependencies().
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
compute_depends_index() {
  if (_depends_index != 0) {
    return true;
  }

  if (_i_depend_on.empty()) {
    _depends_index = 1;
    return true;
  }

  _computing_depends_index = true;
  int max_index = 0;

  Depends::iterator di;
  for (di = _i_depend_on.begin(); di != _i_depend_on.end(); ++di) {
    if ((*di)->_computing_depends_index) {
      // Oops, we have a cycle!
      cerr << "Cycle detected in inter-directory dependencies!\n"
	   << _dirname << " depends on " << (*di)->_dirname << "\n";
      return false;
    }
      
    if (!(*di)->compute_depends_index()) {
      // Keep reporting the cycle as we unroll the recursion.
      cerr << _dirname << " depends on " << (*di)->_dirname << "\n";
      return false;
    }

    max_index = max(max_index, (*di)->_depends_index);
  }

  _computing_depends_index = false;
  _depends_index = max_index + 1;

  return true;
}
