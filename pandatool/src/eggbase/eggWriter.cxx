// Filename: eggWriter.cxx
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggWriter.h"

#include <string_utils.h>
#include <compose_matrix.h>

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::Constructor
//       Access: Public
//  Description: Egg-writing type programs may specify their output
//               file using either the last-filename convention, the
//               -o convention, and/or implicitly writing the result
//               to standard output.  Not all interfaces are
//               appropriate for all applications; some may be
//               confusing or dangerous.
//
//               The calling application should pass allow_last_param
//               true to allow the user to specify the output filename
//               as the last parameter on the command line (the most
//               dangerous, but convenient, method), and allow_stdout
//               true to allow the user to omit the output filename
//               altogether and have the output implicitly go to
//               standard output (not terribly dangerous, but
//               inappropriate when writing binary file formats).
////////////////////////////////////////////////////////////////////
EggWriter::
EggWriter(bool allow_last_param, bool allow_stdout) {
  _allow_last_param = allow_last_param;
  _allow_stdout = allow_stdout;

  clear_runlines();
  if (_allow_last_param) {
    add_runline("[opts] output.egg");
  }
  add_runline("[opts] -o output.egg");
  if (_allow_stdout) {
    add_runline("[opts] >output.egg");
  }

  string o_description;

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

  _output_ptr = (ostream *)NULL;

  _normals_mode = NM_preserve;
  _normals_threshold = 0.0;

  _got_transform = false;
  _transform = LMatrix4d::ident_mat();
}


