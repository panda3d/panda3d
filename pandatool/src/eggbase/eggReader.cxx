// Filename: eggReader.cxx
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggReader.h"

////////////////////////////////////////////////////////////////////
//     Function: EggReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggReader::
EggReader() {
  clear_runlines();
  add_runline("[opts] input.egg");

  redescribe_option
    ("cs",
     "Specify the coordinate system to operate in.  This may be "
     " one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is the coordinate system of the input egg file.");

  add_option
    ("f", "", 80, 
     "Force complete loading: load up the egg file along with all of its "
     "external references.",
     &EggReader::dispatch_none, &_force_complete);
}

////////////////////////////////////////////////////////////////////
//     Function: EggReader::as_reader
//       Access: Public, Virtual
//  Description: Returns this object as an EggReader pointer, if it is
//               in fact an EggReader, or NULL if it is not.
//
//               This is intended to work around the C++ limitation
//               that prevents downcasts past virtual inheritance.
//               Since both EggReader and EggWriter inherit virtually
//               from EggBase, we need functions like this to downcast
//               to the appropriate pointer.
////////////////////////////////////////////////////////////////////
EggReader *EggReader::
as_reader() {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggReader::handle_args
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggReader::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "You must specify the egg file(s) to read on the command line.\n";
    return false;
  }

  // Any separate egg files that are listed on the command line will
  // get implicitly loaded up into one big egg file.

  DSearchPath local_path(".");

  Args::const_iterator ai;
  for (ai = args.begin(); ai != args.end(); ++ai) {
    Filename filename = *ai;
    // First, we always try to resolve a filename from the current
    // directory.  This means a local filename will always be found
    // before the model path is searched.
    filename.resolve_filename(local_path);

    if (!_data.read(filename)) {
      // Rather than returning false, we simply exit here, so the
      // ProgramBase won't try to tell the user how to run the program
      // just because we got a bad egg file.
      exit(1);
    }

    if (_force_complete) {
      if (!_data.resolve_externals()) {
	exit(1);
      }
    }
  }

  return true;
}
