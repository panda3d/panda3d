#include <pandabase.h>
#include <getopt.h>
#include <patchfile.h>
#include <filename.h>

int
main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "Usage: apply_patch <patch> <file>" << endl;
    return 0;
  }

  Filename patch = argv[1];
  patch.set_binary();

  Filename file = argv[2];
  file.set_binary();

  Patchfile pfile;

  cerr << "Applying patch file " << patch << " to " << file << endl;
  if (pfile.apply(patch, file) == false)
    cerr << "apply patch failed" << endl;

  return 1;
}
