// Filename: eggPalettize.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGPALETTIZE_H
#define EGGPALETTIZE_H

#include <pandatoolbase.h>

#include "txaFile.h"

#include <eggMultiFilter.h>

#include <vector>

class PNMFileType;
class EggFile;
class PaletteGroup;
class TextureImage;

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
  int _pal_size[2];
  bool _got_palette_size;
  int _margin;
  bool _got_margin;
  double _repeat_threshold;
  bool _got_repeat_threshold;
  bool _force_power_2;
  bool _off_force_power_2;
  bool _aggressively_clean_mapdir;
  bool _off_aggressively_clean_mapdir;
  string _image_type;
  bool _got_image_type;
  PNMFileType *_color_type;
  PNMFileType *_alpha_type;

private:
  // The following values control behavior specific to this session.
  // They're not saved for future sessions.
  bool _report_pi;
  bool _statistics_only;
  bool _force_optimal;
  bool _force_redo_all;
  bool _optimal_resize;
  bool _touch_eggs;
  bool _dont_lock_pi;

  bool _describe_input_file;
};

#endif
