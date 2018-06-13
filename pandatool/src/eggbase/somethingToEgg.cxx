/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file somethingToEgg.cxx
 * @author drose
 * @date 2000-02-15
 */

#include "somethingToEgg.h"
#include "somethingToEggConverter.h"

#include "config_putil.h"

/**
 * The first parameter to the constructor should be the one-word name of the
 * file format that is to be read, for instance "OpenFlight" or "Alias".  It's
 * just used in printing error messages and such.
 */
SomethingToEgg::
SomethingToEgg(const std::string &format_name,
               const std::string &preferred_extension,
               bool allow_last_param, bool allow_stdout) :
  EggConverter(format_name, preferred_extension, allow_last_param, allow_stdout)
{
  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] input" + _preferred_extension + " output.egg");
  }
  add_runline("[opts] -o output.egg input" + _preferred_extension);
  if (_allow_stdout) {
    add_runline("[opts] input" + _preferred_extension + " >output.egg");
  }

  // -f doesn't make sense if we aren't reading egg files.
  remove_option("f");

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this can inferred from the file itself.");

  add_option
    ("noabs", "", 0,
     "Don't allow the input " + _format_name + " file to have absolute pathnames.  "
     "If it does, abort with an error.  This option is designed to help "
     "detect errors when populating or building a standalone model tree, "
     "which should be self-contained and include only relative pathnames.",
     &SomethingToEgg::dispatch_none, &_noabs);

  add_option
    ("noexist", "", 0,
     "Don't treat it as an error if the input file references pathnames "
     "(e.g. textures) that don't exist.  Normally, this will be flagged as "
     "an error and the command aborted; with this option, an egg file will "
     "be generated anyway, referencing pathnames that do not exist.",
     &SomethingToEgg::dispatch_none, &_noexist);

  add_option
    ("ignore", "", 0,
     "Ignore non-fatal errors and generate an egg file anyway.",
     &SomethingToEgg::dispatch_none, &_allow_errors);

  _input_units = DU_invalid;
  _output_units = DU_invalid;
  _animation_convert = AC_none;
  _got_start_frame = false;
  _got_end_frame = false;
  _got_frame_inc = false;
  _got_neutral_frame = false;
  _got_input_frame_rate = false;
  _got_output_frame_rate = false;
  _merge_externals = false;
}

/**
 * Adds -ui and -uo as valid options for this program.  If the user specifies
 * -uo and -ui, or just -uo and the program specifies -ui by setting
 * _input_units, the indicated units conversion will be automatically applied
 * before writing out the egg file.
 */
void SomethingToEgg::
add_units_options() {
  add_option
    ("ui", "units", 40,
     "Specify the units of the input " + _format_name +
     " file.  Normally, this can be inferred from the file itself.",
     &SomethingToEgg::dispatch_units, nullptr, &_input_units);

  add_option
    ("uo", "units", 40,
     "Specify the units of the resulting egg file.  If this is "
     "specified, the vertices in the egg file will be scaled as "
     "necessary to make the appropriate units conversion; otherwise, "
     "the vertices will be left as they are.",
     &SomethingToEgg::dispatch_units, nullptr, &_output_units);
}

/**
 * Adds options appropriate to animation packages.
 */
void SomethingToEgg::
add_animation_options() {
  add_option
    ("a", "animation-mode", 40,
     "Specifies how animation from the " + _format_name + " file is "
     "converted to egg, if at all.  At present, the following keywords "
     "are supported: none, pose, flip, strobe, model, chan, or both.  "
     "The default is none, which means not to convert animation.",
     &SomethingToEgg::dispatch_animation_convert, nullptr, &_animation_convert);

  add_option
    ("cn", "name", 40,
     "Specifies the name of the animation character.  This should match "
     "between all of the model files and all of the channel files for a "
     "particular model and its associated channels.",
     &SomethingToEgg::dispatch_string, nullptr, &_character_name);

  add_option
    ("sf", "start-frame", 40,
     "Specifies the starting frame of animation to extract.  If omitted, "
     "the first frame of the time slider will be used.  For -a pose, this "
     "is the one frame of animation to extract.",
     &SomethingToEgg::dispatch_double, &_got_start_frame, &_start_frame);

  add_option
    ("ef", "end-frame", 40,
     "Specifies the ending frame of animation to extract.  If omitted, "
     "the last frame of the time slider will be used.",
     &SomethingToEgg::dispatch_double, &_got_end_frame, &_end_frame);

  add_option
    ("if", "frame-inc", 40,
     "Specifies the increment between successive frames.  If omitted, "
     "this is taken from the time slider settings, or 1.0 if the time "
     "slider does not specify.",
     &SomethingToEgg::dispatch_double, &_got_frame_inc, &_frame_inc);

  add_option
    ("nf", "neutral-frame", 40,
     "Specifies the frame number to use for the neutral pose.  The model "
     "will be set to this frame before extracting out the neutral character.  "
     "If omitted, the current frame of the model is used.  This is only "
     "relevant for -a model or -a both.",
     &SomethingToEgg::dispatch_double, &_got_neutral_frame, &_neutral_frame);

  add_option
    ("fri", "fps", 40,
     "Specify the frame rate (frames per second) of the input " + _format_name +
     " file.  Normally, this can be inferred from the file itself.",
     &SomethingToEgg::dispatch_double, &_got_input_frame_rate, &_input_frame_rate);

  add_option
    ("fro", "fps", 40,
     "Specify the frame rate (frames per second) of the generated animation.  "
     "If this is specified, the animation speed is scaled by the appropriate "
     "factor based on the frame rate of the input file (see -fri).",
     &SomethingToEgg::dispatch_double, &_got_output_frame_rate, &_output_frame_rate);
}

