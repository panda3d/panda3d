// Filename: ppDirectoryTree.h
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPDIRECTORYTREE_H
#define PPDIRECTORYTREE_H

#include "ppremake.h"

#include <vector>
#include <map>
#include <set>

class PPCommandFile;
class PPScope;
class PPNamedScopes;

///////////////////////////////////////////////////////////////////
// 	 Class : PPDirectoryTree
// Description : Stores the directory hierarchy relationship of the
//               source tree.  Each PPDirectoryTree object is
//               one-to-one associated with a PPCommandFile object,
//               that corresponds to the source file found within this
//               directory.
////////////////////////////////////////////////////////////////////
class PPDirectoryTree {
public:
  PPDirectoryTree();
  ~PPDirectoryTree();

protected:
  PPDirectoryTree(const string &dirname, PPDirectoryTree *parent);

public:
  bool scan_source(PPNamedScopes *named_scopes);
  bool scan_depends(PPNamedScopes *named_scopes);

  int count_source_files() const;

  const string &get_dirname() const;
  int get_depends_index() const;
  string get_path() const;
  string get_rel_to(const PPDirectoryTree *other) const;

  PPDirectoryTree *find_dirname(const string &dirname);
  PPCommandFile *get_source() const;

  int get_num_children() const;
  PPDirectoryTree *get_child(int n) const;

  string get_child_dirnames() const;
  string get_complete_subtree() const;

private:
  bool r_scan(const string &prefix);
  bool read_source_file(const string &prefix, PPNamedScopes *named_scopes);
  bool read_depends_file(PPNamedScopes *named_scopes);
  bool resolve_dependencies();
  bool compute_depends_index();

  string _dirname;
  PPScope *_scope;
  PPCommandFile *_source;
  PPDirectoryTree *_parent;
  typedef vector<PPDirectoryTree *> Children;
  Children _children;
  int _depth;

  typedef set<PPDirectoryTree *> Depends;
  Depends _i_depend_on;
  Depends _depends_on_me;
  int _depends_index;
  bool _computing_depends_index;


  typedef map<string, PPDirectoryTree *> Dirnames;
  Dirnames *_dirnames;
};

extern PPDirectoryTree *current_output_directory;

#endif

