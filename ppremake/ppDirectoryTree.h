// Filename: ppDirectoryTree.h
// Created by:  drose (15Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPDIRECTORYTREE_H
#define PPDIRECTORYTREE_H

#include "ppremake.h"

#include <map>

class PPNamedScopes;
class PPDirectory;
class PPDependableFile;

///////////////////////////////////////////////////////////////////
//       Class : PPDirectoryTree
// Description : Stores the entire directory hierarchy relationship of the
//               source tree.  This is the root of a tree of
//               PPDirectory objects, each of which corresponds to a
//               particular directory.
////////////////////////////////////////////////////////////////////
class PPDirectoryTree {
public:
  PPDirectoryTree();
  ~PPDirectoryTree();

  bool scan_source(PPNamedScopes *named_scopes);
  bool scan_depends(PPNamedScopes *named_scopes);

  int count_source_files() const;
  PPDirectory *get_root() const;

  string get_complete_tree() const;

  PPDirectory *find_dirname(const string &dirname) const;

  PPDependableFile *find_dependable_file(const string &filename) const;
  PPDependableFile *get_dependable_file_by_dirpath(const string &dirpath,
                                                   bool is_header);

  void read_file_dependencies(const string &cache_filename);
  void update_file_dependencies(const string &cache_filename);

private:
  PPDirectory *_root;

  typedef map<string, PPDirectory *> Dirnames;
  Dirnames _dirnames;

  typedef map<string, PPDependableFile *> Dependables;
  Dependables _dependables;

  friend class PPDirectory;
};

#endif
