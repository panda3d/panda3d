// Filename: eggPalettize.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGPALETTIZE_H
#define EGGPALETTIZE_H

#include <pandatoolbase.h>

#include "attribFile.h"
#include <eggMultiFilter.h>

#include <vector>

class PTexture;
class PNMFileType;

////////////////////////////////////////////////////////////////////
// 	 Class : EggPalettize
// Description : This is the heart of the program.
////////////////////////////////////////////////////////////////////
class EggPalettize : public EggMultiFilter {
public:
  EggPalettize();

  virtual bool handle_args(Args &args);
  virtual EggData *read_egg(const Filename &filename);

  void describe_input_file();

  string format_space(int size_pixels, bool verbose = false);
  void report_statistics();
  void run();

  typedef vector<AttribFile *> AttribFiles;
  AttribFiles _attrib_files;

  // The following parameter values specifically relate to textures
  // and palettes.  These values are stored in the .pi file for future
  // reference.
  string _map_dirname;
  bool _got_map_dirname;
  Filename _rel_dirname;
  bool _got_rel_dirname;
  string _default_groupname;
  bool _got_default_groupname;
  string _default_groupdir;
  bool _got_default_groupdir;
  int _pal_size[2];
  bool _got_palette_size;
  int _default_margin;
  bool _got_default_margin;
  bool _force_power_2;
  bool _got_force_power_2;
  bool _aggressively_clean_mapdir;
  bool _got_aggressively_clean_mapdir;
  string _image_type;
  bool _got_image_type;
  PNMFileType *_color_type;
  PNMFileType *_alpha_type;

  // The following values relate specifically to egg files.  They're
  // not saved for future sessions.
  double _fuzz_factor;
  bool _dont_override_repeats;

  // The following values control behavior specific to this session.
  // They're not saved either.
  bool _statistics_only;
  bool _force_optimal;
  bool _force_redo_all;
  bool _optimal_resize;
  bool _touch_eggs;
  bool _eggs_include_images;
  bool _dont_lock_pi;
  bool _remove_unused_lines;

  bool _describe_input_file;
};

#endif
