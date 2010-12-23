// Filename: pfmTrans.h
// Created by:  drose (23Dec10)
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

#ifndef PFMTRANS_H
#define PFMTRANS_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "filename.h"
#include "pvector.h"
#include "nodePath.h"
#include "luse.h"

class PfmFile;

////////////////////////////////////////////////////////////////////
//       Class : PfmTrans
// Description : Operates on a pfm file.
////////////////////////////////////////////////////////////////////
class PfmTrans : public ProgramBase {
public:
  PfmTrans();

  void run();
  bool process_pfm(const Filename &input_filename, PfmFile &file);

  void add_transform_options();

protected:
  virtual bool handle_args(Args &args);

  static bool dispatch_scale(const string &opt, const string &arg, void *var);
  static bool dispatch_rotate_xyz(ProgramBase *self, const string &opt, const string &arg, void *var);
  bool ns_dispatch_rotate_xyz(const string &opt, const string &arg, void *var);
  static bool dispatch_rotate_axis(ProgramBase *self, const string &opt, const string &arg, void *var);
  bool ns_dispatch_rotate_axis(const string &opt, const string &arg, void *var);
  static bool dispatch_translate(const string &opt, const string &arg, void *var);

private:
  typedef pvector<Filename> Filenames;
  Filenames _input_filenames;

  bool _got_reverse;
  bool _got_zero_special;
  bool _got_resize;
  int _resize[2];
  bool _got_bba;

  bool _got_output_filename;
  Filename _output_filename;
  bool _got_output_dirname;
  Filename _output_dirname;
  bool _got_vis_filename;
  Filename _vis_filename;
  bool _got_vistex_filename;
  Filename _vistex_filename;

  bool _got_transform;
  LMatrix4f _transform;

  NodePath _mesh_root;
};

#endif
