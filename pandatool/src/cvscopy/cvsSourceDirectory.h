// Filename: cvsSourceDirectory.h
// Created by:  drose (31Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CVSSOURCEDIRECTORY_H
#define CVSSOURCEDIRECTORY_H

#include <pandatoolbase.h>
#include <filename.h>

#include <vector>

class CVSSourceTree;

////////////////////////////////////////////////////////////////////
// 	 Class : CVSSourceDirectory
// Description : This represents one particular directory in the
//               hierarchy of source directory files.  We must scan
//               the source directory to identify where the related
//               files have previously been copied.
////////////////////////////////////////////////////////////////////
class CVSSourceDirectory {
public:
  CVSSourceDirectory(CVSSourceTree *tree, CVSSourceDirectory *parent,
		     const string &dirname);
  ~CVSSourceDirectory();

  string get_dirname() const;
  string get_fullpath() const;
  string get_path() const;
  string get_rel_to(const CVSSourceDirectory *other) const;

  int get_num_children() const;
  CVSSourceDirectory *get_child(int n) const;

  CVSSourceDirectory *find_relpath(const string &relpath);
  CVSSourceDirectory *find_dirname(const string &dirname);

public:
  bool scan(const Filename &directory, const string &key_filename);

private:
  CVSSourceTree *_tree;
  CVSSourceDirectory *_parent;
  string _dirname;
  int _depth;

  typedef vector<CVSSourceDirectory *> Children;
  Children _children;
};

#endif
