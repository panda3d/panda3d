#include <filename.h>
#include <decompressor.h>
#include <buffer.h>

int 
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: pdecompress <file>" << endl;
    return 0;
  }

  Filename source_file = argv[1];

  PT(Buffer) buffer = new Buffer(1000000);
  Decompressor decompressor(buffer);
  if (decompressor.decompress(source_file) == false)
    cerr << "Decompress failed" << endl;

  return 1;
}
