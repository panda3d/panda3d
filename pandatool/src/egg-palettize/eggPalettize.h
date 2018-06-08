/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPalettize.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef EGGPALETTIZE_H
#define EGGPALETTIZE_H

#include "pandatoolbase.h"

#include "eggMultiFilter.h"

/**
 * This is the program wrapper for egg-palettize, but it mainly serves to read
 * in all the command-line parameters and then invoke the Palettizer.
 */
class EggPalettize : public EggMultiFilter {
public:
  EggPalettize();

  virtual bool handle_args(Args &args);

  void describe_input_file();

  void run();

  // The following parameter values specifically relate to textures and
  // palettes.  These values are copied to the Palettizer.
  bool _got_txa_filename;
  Filename _txa_filename;
  bool _got_txa_script;
  std::string _txa_script;
  bool _nodb;
  std::string _generated_image_pattern;
  bool _got_generated_image_pattern;
  std::string _map_dirname;
  bool _got_map_dirname;
  Filename _shadow_dirname;
  bool _got_shadow_dirname;
  Filename _rel_dirname;
  bool _got_rel_dirname;
  std::string _default_groupname;
  bool _got_default_groupname;
  std::string _default_groupdir;
  bool _got_default_groupdir;

private:
  // The following values control behavior specific to this session.  They're
  // not saved for future sessions.
  bool _report_pi;
  bool _report_statistics;
  bool _all_textures;
  bool _optimal;
  bool _omitall;
  bool _redo_all;
  bool _redo_eggs;

  bool _describe_input_file;
  bool _remove_eggs;
  Args _remove_egg_list;
};

#endif
