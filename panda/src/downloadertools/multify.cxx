#include <pandabase.h>
#ifndef HAVE_GETOPT
  #include <gnu_getopt.h>
#else
  #include <getopt.h>
#endif
#include <multifile.h>
#include <filename.h>

int
main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: multify -[x,c|vr] <dest_file> <src_file> ..." << endl;
    return 0;
  }

  bool verbose = true;
  bool extract = false;

  extern char *optarg;
  extern int optind;
  int flag = getopt(argc, argv, "xcvr:");
  Filename rel_path;
  while (flag != EOF) {
    switch (flag) {
      case 'x':
	extract = true;
        break;
      case 'c':
        break;
      case 'v':
	verbose = true;
 	break;
      case 'r':
	rel_path = optarg;
	break;
      default:
	cerr << "Unhandled switch: " << flag << endl;
	break;
    } 
    flag = getopt(argc, argv, "xcvr:");
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  if (argc <= 1) {
    cerr << "Usage: multify -[x,c|v] <dest_file> <src_file> ..." << endl;
    return 0;
  }

  Filename dest_file = argv[1];
  dest_file.set_binary();

  if (verbose == true) {
    if (extract == true)
      cerr << "Extracting from: " << dest_file << endl;
    else
      cerr << "Appending to: " << dest_file << endl;
  }

  Multifile mfile;

  if (extract == false) {
    for (int i = 2; i < argc; i++) {
      Filename in_file = argv[i];
      in_file.set_binary();
      mfile.add(in_file);
    }

    if (mfile.write(dest_file) == false)
      cerr << "Failed to write: " << dest_file << endl;
  } else {
    mfile.read(dest_file);
    mfile.extract_all(rel_path);
  }

  return 1;
}
