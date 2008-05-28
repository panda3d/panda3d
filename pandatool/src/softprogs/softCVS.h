// Filename: softCVS.h
// Created by:  drose (10Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef SOFTCVS_H
#define SOFTCVS_H

#include "pandatoolbase.h"

#include "softFilename.h"

#include "programBase.h"
#include "vector_string.h"
#include "filename.h"

#include "pvector.h"
#include "pset.h"

class Multifile;

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
  typedef pvector<SoftFilename> SceneFiles;
  typedef pmultiset<SoftFilename> ElementFiles;

  void traverse_root();
  void traverse_subdir(const Filename &directory);

  void collapse_scene_files();
  bool get_scenes();
  void remove_unused_elements();

  bool rename_file(SceneFiles::iterator begin, SceneFiles::iterator end);
  bool scan_cvs(const string &dirname, pset<string> &cvs_elements);
  bool scan_scene_file(istream &in, Multifile &multifile);

  bool cvs_add(const string &path);
  bool cvs_add_or_remove(const string &cvs_command,
                         const vector_string &paths);

  SceneFiles _scene_files;
  ElementFiles _element_files;
  vector_string _global_files;

  vector_string _cvs_add;
  vector_string _cvs_remove;

  bool _no_cvs;
  string _cvs_binary;
};

#endif
