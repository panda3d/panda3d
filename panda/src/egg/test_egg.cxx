// Filename: test_egg.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggData.h"
#include <notify.h>


int
main(int argc, char *argv[]) {
  if (argc != 2) {
    nout << "Specify an egg file to load.\n";
    exit(1);
  }
  const char *egg_filename = argv[1];

  EggData data;
  data.set_coordinate_system(CS_default);

  if (data.read(egg_filename)) {
    data.resolve_externals("");
    data.write_egg(cout);
  } else {
    nout << "Errors.\n";
  }
  return (0);
}

