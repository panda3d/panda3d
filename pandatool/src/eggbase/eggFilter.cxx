// Filename: eggFilter.cxx
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggFilter.h"

////////////////////////////////////////////////////////////////////
//     Function: EggFilter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggFilter::
EggFilter(bool allow_last_param, bool allow_stdout) : 
  EggWriter(allow_last_param, allow_stdout) 
{
  clear_runlines();
  if (allow_last_param) {
    add_runline("[opts] input.egg output.egg");
  }
  add_runline("[opts] -o output.egg input.egg");
  if (allow_stdout) {
    add_runline("[opts] input.egg >output.egg");
  }

  redescribe_option
    ("cs",
     "Specify the coordinate system of the resulting egg file.  This may be "
     "one of 'y-up', 'z-up', 'y-up-left', or 'z-up-left'.  The default "
     "is the same coordinate system as the input egg file.  If this is "
     "different from the input egg file, a conversion will be performed.");
}


////////////////////////////////////////////////////////////////////
//     Function: EggFilter::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggFilter::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args)) {
    return false;
  }

  return EggReader::handle_args(args);
}
