/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmTrans.h
 * @author drose
 * @date 2010-12-23
 */

#ifndef PFMTRANS_H
#define PFMTRANS_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "filename.h"
#include "pvector.h"
#include "nodePath.h"
#include "luse.h"

class PfmFile;

/**
 * Operates on a pfm file.
 */
class PfmTrans : public ProgramBase {
public:
  PfmTrans();

  void run();
  bool process_pfm(const Filename &input_filename, PfmFile &file);

  void add_transform_options();

protected:
  virtual bool handle_args(Args &args);

  static bool dispatch_scale(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_rotate_xyz(ProgramBase *self, const std::string &opt, const std::string &arg, void *var);
  bool ns_dispatch_rotate_xyz(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_rotate_axis(ProgramBase *self, const std::string &opt, const std::string &arg, void *var);
  bool ns_dispatch_rotate_axis(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_translate(const std::string &opt, const std::string &arg, void *var);

private:
  typedef pvector<Filename> Filenames;
  Filenames _input_filenames;

  bool _got_zero_special;
  bool _got_no_data_nan;
  int _no_data_nan_num_channels;
  bool _got_vis_inverse;
  bool _got_vis_2d;
  bool _got_resize;
  int _resize[2];
  bool _got_crop;
  int _crop[4];
  bool _got_autocrop;
  int _rotate;
  bool _got_mirror_x;
  bool _got_mirror_y;

  bool _got_output_filename;
  Filename _output_filename;
  bool _got_output_dirname;
  Filename _output_dirname;
  bool _got_vis_filename;
  Filename _vis_filename;
  bool _got_vistex_filename;
  Filename _vistex_filename;
  bool _got_ls_filename;
  Filename _ls_filename;

  bool _got_transform;
  LMatrix4 _transform;

  NodePath _mesh_root;
};

#endif
