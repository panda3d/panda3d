// Filename: dcparse.cxx
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#include <dcbase.h>
#include <dcFile.h>

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr <<
      "dcparse - a simple program to read one or more .dc files and report their\n"
      "contents to standard output.\n\n";
    return (1);
  }

  DCFile file;
  for (int i = 1; i < argc; i++) {
    if (!file.read(argv[i])) {
      return (1);
    }
  }

  if (!file.write(cout, "standard output")) {
    return (1);
  }

  return (0);
}
