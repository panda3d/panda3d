/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file somethingToEgg.h
 * @author drose
 * @date 2000-02-15
 */

#ifndef SOMETHINGTOEGG_H
#define SOMETHINGTOEGG_H

#include "pandatoolbase.h"

#include "eggConverter.h"
#include "distanceUnit.h"
#include "animationConvert.h"

class SomethingToEggConverter;

/**
 * This is the general base class for a file-converter program that reads some
 * model file format and generates an egg file.
 */
class SomethingToEgg : public EggConverter {
public:
  SomethingToEgg(const std::string &format_name,
                 const std::string &preferred_extension = std::string(),
                 bool allow_last_param = true,
                 bool allow_stdout = true);

  void add_units_options();
  void add_animation_options();
  void add_merge_externals_options();

protected:
  void apply_units_scale(EggData *data);
  void apply_parameters(SomethingToEggConverter &converter);

  virtual bool handle_args(Args &args);
  virtual bool post_command_line();
  virtual void post_process_egg_file();

  static bool dispatch_animation_convert(const std::string &opt, const std::string &arg, void *var);


  Filename _input_filename;

  DistanceUnit _input_units;
  DistanceUnit _output_units;

  AnimationConvert _animation_convert;
  std::string _character_name;
  double _start_frame;
  double _end_frame;
  double _frame_inc;
  double _neutral_frame;
  double _input_frame_rate;
  double _output_frame_rate;
  bool _got_start_frame;
  bool _got_end_frame;
  bool _got_frame_inc;
  bool _got_neutral_frame;
  bool _got_input_frame_rate;
  bool _got_output_frame_rate;

  bool _merge_externals;
  bool _noexist;
  bool _allow_errors;
};

#endif
