// Filename: eggPalettize.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGPALETTIZE_H
#define EGGPALETTIZE_H

#include <pandatoolbase.h>

#include <eggMultiFilter.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggPalettize
// Description : This is the program wrapper for egg-palettize, but it
//               mainly serves to read in all the command-line
//               parameters and then invoke the Palettizer.
////////////////////////////////////////////////////////////////////
class EggPalettize : public EggMultiFilter {
public:
  EggPalettize();

  virtual bool handle_args(Args &args);

  void describe_input_file();

  void run();

  // The following parameter values specifically relate to textures
  // and palettes.  These values are copied to the Palettizer.
  Filename _txa_filename;
  string _map_dirname;
  bool _got_map_dirname;
  Filename _rel_dirname;
  bool _got_rel_dirname;
  string _default_groupname;
  bool _got_default_groupname;
  string _default_groupdir;
  bool _got_default_groupdir;

private:
  // The following values control behavior specific to this session.
  // They're not saved for future sessions.
  bool _report_pi;
  bool _statistics_only;
  bool _all_textures; 
  bool _force_optimal;
  bool _redo_all;
  bool _redo_eggs;
  bool _dont_lock_pi;

  bool _describe_input_file;
};

#endif
