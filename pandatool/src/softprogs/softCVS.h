// Filename: softCVS.h
// Created by:  drose (10Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
//       Class : SoftCVS
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
