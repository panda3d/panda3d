// Filename: cvsCopy.h
// Created by:  drose (31Oct00)
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

#ifndef CVSCOPY_H
#define CVSCOPY_H

#include "pandatoolbase.h"

#include "cvsSourceTree.h"

#include "programBase.h"
#include "filename.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : CVSCopy
// Description : This is the base class for a family of programs that
//               copy files, typically model files like .flt files and
//               their associated textures, into a CVS-controlled
//               source tree.
////////////////////////////////////////////////////////////////////
class CVSCopy : public ProgramBase {
public:
  CVSCopy();

  CVSSourceTree::FilePath
  import(const Filename &source, void *extra_data,
         CVSSourceDirectory *suggested_dir);

  bool continue_after_error();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  virtual bool verify_file(const Filename &source, const Filename &dest,
                           CVSSourceDirectory *dest_dir,
                           void *extra_data);
  virtual bool copy_file(const Filename &source, const Filename &dest,
                         CVSSourceDirectory *dest_dir,
                         void *extra_data, bool new_file)=0;

  bool verify_binary_file(Filename source, Filename dest);
  bool copy_binary_file(Filename source, Filename dest);

  bool cvs_add(const Filename &filename);
  static string protect_from_shell(const string &source);

  virtual string filter_filename(const string &source);

private:
  bool scan_hierarchy();
  bool scan_for_root(const string &dirname);
  string prompt(const string &message);

protected:
  bool _force;
  bool _interactive;
  bool _got_model_dirname;
  Filename _model_dirname;
  bool _got_map_dirname;
  Filename _map_dirname;
  bool _got_root_dirname;
  Filename _root_dirname;
  Filename _key_filename;
  bool _no_cvs;
  string _cvs_binary;
  bool _user_aborted;

  typedef pvector<Filename> SourceFiles;
  SourceFiles _source_files;

  CVSSourceTree _tree;
  CVSSourceDirectory *_model_dir;
  CVSSourceDirectory *_map_dir;

  typedef pmap<string, CVSSourceTree::FilePath> CopiedFiles;
  CopiedFiles _copied_files;
};

#endif
