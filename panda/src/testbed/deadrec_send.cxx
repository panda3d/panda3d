// Filename: deadrec_send.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"

void deadrec_keys(EventHandler& eh) {
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
