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

#ifdef TESTING_CODE
  Decompressor decompressor;
#else
  PT(Buffer) buffer = new Buffer(1000000);
  Decompressor decompressor(buffer);
#endif
  if (decompressor.decompress(source_file) == false)
    cerr << "Decompress failed" << endl;

  return 1;
}