////////////////////////////////////////////////////////////////////
//     Function: EggWriter::add_normals_options();
//       Access: Public
//  Description: Adds -no, -np, etc. as valid options for this
//               program.  If the user specifies one of the options on
//               the command line, the normals will be adjusted when
//               the egg file is written out.
////////////////////////////////////////////////////////////////////
void EggWriter::
add_normals_options() {
  static NormalsMode strip = NM_strip;
  static NormalsMode polygon = NM_polygon;
  static NormalsMode vertex = NM_vertex;
  static NormalsMode preserve = NM_preserve;

  add_option
    ("no", "", 49, 
     "Strip all normals.",
     &EggWriter::dispatch_normals, NULL, &strip);

  add_option
    ("np", "", 49, 
     "Strip existing normals and redefine polygon normals.",
     &EggWriter::dispatch_normals, NULL, &polygon);

  add_option
    ("nv", "threshold", 49, 
     "Strip existing normals and redefine vertex normals.  Consider an edge "
     "between adjacent polygons to be smooth if the angle between them "
     "is less than threshold degrees.",
     &EggWriter::dispatch_normals, NULL, &vertex);

  add_option
    ("nn", "", 49, 
     "Preserve normals exactly as they are.  This is the default.",
     &EggWriter::dispatch_normals, NULL, &preserve);
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::add_transform_options();
//       Access: Public
//  Description: Adds -TS, -TT, etc. as valid options for this
//               program.  If the user specifies one of the options on
//               the command line, the data will be transformed when
//               the egg file is written out.
////////////////////////////////////////////////////////////////////
void EggWriter::
add_transform_options() {
  add_option
    ("TS", "sx[,sy,sz]", 49, 
     "Scale the model uniformly by the given factor (if only one number "
     "is given) or in each axis by sx, sy, sz (if three numbers are given).",
     &EggWriter::dispatch_scale, &_got_transform, &_transform);

  add_option
    ("TR", "angle,x,y,z", 49, 
     "Rotate the model angle degrees counterclockwise about the given "
     "axis.",
     &EggWriter::dispatch_rotate, &_got_transform, &_transform);

  add_option
    ("TT", "x,y,z", 49, 
     "Translate the model by the indicated amount.\n\n"
     "All transformation options (-TS, -TR, -TT) are cumulative and are "
     "applied in the order they are encountered on the command line.",
     &EggWriter::dispatch_translate, &_got_transform, &_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::as_writer
//       Access: Public, Virtual
//  Description: Returns this object as an EggWriter pointer, if it is
//               in fact an EggWriter, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggWriter *EggWriter::
as_writer() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::get_output
//       Access: Public
//  Description: Returns an output stream that corresponds to the
//               user's intended egg file output--either stdout, or
//               the named output file.  Normally, you should not need
//               to call this directly, as write_egg_file() will write
//               the egg file to the resulting stream.  Call this only
//               if you cannot use write_egg_file() to write out the
//               egg file for some reason.
////////////////////////////////////////////////////////////////////
ostream &EggWriter::
get_output() {
  if (_output_ptr == (ostream *)NULL) {
    if (!_got_output_filename) {
      // No filename given; use standard output.
      assert(_allow_stdout);
      _output_ptr = &cout;
      
    } else {
      // Attempt to open the named file.
      unlink(_output_filename.c_str());
      _output_filename.make_dir();
      _output_filename.set_text();
      if (!_output_filename.open_write(_output_stream)) {
	nout << "Unable to write to " << _output_filename << "\n";
	exit(1);
      }
      nout << "Writing " << _output_filename << "\n";
      _output_ptr = &_output_stream;
    }
  }
  return *_output_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::has_output_filename
//       Access: Public
//  Description: Returns true if the user specified an output
//               filename, false otherwise (e.g. the output file is
//               implicitly stdout).
////////////////////////////////////////////////////////////////////
bool EggWriter::
has_output_filename() const {
  return _got_output_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::get_output_filename
//       Access: Public
//  Description: If has_output_filename() returns true, this is the
//               filename that the user specified.  Otherwise, it
//               returns the empty string.
////////////////////////////////////////////////////////////////////
Filename EggWriter::
get_output_filename() const {
  if (_got_output_filename) {
    return _output_filename;
  }
  return Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::post_process_egg_file
//       Access: Public, Virtual
//  Description: Performs any processing of the egg file that is
//               appropriate before writing it out.  This includes any
//               normal adjustments the user requested via -np, etc.
//
//               Normally, you should not need to call this function
//               directly; write_egg_file() calls it for you.  You
//               should call this only if you do not use
//               write_egg_file() to write out the resulting egg file.
////////////////////////////////////////////////////////////////////
void EggWriter::
post_process_egg_file() {
  if (_got_transform) {
    nout << "Applying transform matrix:\n";
    _transform.write(nout, 2);
    LVecBase3d scale, hpr, translate;
    if (decompose_matrix(_transform, scale, hpr, translate, 
			 _data.get_coordinate_system())) {
      nout << "(scale " << scale << ", hpr " << hpr << ", translate "
	   << translate << ")\n";
    }
    _data.transform(_transform);
  }

  switch (_normals_mode) {
  case NM_strip:
    nout << "Stripping normals.\n";
    _data.strip_normals();
    _data.remove_unused_vertices();
    break;

  case NM_polygon:
    nout << "Recomputing polygon normals.\n";
    _data.recompute_polygon_normals();
    _data.remove_unused_vertices();
    break;

  case NM_vertex:
    nout << "Recomputing vertex normals.\n";
    _data.recompute_vertex_normals(_normals_threshold);
    _data.remove_unused_vertices();
    break;

  case NM_preserve:
    // Do nothing.
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::write_egg_file
//       Access: Public
//  Description: Writes out the egg file as the normal result of the
//               program.  This calls post_process_egg_file() to
//               perform any last minute processing (like normal
//               computation) and then writes out the file to the
//               output stream returned by get_output().
////////////////////////////////////////////////////////////////////
void EggWriter::
write_egg_file() {
  post_process_egg_file();
  _data.write_egg(get_output());
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggWriter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args)) {
    return false;
  }

  if (!args.empty()) {
    nout << "Unexpected arguments on command line:\n";
    copy(args.begin(), args.end(), ostream_iterator<string>(nout, " "));
    nout << "\r";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::check_last_arg
//       Access: Protected
//  Description: Checks if the last filename on the argument list is
//               an egg file (if _allow_last_param was set true), and
//               removes it from the argument list if it is.  Returns
//               true if the arguments are good, false if something is
//               invalid.
////////////////////////////////////////////////////////////////////
bool EggWriter::
check_last_arg(ProgramBase::Args &args) {
  if (_allow_last_param && !_got_output_filename && !args.empty()) {
    Filename filename = args.back();
    if (filename.get_extension() == "egg") {
      _got_output_filename = true;
      _output_filename = filename;
      args.pop_back();

      if (!verify_output_file_safe()) {
	return false;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::post_command_line
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggWriter::
post_command_line() {
  if (!_allow_stdout && !_got_output_filename) {
    nout << "You must specify the filename to write with -o.\n";
    return false;
  }

  append_command_comment(_data);
  
  return EggBase::post_command_line();
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::dispatch_normals
//       Access: Protected, Static
//  Description: Accepts one of -no, -np, etc. and sets _normals_mode
//               as indicated.  The void * argument is a pointer to a
//               NormalsMode variable that indicates which switch was
//               passed.
////////////////////////////////////////////////////////////////////
bool EggWriter::
dispatch_normals(ProgramBase *self, const string &opt, const string &arg, void *mode) {
  EggBase *base = (EggBase *)self;
  EggWriter *me = base->as_writer();
  return me->ns_dispatch_normals(opt, arg, mode);
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::ns_dispatch_normals
//       Access: Protected
//  Description: Accepts one of -no, -np, etc. and sets _normals_mode
//               as indicated.  The void * argument is a pointer to a
//               NormalsMode variable that indicates which switch was
//               passed.
////////////////////////////////////////////////////////////////////
bool EggWriter::
ns_dispatch_normals(const string &opt, const string &arg, void *mode) {
  _normals_mode = *(NormalsMode *)mode;

  if (_normals_mode == NM_vertex) {
    if (!string_to_double(arg, _normals_threshold)) {
      nout << "Invalid numeric parameter for -" << opt << ": " 
	   << arg << "\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::dispatch_scale
//       Access: Protected, Static
//  Description: Handles -TS, which specifies a scale transform.  Var
//               is an LMatrix4d.
////////////////////////////////////////////////////////////////////
bool EggWriter::
dispatch_scale(const string &opt, const string &arg, void *var) {
  LMatrix4d *transform = (LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  double sx, sy, sz;

  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_double(words[0], sx) &&
      string_to_double(words[1], sy) &&
      string_to_double(words[2], sz);

  } else if (words.size() == 1) {
    okflag =
      string_to_double(words[0], sx);
    sy = sz = sx;
  }

  if (!okflag) {
    nout << "-" << opt
	 << " requires one or three numbers separated by commas.\n";
    return false;
  }

  *transform = (*transform) * LMatrix4d::scale_mat(sx, sy, sz);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::dispatch_rotate
//       Access: Protected, Static
//  Description: Handles -TR, which specifies a rotate transform.  Var
//               is an LMatrix4d.
////////////////////////////////////////////////////////////////////
bool EggWriter::
dispatch_rotate(ProgramBase *self, const string &opt, const string &arg, void *var) {
  EggBase *base = (EggBase *)self;
  EggWriter *me = base->as_writer();
  return me->ns_dispatch_rotate(opt, arg, var);
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::ns_dispatch_rotate
//       Access: Protected
//  Description: Handles -TR, which specifies a rotate transform.  Var
//               is an LMatrix4d.
////////////////////////////////////////////////////////////////////
bool EggWriter::
ns_dispatch_rotate(const string &opt, const string &arg, void *var) {
  LMatrix4d *transform = (LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  double angle;
  LVecBase3d axis;

  bool okflag = false;
  if (words.size() == 4) {
    okflag =
      string_to_double(words[0], angle) &&
      string_to_double(words[1], axis[0]) &&
      string_to_double(words[2], axis[1]) &&
      string_to_double(words[3], axis[2]);
  }

  if (!okflag) {
    nout << "-" << opt
	 << " requires four numbers separated by commas.\n";
    return false;
  }

  *transform = (*transform) * LMatrix4d::rotate_mat(angle, axis, _coordinate_system);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::dispatch_translate
//       Access: Protected, Static
//  Description: Handles -TT, which specifies a translate transform.
//               Var is an LMatrix4d.
////////////////////////////////////////////////////////////////////
bool EggWriter::
dispatch_translate(const string &opt, const string &arg, void *var) {
  LMatrix4d *transform = (LMatrix4d *)var;

  vector_string words;
  tokenize(arg, words, ",");

  LVector3d trans;

  bool okflag = false;
  if (words.size() == 3) {
    okflag =
      string_to_double(words[0], trans[0]) &&
      string_to_double(words[1], trans[1]) &&
      string_to_double(words[2], trans[2]);
  }

  if (!okflag) {
    nout << "-" << opt
	 << " requires three numbers separated by commas.\n";
    return false;
  }

  *transform = (*transform) * LMatrix4d::translate_mat(trans);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggWriter::verify_output_file_safe
//       Access: Protected
//  Description: This is called when the output file is given as the
//               last parameter on the command line.  Since this is a
//               fairly dangerous way to specify the output file (it's
//               easy to accidentally overwrite an input file this
//               way), the convention is to disallow this syntax if
//               the output file already exists.
//
//               This function will test if the output file exists,
//               and issue a warning message if it does, returning
//               false.  If all is well, it will return true.
////////////////////////////////////////////////////////////////////
bool EggWriter::
verify_output_file_safe() const {
  nassertr(_got_output_filename, false);

  if (_output_filename.exists()) {
    nout << "The output filename " << _output_filename << " already exists.  "
      "If you wish to overwrite it, you must use the -o option to specify "
      "the output filename, instead of simply specifying it as the last "
      "parameter.\n";
    return false;
  }
  return true;
}
