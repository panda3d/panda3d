// Filename: test_flt.cxx
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltHeader.h"

#include <getopt.h>

void
usage() {
  cerr << "Usage: test_flt [opts] filename.flt\n";
}

int
main(int argc, char *argv[]) {
  static const char * const opts = "t:";
  extern char *optarg;
  extern int optind;

  DSearchPath texture_path;

  int flag = getopt(argc, argv, opts);
  while (flag != EOF) {
    switch (flag) {
    case 't':
      // t: Texture search path.
      texture_path.append_directory(optarg);
      break;

    default:
      usage();
      exit(1);
    } 
    flag = getopt(argc, argv, opts);
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  if (argc != 2) {
    usage();
    exit(1);
  }

  Filename filename = argv[1];

  PT(FltHeader) header = new FltHeader;
  header->set_texture_path(texture_path);

  FltError result = header->read_flt(filename);
  cerr << "Read result is " << result << "\n"
       << "Version is " << header->get_flt_version() << "\n";
  header->check_version();

  if (result == FE_ok) {
    //header->write(cerr);

    result = header->write_flt("t.flt");
    cerr << "Write result is " << result << "\n\n";
  }

  return (0);
}
