// Filename: softCVS.h
// Created by:  drose (10Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOFTCVS_H
#define SOFTCVS_H

#include <pandatoolbase.h>

#include "softFilename.h"

#include <programBase.h>
#include <vector_string.h>
#include <filename.h>

#include <vector>
#include <set>

////////////////////////////////////////////////////////////////////
// 	 Class : SoftCVS
// Description : This program prepares a SoftImage database for CVS by
//               renaming everything to version 1-0, and adding new
//               files to CVS.
////////////////////////////////////////////////////////////////////
class SoftCVS : public ProgramBase {
public:
  SoftCVS();

  void run();

private:
  typedef vector<SoftFilename> SceneFiles;
  typedef multiset<SoftFilename> ElementFiles;

  void traverse_root();
  void traverse_subdir(const Filename &directory);

  void collapse_scene_files();
  void count_references();
  void remove_unused_elements();

  bool rename_file(SceneFiles::iterator begin, SceneFiles::iterator end);
  bool scan_cvs(const string &dirname, set<string> &cvs_elements);

  void scan_scene_file(istream &in);

  bool cvs_add(const string &path);
  bool cvs_add_or_remove(const string &cvs_command, 
			 const vector_string &paths);

  SceneFiles _scene_files;
  ElementFiles _element_files;

  vector_string _cvs_add;
  vector_string _cvs_remove;
  
protected:
  bool _no_cvs;
  string _cvs_binary;
};

#endif
