// Filename: eggBase.h
// Created by:  drose (14Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGBASE_H
#define EGGBASE_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "eggData.h"

////////////////////////////////////////////////////////////////////
//       Class : EggBase
// Description : This is a base class for both EggSingleBase and
//               EggMultiBase.  Don't inherit directly from this; use
//               one of those two classes instead.
//
//               This is just a base class; see EggReader, EggWriter,
//               or EggFilter according to your particular I/O needs.
////////////////////////////////////////////////////////////////////
class EggBase : public ProgramBase {
public:
  EggBase();

  void add_normals_options();
  void add_transform_options();

  static void convert_paths(EggNode *node, PathReplace *path_replace,
                            const DSearchPath &additional_path);

protected:
  void append_command_comment(EggData *_data);
  static void append_command_comment(EggData *_data, const string &comment);

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
  vector_string _tbn_names;
  bool _got_tbnall;

  bool _got_transform;
  LMatrix4d _transform;

  bool _got_coordinate_system;
  CoordinateSystem _coordinate_system;

  bool _noabs;
};

#endif


