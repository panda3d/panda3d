// Filename: fltToEgg.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "fltToEgg.h"

#include <fltToEggConverter.h>

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltToEgg::
FltToEgg() :
  SomethingToEgg("MultiGen", ".flt")
{
  add_normals_options();

  set_program_description
    ("This program converts MultiGen OpenFlight (.flt) files to egg.  Most "
     "features of MultiGen that are also recognized by egg are supported.");

  add_option
    ("rt", "", 0,
     "Convert texture filenames to relative pathnames, relative to the "
     "directory specified by -rd.",
     &FltToEgg::dispatch_path_convert_relative, NULL, &_texture_path_convert);

  add_option
    ("rta", "", 0,
     "Convert texture filenames to absolute pathnames.",
     &FltToEgg::dispatch_path_convert_absolute, NULL, &_texture_path_convert);

  add_option
    ("rtA", "", 0,
     "Convert texture filenames to absolute pathnames that begin with the "
     "prefix specified by -rd.",
     &FltToEgg::dispatch_path_convert_rel_abs, NULL, &_texture_path_convert);

  add_option
    ("rtu", "", 0,
     "Leave texture filenames unchanged.  They will be relative if they "
     "are relative in the flt file, or absolute if they are absolute in "
     "the flt file.",
     &FltToEgg::dispatch_path_convert_unchanged, NULL, &_texture_path_convert);

  add_option
    ("re", "", 0,
     "Convert model filenames (external references) to relative pathnames, "
     "relative to the directory specified by -rd.",
     &FltToEgg::dispatch_path_convert_relative, NULL, &_model_path_convert);

  add_option
    ("rea", "", 0,
     "Convert model filenames to absolute pathnames.",
     &FltToEgg::dispatch_path_convert_absolute, NULL, &_model_path_convert);

  add_option
    ("reA", "", 0,
     "Convert model filenames to absolute pathnames that begin with the "
     "prefix specified by -rd.",
     &FltToEgg::dispatch_path_convert_rel_abs, NULL, &_model_path_convert);

  add_option
    ("reu", "", 0,
     "Leave model filenames unchanged.  They will be relative if they "
     "are relative in the flt file, or absolute if they are absolute in "
     "the flt file.",
     &FltToEgg::dispatch_path_convert_unchanged, NULL, &_model_path_convert);

  add_option
    ("rd", "dir", 0,
     "Specify the directory to make relative to.  This is the "
     "directory that all pathnames given in the flt file will be "
     "rewritten to be relative to, if -re or -rt is given.  It is "
     "ignored if one of these options is not given.  If omitted, it "
     "is taken from the source filename.",
     &FltToEgg::dispatch_string, &_got_make_rel_dir, &_make_rel_dir);

  add_option
    ("rs", "path", 0, 
     "A search path for textures and external file references.  This "
     "is a colon-separated set of directories that will be searched "
     "for filenames that are not fully specified in the flt file.  It "
     "is unrelated to -re and -rt, and is used only if the flt file "
     "does not store absolute pathnames.  The directory containing "
     "the source filename is always implicitly included.",
     &FltToEgg::dispatch_search_path, NULL, &_search_path);

  redescribe_option
    ("cs",
     "Specify the coordinate system of the input " + _format_name +
     " file.  Normally, this is z-up.");

  _texture_path_convert = FltToEggConverter::PC_unchanged;
  _model_path_convert = FltToEggConverter::PC_unchanged;
  _coordinate_system = CS_zup_right;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FltToEgg::
run() {
  if (!_got_make_rel_dir) {
    _make_rel_dir = _input_filename.get_dirname();
  }

  PT(FltHeader) header = new FltHeader;
  header->set_texture_path(_search_path);
  header->set_model_path(_search_path);

  nout << "Reading " << _input_filename << "\n";
  FltError result = header->read_flt(_input_filename);
  if (result != FE_ok) {
    nout << "Unable to read: " << result << "\n";
    exit(1);
  }

  header->check_version();

  _data.set_coordinate_system(_coordinate_system);

  if (_input_units == DU_invalid) {
    _input_units = header->get_units();
  }

  FltToEggConverter converter(_data);
  converter.set_texture_path_convert(_texture_path_convert, _make_rel_dir);
  converter.set_model_path_convert(_model_path_convert, _make_rel_dir);

  if (!converter.convert_flt(header)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  write_egg_file();
  nout << "\n";
}


////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::dispatch_path_convert_relative
//       Access: Protected, Static
//  Description: Dispatch function to set the given path convert mode
//               to PC_relative.  var is a pointer to a
//               FltToEggConverter::PathConvert variable.
////////////////////////////////////////////////////////////////////
bool FltToEgg::
dispatch_path_convert_relative(const string &opt, const string &, void *var) {
  FltToEggConverter::PathConvert *ip = (FltToEggConverter::PathConvert *)var;
  (*ip) = FltToEggConverter::PC_relative;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::dispatch_path_convert_absolute
//       Access: Protected, Static
//  Description: Dispatch function to set the given path convert mode
//               to PC_absolute.  var is a pointer to a
//               FltToEggConverter::PathConvert variable.
////////////////////////////////////////////////////////////////////
bool FltToEgg::
dispatch_path_convert_absolute(const string &opt, const string &, void *var) {
  FltToEggConverter::PathConvert *ip = (FltToEggConverter::PathConvert *)var;
  (*ip) = FltToEggConverter::PC_absolute;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::dispatch_path_convert_rel_abs
//       Access: Protected, Static
//  Description: Dispatch function to set the given path convert mode
//               to PC_rel_abs.  var is a pointer to a
//               FltToEggConverter::PathConvert variable.
////////////////////////////////////////////////////////////////////
bool FltToEgg::
dispatch_path_convert_rel_abs(const string &opt, const string &, void *var) {
  FltToEggConverter::PathConvert *ip = (FltToEggConverter::PathConvert *)var;
  (*ip) = FltToEggConverter::PC_rel_abs;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEgg::dispatch_path_convert_unchanged
//       Access: Protected, Static
//  Description: Dispatch function to set the given path convert mode
//               to PC_unchanged.  var is a pointer to a
//               FltToEggConverter::PathConvert variable.
////////////////////////////////////////////////////////////////////
bool FltToEgg::
dispatch_path_convert_unchanged(const string &opt, const string &, void *var) {
  FltToEggConverter::PathConvert *ip = (FltToEggConverter::PathConvert *)var;
  (*ip) = FltToEggConverter::PC_unchanged;
  return true;
}

int main(int argc, char *argv[]) {
  FltToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
