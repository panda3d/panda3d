#include <pandabase.h>
#include <getopt.h>
#include <patchfile.h>
#include <filename.h>

int
main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: build_patch <src_file> <dest_file>" << endl;
    return 0;
  }

  Filename src_file = argv[1];
  src_file.set_binary();

  Filename dest_file = argv[2];
  dest_file.set_binary();

  Patchfile pfile;

  cerr << "Building patch file to convert " << src_file << " to "
    << dest_file << endl;
  if (pfile.build(src_file, dest_file) == false)
    cerr << "build patch failed" << endl;

  return 1;
}
