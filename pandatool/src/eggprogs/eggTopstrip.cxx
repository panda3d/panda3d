// Filename: eggTopstrip.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggTopstrip.h"

////////////////////////////////////////////////////////////////////
//     Function: EggTopstrip::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggTopstrip::
EggTopstrip() {
  set_program_description
    ("egg-topstrip reads a character model and its associated animation "
     "files, and unapplies the animation from one of the top joints.  "
     "This effectively freezes that particular joint, and makes the rest "
     "of the character relative to that joint.\n\n"

     "This is a particularly useful thing to do to generate character "
     "models that can stack one on top of the other in a sensible way.");
}

////////////////////////////////////////////////////////////////////
//     Function: EggTopstrip::run
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void EggTopstrip::
run() {
}


int main(int argc, char *argv[]) {
  EggTopstrip prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
