// Filename: eggTrans.cxx
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "eggTrans.h"

////////////////////////////////////////////////////////////////////
//     Function: EggTrans::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggTrans::
EggTrans() {
  set_program_description
    ("This program reads an egg file and writes an essentially equivalent "
     "egg file to the standard output, or to the file specified with -o.  "
     "Some simple operations on the egg file are supported."); 
}

////////////////////////////////////////////////////////////////////
//     Function: EggTrans::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggTrans::
run() {
  _data.write_egg(get_output());
}


int main(int argc, char *argv[]) {
  EggTrans prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
