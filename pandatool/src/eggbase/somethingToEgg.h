// Filename: somethingToEgg.h
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOMETHINGTOEGG_H
#define SOMETHINGTOEGG_H

#include <pandatoolbase.h>

#include "eggConverter.h"

#include <distanceUnit.h>
#include <somethingToEggConverter.h>


////////////////////////////////////////////////////////////////////
//       Class : SomethingToEgg
// Description : This is the general base class for a file-converter
//               program that reads some model file format and
//               generates an egg file.
////////////////////////////////////////////////////////////////////
class SomethingToEgg : public EggConverter {
public:
  SomethingToEgg(const string &format_name, 
                 const string &preferred_extension = string(),
                 bool allow_last_param = true,
                 bool allow_stdout = true);

  void add_units_options();
  void add_texture_path_options();
  void add_model_path_options();
  void add_rel_dir_options();
  void add_search_path_options(bool append_to_sys_paths);
  void add_merge_externals_options();

protected:
  void apply_units_scale(EggData &data);

  virtual bool handle_args(Args &args);
  virtual bool post_command_line();
  virtual void post_process_egg_file();

  static bool dispatch_path_convert_relative(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_absolute(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_rel_abs(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_strip(const string &opt, const string &arg, void *var);
  static bool dispatch_path_convert_unchanged(const string &opt, const string &arg, void *var);


  Filename _input_filename;

  DistanceUnit _input_units;
  DistanceUnit _output_units;

  SomethingToEggConverter::PathConvert _texture_path_convert;
  SomethingToEggConverter::PathConvert _model_path_convert;

  Filename _make_rel_dir;
  bool _got_make_rel_dir;

  DSearchPath _search_path;
  bool _got_search_path;
  bool _append_to_sys_paths;

  bool _merge_externals;
};

#endif