/**
 * Adds -f.
 */
void SomethingToEgg::
add_merge_externals_options() {
  add_option
    ("f", "", 40,
     "Follow and convert all external references in the source file.",
     &SomethingToEgg::dispatch_none, &_merge_externals);
}

/**
 * Applies the scale indicated by the input and output units to the indicated
 * egg file.  This is normally done automatically when the file is written
 * out.
 */
void SomethingToEgg::
apply_units_scale(EggData *data) {
  if (_output_units != DU_invalid && _input_units != DU_invalid &&
      _input_units != _output_units) {
    nout << "Converting from " << format_long_unit(_input_units)
         << " to " << format_long_unit(_output_units) << "\n";
    double scale = convert_units(_input_units, _output_units);
    data->transform(LMatrix4d::scale_mat(scale));
  }
}

/**
 * Copies the relevant parameters specified by the user on the command line
 * (if add_path_replace_options(), add_path_store_options(), or
 * add_animation_options() was used) to the converter.
 */
void SomethingToEgg::
apply_parameters(SomethingToEggConverter &converter) {
  _path_replace->_noabs = _noabs;
  _path_replace->_exists = !_noexist;
  converter.set_path_replace(_path_replace);

  converter.set_animation_convert(_animation_convert);
  converter.set_character_name(_character_name);
  if (_got_start_frame) {
    converter.set_start_frame(_start_frame);
  }
  if (_got_end_frame) {
    converter.set_end_frame(_end_frame);
  }
  if (_got_frame_inc) {
    converter.set_frame_inc(_frame_inc);
  }
  if (_got_neutral_frame) {
    converter.set_neutral_frame(_neutral_frame);
  }
  if (_got_input_frame_rate) {
    converter.set_input_frame_rate(_input_frame_rate);
  }
  if (_got_output_frame_rate) {
    converter.set_output_frame_rate(_output_frame_rate);
  }
}

/**
 *
 */
bool SomethingToEgg::
handle_args(Args &args) {
  if (_allow_last_param && !_got_output_filename && args.size() > 1) {
    _got_output_filename = true;
    _output_filename = Filename::from_os_specific(args.back());
    args.pop_back();

    if (!(_output_filename.get_extension() == "egg")) {
      nout << "Output filename " << _output_filename
           << " does not end in .egg.  If this is really what you intended, "
        "use the -o output_file syntax.\n";
      return false;
    }

    if (!verify_output_file_safe()) {
      return false;
    }
  }

  if (args.empty()) {
    nout << "You must specify the " << _format_name
          << " file to read on the command line.\n";
    return false;
  }

  if (args.size() != 1) {
    nout << "You may only specify one " << _format_name
         << " file to read on the command line.  "
         << "You specified: ";
    Args::const_iterator ai;
    for (ai = args.begin(); ai != args.end(); ++ai) {
      nout << (*ai) << " ";
    }
    nout << "\n";
    return false;
  }

  _input_filename = Filename::from_os_specific(args[0]);

  if (!_input_filename.exists()) {
    nout << "Cannot find input file " << _input_filename << "\n";
    return false;
  }

  if (!_got_path_directory && _got_output_filename) {
    // Put in the name of the output directory.
    _path_replace->_path_directory = _output_filename.get_dirname();
  }

  return true;
}

/**
 * This is called after the command line has been completely processed, and it
 * gives the program a chance to do some last-minute processing and validation
 * of the options and arguments.  It should return true if everything is fine,
 * false if there is an error.
 */
bool SomethingToEgg::
post_command_line() {
  // Prepend the source filename to the model path.
  ConfigVariableSearchPath &model_path = get_model_path();
  Filename directory = _input_filename.get_dirname();
  if (directory.empty()) {
    directory = ".";
  }
  model_path.prepend_directory(directory);

  return EggConverter::post_command_line();
}

/**
 * Performs any processing of the egg file that is appropriate before writing
 * it out.  This includes any normal adjustments the user requested via -np,
 * etc.
 *
 * Normally, you should not need to call this function directly;
 * write_egg_file() calls it for you.  You should call this only if you do not
 * use write_egg_file() to write out the resulting egg file.
 */
void SomethingToEgg::
post_process_egg_file() {
  apply_units_scale(_data);
  EggConverter::post_process_egg_file();
}

/**
 * Dispatch function to set the given animation convert mode according to the
 * specified parameter.  var is a pointer to an AnimationConvert variable.
 */
bool SomethingToEgg::
dispatch_animation_convert(const std::string &opt, const std::string &arg, void *var) {
  AnimationConvert *ip = (AnimationConvert *)var;
  (*ip) = string_animation_convert(arg);
  if ((*ip) == AC_invalid) {
    nout << "Invalid keyword for -" << opt << ": " << arg << "\n";
    return false;
  }

  return true;
}
