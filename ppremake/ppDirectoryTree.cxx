// Filename: ppDirectoryTree.cxx
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#include "ppDirectoryTree.h"
#include "ppDirectory.h"

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PPDirectoryTree::
PPDirectoryTree() {
  _root = new PPDirectory(this);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PPDirectoryTree::
~PPDirectoryTree() {
  delete _root;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::scan_source
//       Access: Public
//  Description: Reads in the complete hierarchy of source files,
//               beginning at the current directory.
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
scan_source(PPNamedScopes *named_scopes) {
  if (!_root->r_scan("")) {
    return false;
  }

  if (!_root->read_source_file("", named_scopes)) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::scan_depends
//       Access: Public
//  Description: Reads in the depends file for each source file, and
//               then sorts the files into dependency order.
////////////////////////////////////////////////////////////////////
bool PPDirectoryTree::
scan_depends(PPNamedScopes *named_scopes) {
  if (!_root->read_depends_file(named_scopes)) {
    return false;
  }

  if (!_root->resolve_dependencies()) {
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
  return _root->count_source_files();
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_root
//       Access: Public
//  Description: Returns the root directory of the tree.
////////////////////////////////////////////////////////////////////
PPDirectory *PPDirectoryTree::
get_root() const {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_complete_tree
//       Access: Public
//  Description: Returns a single string listing the relative path
//               from the source root to each source directory in the
//               tree, delimited by spaces.
////////////////////////////////////////////////////////////////////
string PPDirectoryTree::
get_complete_tree() const {
  return _root->get_complete_subtree();
}


////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::find_dirname
//       Access: Public
//  Description: Searches for the a source directory with the
//               matching dirname.  This is just the name of the
//               directory itself, not the relative path to the
//               directory.
////////////////////////////////////////////////////////////////////
PPDirectory *PPDirectoryTree::
find_dirname(const string &dirname) const {
  Dirnames::const_iterator di;
  di = _dirnames.find(dirname);
  if (di != _dirnames.end()) {
    return (*di).second;
  }

  // No such dirname; too bad.
  return (PPDirectory *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::find_dependable_file
//       Access: Public
//  Description: Returns a PPDependableFile object corresponding to
//               the named filename, searching all of the known source
//               subdirectories.  This can only find files marked by a
//               previous call to get_dependable_file() with is_header
//               set to true.  Unlike
//               get_dependable_file_by_pathname() or
//               PPDirectory::get_dependable_file(), this does not
//               create an entry if it does not exist; instead, it
//               returns NULL if no matching file can be found.
////////////////////////////////////////////////////////////////////
PPDependableFile *PPDirectoryTree::
find_dependable_file(const string &filename) const {
  Dependables::const_iterator di;
  di = _dependables.find(filename);
  if (di != _dependables.end()) {
    return (*di).second;
  }

  return (PPDependableFile *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::get_dependable_file_by_pathname
//       Access: Public
//  Description: Given a dirname/filename for a particular dependable
//               filename, return (or create and return) the
//               corresponding PPDirectoryTree.  This is different
//               from find_dependable_file() in that an explicit
//               dirname is given, and the entry will be created if
//               it does not already exist.  However, if the directory
//               name does not exist, nothing is created, and NULL is
//               returned.
////////////////////////////////////////////////////////////////////
PPDependableFile *PPDirectoryTree::
get_dependable_file_by_dirpath(const string &dirpath, bool is_header) {
  size_t slash = dirpath.rfind('/');
  if (slash == string::npos) {
    // No valid directory name.
    return (PPDependableFile *)NULL;
  }

  string dirname = dirpath.substr(0, slash);
  string filename = dirpath.substr(slash + 1);

  PPDirectory *dir = find_dirname(dirname);
  if (dir == (PPDirectory *)NULL) {
    // No valid directory name.
    return (PPDependableFile *)NULL;
  }

  return dir->get_dependable_file(filename, is_header);
}

////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::read_file_dependencies
//       Access: Public
//  Description: Before processing the source files, makes a pass and
//               reads in all of the dependency cache files so we'll
//               have a heads-up on which files depend on the others.
////////////////////////////////////////////////////////////////////
void PPDirectoryTree::
read_file_dependencies(const string &cache_filename) {
  _root->read_file_dependencies(cache_filename);
}


////////////////////////////////////////////////////////////////////
//     Function: PPDirectoryTree::update_file_dependencies
//       Access: Public
//  Description: After all source processing has completed, makes one
//               more pass through the directory hierarchy and writes
//               out the inter-file dependency cache.
////////////////////////////////////////////////////////////////////
void PPDirectoryTree::
update_file_dependencies(const string &cache_filename) {
  _root->update_file_dependencies(cache_filename);
}

