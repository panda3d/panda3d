// Filename: eggWriter.h
// Created by:  drose (14Feb00)
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

#ifndef EGGWRITER_H
#define EGGWRITER_H

#include "pandatoolbase.h"
#include "eggBase.h"
#include "withOutputFile.h"

#include "filename.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggWriter
// Description : This is the base class for a program that generates
//               an egg file output, but doesn't read any for input.
////////////////////////////////////////////////////////////////////
class EggWriter : virtual public EggBase, public WithOutputFile {
public:
  EggWriter(bool allow_last_param = false, bool allow_stdout = true);

  void add_normals_options();
  void add_transform_options();

  virtual EggWriter *as_writer();

  virtual void post_process_egg_file();
  void write_egg_file();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  static bool dispatch_normals(ProgramBase *self, const string &opt, const string &arg, void *mode);
  bool ns_dispatch_normals(const string &opt, const string &arg, void *mode);

  static bool dispatch_scale(const string &opt, const string &arg, void *var);
  static bool dispatch_rotate_xyz(ProgramBase *self, const string &opt, const string &arg, void *var);
  bool ns_dispatch_rotate_xyz(const string &opt, const string &arg, void *var);
  static bool dispatch_rotate_axis(ProgramBase *self, const string &opt, const string &arg, void *var);
  bool ns_dispatch_rotate_axis(const string &opt, const string &arg, void *var);
  static bool dispatch_translate(const string &opt, const string &arg, void *var);

protected:
  enum NormalsMode {
    NM_strip,
    NM_polygon,
    NM_vertex,
    NM_preserve
  };
  NormalsMode _normals_mode;
  double _normals_threshold;

  bool _got_transform;
  LMatrix4d _transform;

private:
  ofstream _output_stream;
  ostream *_output_ptr;
};

#endif


