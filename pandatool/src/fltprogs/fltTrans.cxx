// Filename: fltTrans.cxx
// Created by:  drose (11Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "fltTrans.h"

#include <fltHeader.h>

////////////////////////////////////////////////////////////////////
//     Function: FltTrans::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltTrans::
FltTrans() :
  WithOutputFile(true, false, true)
{
  // Indicate the extension name we expect the user to supply for
  // output files.
  _preferred_extension = ".flt";
  
  set_program_description
    ("This program reads a MultiGen OpenFlight (.flt) file and writes an "
     "essentially equivalent .flt file, to the file specified with -o (or "
     "as the second parameter).  Some simple operations may be performed.");

  clear_runlines();
  add_runline("[opts] input.flt output.flt");
  add_runline("[opts] -o output.flt input.flt");

  add_option
    ("tp", "path", 0, 
     "Add the indicated colon-delimited paths to the path that is searched "
     "for textures referenced by the flt file.  This "
     "option may also be repeated to add multiple paths.",
     &FltTrans::dispatch_search_path, NULL, &_texture_path);

  add_option
    ("v", "version", 0, 
     "Upgrade (or downgrade) the flt file to the indicated version.  This "
     "may not be completely correct for all version-to-version combinations.",
     &FltTrans::dispatch_double, &_got_new_version, &_new_version);

  add_option
    ("o", "filename", 0,
     "Specify the filename to which the resulting .flt file will be written.  "
     "If this option is omitted, the last parameter name is taken to be the "
     "name of the output file.",
     &FltTrans::dispatch_filename, &_got_output_filename, &_output_filename);
}


////////////////////////////////////////////////////////////////////
//     Function: FltTrans::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void FltTrans::
run() {
  if (_got_new_version) {
    if (_new_version < FltHeader::min_flt_version() ||
	_new_version > FltHeader::max_flt_version()) {
      nout << "Cannot write flt files of version " << _new_version
	   << ".  This program only understands how to write flt files between version " 
	   << FltHeader::min_flt_version() << " and " 
	   << FltHeader::max_flt_version() << ".\n";
      exit(1);
    }
  }

  PT(FltHeader) header = new FltHeader;
  header->set_texture_path(_texture_path);

  nout << "Reading " << _input_filename << "\n";
  FltError result = header->read_flt(_input_filename);
  if (result != FE_ok) {
    nout << "Unable to read: " << result << "\n";
    exit(1);
  }

  if (header->check_version()) {
    nout << "Version is " << header->get_flt_version() << "\n";
  }
  
  if (_got_new_version) {
    header->set_flt_version(_new_version);
  }
  
  result = header->write_flt(get_output());
  if (result != FE_ok) {
    nout << "Unable to write: " << result << "\n";
    exit(1);
  }
  
  nout << "Successfully written.\n";
}


////////////////////////////////////////////////////////////////////
//     Function: FltTrans::handle_args
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool FltTrans::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 1)) {
    return false;
  }

  if (args.empty()) {
    nout << "You must specify the .flt file to read on the command line.\n";
    return false;

  } else if (args.size() != 1) {
    nout << "You must specify only one .flt file to read on the command line.\n";
    return false;
  }

  _input_filename = args[0];

  return true;
}


int main(int argc, char *argv[]) {
  FltTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
