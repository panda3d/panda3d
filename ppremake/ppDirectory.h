// Filename: ppDirectory.h
// Created by:  drose (28Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PPDIRECTORY_H
#define PPDIRECTORY_H

#include "ppremake.h"

#include <vector>
#include <map>
#include <set>

class PPCommandFile;
class PPScope;
class PPNamedScopes;
class PPDirectoryTree;
class PPDependableFile;

///////////////////////////////////////////////////////////////////
//       Class : PPDirectory
// Description : Represents a single directory in the source
//               hierarchy.  Each PPDirectory object is one-to-one
//               associated with a PPCommandFile object, that
//               corresponds to the source file found within this
//               directory.
////////////////////////////////////////////////////////////////////
class PPDirectory {
public:
  PPDirectory(PPDirectoryTree *tree);
  PPDirectory(const string &dirname, PPDirectory *parent);
  ~PPDirectory();

  PPDirectoryTree *get_tree() const;
  int count_source_files() const;

  const string &get_dirname() const;
  int get_depends_index() const;
  string get_path() const;
  string get_fullpath() const;
  string get_rel_to(const PPDirectory *other) const;

  PPCommandFile *get_source() const;

  int get_num_children() const;
  PPDirectory *get_child(int n) const;

  string get_child_dirnames() const;
  string get_complete_subtree() const;

  PPDependableFile *get_dependable_file(const string &filename, 
                                        bool is_header);

  void report_depends() const;
  void report_reverse_depends() const;

private:
  typedef set<PPDirectory *> Depends;

  bool r_scan(const string &prefix);
  bool scan_extra_depends(const string &cache_filename);
  bool read_source_file(const string &prefix, PPNamedScopes *named_scopes);
  bool read_depends_file(PPNamedScopes *named_scopes);
  bool resolve_dependencies();
  bool compute_depends_index();
  void read_file_dependencies(const string &cache_filename);
  void update_file_dependencies(const string &cache_filename);

  void get_complete_i_depend_on(Depends &dep) const;
  void get_complete_depends_on_me(Depends &dep) const;
  void show_directories(const Depends &dep) const;

  string _dirname;
  PPScope *_scope;
  PPCommandFile *_source;
  PPDirectory *_parent;
  PPDirectoryTree *_tree;
  typedef vector<PPDirectory *> Children;
  Children _children;
  int _depth;

  Depends _i_depend_on;
  Depends _depends_on_me;
  int _depends_index;
  bool _computing_depends_index;

  typedef map<string, PPDependableFile *> Dependables;
  Dependables _dependables;

  friend class PPDirectoryTree;
};

extern PPDirectory *current_output_directory;

#endif

