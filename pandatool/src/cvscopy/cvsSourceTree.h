// Filename: cvsSourceTree.h
// Created by:  drose (31Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CVSSOURCETREE_H
#define CVSSOURCETREE_H

#include <pandatoolbase.h>

#include <vector>
#include <map>

class CVSSourceDirectory;

////////////////////////////////////////////////////////////////////
//       Class : CVSSourceTree
// Description : This represents the root of the tree of source
//               directory files.
////////////////////////////////////////////////////////////////////
class CVSSourceTree {
public:
  CVSSourceTree();
  ~CVSSourceTree();

  void set_root(const string &root_path);
  bool scan(const string &key_filename);

  CVSSourceDirectory *get_root() const;
  CVSSourceDirectory *find_directory(const string &path);
  CVSSourceDirectory *find_relpath(const string &relpath);
  CVSSourceDirectory *find_dirname(const string &dirname);

  CVSSourceDirectory *choose_directory(const string &filename,
                                       CVSSourceDirectory *suggested_dir,
                                       bool force, bool interactive);

  string get_root_fullpath();
  string get_root_dirname() const;

  static bool temp_chdir(const string &path);
  static void restore_cwd();

public:
  void add_file(const string &filename, CVSSourceDirectory *dir);

private:
  typedef vector<CVSSourceDirectory *> Directories;

  CVSSourceDirectory *
  prompt_user(const string &filename, CVSSourceDirectory *suggested_dir,
              const Directories &dirs, bool force, bool interactive);

  CVSSourceDirectory *ask_existing(const string &filename, 
                                   CVSSourceDirectory *dir);
  CVSSourceDirectory *ask_existing(const string &filename,
                                   const Directories &dirs,
                                   CVSSourceDirectory *suggested_dir);
  CVSSourceDirectory *ask_new(const string &filename, CVSSourceDirectory *dir);
  CVSSourceDirectory *ask_any(const string &filename);

  string prompt(const string &message);

  static string get_actual_fullpath(const string &path);
  static string get_start_fullpath();

private:
  string _path;
  CVSSourceDirectory *_root;

  typedef map<string, Directories> Filenames;
  Filenames _filenames;

  static bool _got_start_fullpath;
  static string _start_fullpath;
  bool _got_root_fullpath;
  string _root_fullpath;
};

#endif
