// Filename: test_lwo.cxx
// Created by:  drose (24Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "lwoInputFile.h"
#include "lwoChunk.h"
#include "config_lwo.h"

int
main(int argc, char *argv[]) {
  init_liblwo();
  if (argc != 2) {
    cerr << "test_lwo file.lwo\n";
    exit(1);
  }

  LwoInputFile in;
  if (!in.open_read(argv[1])) {
    cerr << "Unable to open " << argv[1] << "\n";
    exit(1);
  }

  PT(IffChunk) chunk = in.get_chunk();
  while (chunk != (IffChunk *)NULL) {
    cerr << "Got chunk type " << chunk->get_type() << ":\n";
    chunk->write(cerr, 2);
    chunk = in.get_chunk();
  }

  cerr << "EOF = " << in.is_eof() << "\n";

  return (0);
}
