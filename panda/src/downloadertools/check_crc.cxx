#include <download_utils.h>

int 
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: check_crc <file>" << endl;
    return 0;
  }

  Filename source_file = argv[1];

  cout << check_crc(source_file);

  return 1;
}
