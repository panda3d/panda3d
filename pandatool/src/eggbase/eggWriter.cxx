/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggWriter.cxx
 * @author drose
 * @date 2000-02-14
 */

#include "eggWriter.h"

#include "string_utils.h"
#include "compose_matrix.h"
#include "globPattern.h"

/**
 * Egg-writing type programs may specify their output file using either the
 * last-filename convention, the -o convention, and/or implicitly writing the
 * result to standard output.  Not all interfaces are appropriate for all
 * applications; some may be confusing or dangerous.
 *
 * The calling application should pass allow_last_param true to allow the user
 * to specify the output filename as the last parameter on the command line
 * (the most dangerous, but convenient, method), and allow_stdout true to
 * allow the user to omit the output filename altogether and have the output
 * implicitly go to standard output (not terribly dangerous, but inappropriate
 * when writing binary file formats).
 */
EggWriter::
EggWriter(bool allow_last_param, bool allow_stdout) :
  WithOutputFile(allow_last_param, allow_stdout, false)
{
  // Indicate the extension name we expect the user to supply for output
  // files.
  _preferred_extension = ".egg";

  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] output.egg");
  }
  add_runline("[opts] -o output.egg");
  if (_allow_stdout) {
    add_runline("[opts] >output.egg");
  }

  std::string o_description;

  if (_allow_stdout) {
    if (_allow_last_param) {
      o_description =
        "Specify the filename to which the resulting egg file will be written.  "
        "If this option is omitted, the last parameter name is taken to be the "
        "name of the output file, or standard output is used if there are no "
        "other parameters.";
    } else {
      o_description =
        "Specify the filename to which the resulting egg file will be written.  "
        "If this option is omitted, the egg file is written to standard output.";
    }
  } else {
    if (_allow_last_param) {
      o_description =
        "Specify the filename to which the resulting egg file will be written.  "
        "If this option is omitted, the last parameter name is taken to be the "
        "name of the output file.";
    } else {
      o_description =
        "Specify the filename to which the resulting egg file will be written.";
    }
  }

  add_option
    ("o", "filename", 50, o_description,
     &EggWriter::dispatch_filename, &_got_output_filename, &_output_filename);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting egg file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default is "
     "y-up.");
}


/**
 * Returns this object as an EggWriter pointer, if it is in fact an EggWriter,
 * or NULL if it is not.
 *
 * This is intended to work around the C++ limitation that prevents downcasts
 * past virtual inheritance.  Since both EggReader and EggWriter inherit
 * virtually from EggSingleBase, we need functions like this to downcast to
 * the appropriate pointer.
 */
EggWriter *EggWriter::
as_writer() {
  return this;
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
void EggWriter::
post_process_egg_file() {
  if (_got_transform) {
    nout << "Applying transform matrix:\n";
    _transform.write(nout, 2);
    LVecBase3d scale, hpr, translate;
    if (decompose_matrix(_transform, scale, hpr, translate,
                         _data->get_coordinate_system())) {
      nout << "(scale " << scale << ", hpr " << hpr << ", translate "
           << translate << ")\n";
    }
    _data->transform(_transform);
  }

  if (_make_points) {
    nout << "Making points\n";
    _data->make_point_primitives();
  }

  bool needs_remove = false;

  switch (_normals_mode) {
  case NM_strip:
    nout << "Stripping normals.\n";
    _data->strip_normals();
    needs_remove = true;
    break;

  case NM_polygon:
    nout << "Recomputing polygon normals.\n";
    _data->recompute_polygon_normals();
    needs_remove = true;
    break;

  case NM_vertex:
    nout << "Recomputing vertex normals.\n";
    _data->recompute_vertex_normals(_normals_threshold);
    needs_remove = true;
    break;

  case NM_preserve:
    // Do nothing.
    break;
  }

  if (_got_tbnall) {
    needs_remove |= _data->recompute_tangent_binormal(GlobPattern("*"));
  } else {
    if (_got_tbnauto) {
      needs_remove |= _data->recompute_tangent_binormal_auto();
    }
    needs_remove |= _data->recompute_tangent_binormal(_tbn_names);
  }

  if (needs_remove) {
    _data->remove_unused_vertices(true);
  }
}

/**
 * Writes out the egg file as the normal result of the program.  This calls
 * post_process_egg_file() to perform any last minute processing (like normal
 * computation) and then writes out the file to the output stream returned by
 * get_output().
 */
void EggWriter::
write_egg_file() {
  post_process_egg_file();
  _data->write_egg(get_output());
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool EggWriter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 0)) {
    return false;
  }

  if (!args.empty()) {
    nout << "Unexpected arguments on command line:\n";
    Args::const_iterator ai;
    for (ai = args.begin(); ai != args.end(); ++ai) {
      nout << (*ai) << " ";
    }
    nout << "\r";
    return false;
  }

  if (!_got_path_directory && _got_output_filename) {
    // Put in the name of the output directory.
    _path_replace->_path_directory = _output_filename.get_dirname();
  }

  return true;
}

/**
 *
 */
bool EggWriter::
post_command_line() {
  if (!_allow_stdout && !_got_output_filename) {
    nout << "You must specify the filename to write with -o.\n";
    return false;
  }

  append_command_comment(_data);

  return EggSingleBase::post_command_line();
}
