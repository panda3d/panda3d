// Filename: sedProcess.cxx
// Created by:  drose (24Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "sedProcess.h"
#include "sedContext.h"

////////////////////////////////////////////////////////////////////
//     Function: SedProcess::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedProcess::
SedProcess() {
}

////////////////////////////////////////////////////////////////////
//     Function: SedProcess::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SedProcess::
~SedProcess() {
}


////////////////////////////////////////////////////////////////////
//     Function: SedProcess::add_script_line
//       Access: Public
//  Description: Appends the indicated line to the end of the script
//               that will be executed for each line of the input
//               stream.  This may be called as many times as you
//               like.
//
//               The return value is true if the line was added
//               successfully, or false if there was an error in the
//               line (in which case, some commands on the line might
//               have been added, and others not).
////////////////////////////////////////////////////////////////////
bool SedProcess::
add_script_line(const string &line) {
  return _script.add_line(line);
}


////////////////////////////////////////////////////////////////////
//     Function: SedProcess::run
//       Access: Public
//  Description: Reads the input stream and executes the script once
//               for each line on the input stream.  Output is written
//               to the indicated output stream.
////////////////////////////////////////////////////////////////////
void SedProcess::
run(istream &in, ostream &out) {
  SedContext context(out);

  string line;
  getline(in, line);
  while (!in.fail() && !in.eof()) {
    context._pattern_space = line;
    context._line_number++;
    getline(in, line);

    if (in.eof()) {
      context._is_last_line = true;
    }
    _script.run(context);
  }
}
