#include <crypto_utils.h>

int 
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: check_md5 <file>" << endl;
    return 0;
  }

  Filename source_file = argv[1];

  HashVal hash;
  md5_a_file(source_file, hash);
  cout << hash << endl;

  return 1;
}
