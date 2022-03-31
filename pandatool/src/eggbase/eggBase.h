/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBase.h
 * @author drose
 * @date 2000-02-14
 */

#ifndef EGGBASE_H
#define EGGBASE_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "eggData.h"

/**
 * This is a base class for both EggSingleBase and EggMultiBase.  Don't
 * inherit directly from this; use one of those two classes instead.
 *
 * This is just a base class; see EggReader, EggWriter, or EggFilter according
 * to your particular I/O needs.
 */
class EggBase : public ProgramBase {
public:
  EggBase();

  void add_normals_options();
  void add_points_options();
  void add_transform_options();

  static void convert_paths(EggNode *node, PathReplace *path_replace,
                            const DSearchPath &additional_path);

protected:
  void append_command_comment(EggData *_data);
  static void append_command_comment(EggData *_data, const std::string &comment);

  static bool dispatch_normals(ProgramBase *self, const std::string &opt, const std::string &arg, void *mode);
  bool ns_dispatch_normals(const std::string &opt, const std::string &arg, void *mode);

  static bool dispatch_scale(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_rotate_xyz(ProgramBase *self, const std::string &opt, const std::string &arg, void *var);
  bool ns_dispatch_rotate_xyz(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_rotate_axis(ProgramBase *self, const std::string &opt, const std::string &arg, void *var);
  bool ns_dispatch_rotate_axis(const std::string &opt, const std::string &arg, void *var);
  static bool dispatch_translate(const std::string &opt, const std::string &arg, void *var);

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
  bool _got_tbnauto;

  bool _make_points;

  bool _got_transform;
  LMatrix4d _transform;

  bool _got_coordinate_system;
  CoordinateSystem _coordinate_system;

  bool _noabs;
};

#endif
