#include <pandabase.h>
#include <getopt.h>
#include <multifile.h>
#include <filename.h>

int
main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: multify -[x,c|v] <dest_file> <src_file> ..." << endl;
    return 0;
  }

  bool verbose = true;
  bool extract = false;

  extern char *optarg;
  extern int optind;
  int flag = getopt(argc, argv, "x:c:v");
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
      default:
	cerr << "Unhandled switch: " << flag << endl;
	break;
    } 
    flag = getopt(argc, argv, "x:c:v");
  }
  argc -= (optind - 1);
  argv += (optind - 1);

  Filename dest_file = argv[0];
  dest_file.set_binary();

  if (verbose == true) {
    if (extract == true)
      cerr << "Extracting from: " << dest_file << endl;
    else
      cerr << "Appending to: " << dest_file << endl;
  }

  Multifile mfile;

  if (extract == false) {
    for (int i = 1; i < argc; i++) {
      Filename in_file = argv[i];
      in_file.set_binary();
      mfile.add(in_file);
    }

    if (mfile.write(dest_file) == false)
      cerr << "Failed to write: " << dest_file << endl;
  } else {
    mfile.read(dest_file);
    mfile.extract_all();
  }

  return 1;
}
