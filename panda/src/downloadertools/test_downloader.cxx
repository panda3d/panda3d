#include <pandabase.h>
#include <downloader.h>

int
main(int argc, char *argv[]) {

  if (argc < 4) {
    cerr << "Usage: test_downloader <server> <source file> <dest file>"
      << endl;
    return 0;
  }

  string server_name = argv[1];
  Filename src_file = argv[2];
  Filename dest_file = argv[3];

  Downloader dl;
  if (dl.connect_to_server(server_name) == false)
    return 0;

  int ret = dl.initiate(src_file, dest_file);
  if (ret < 0)
    return 0;

  for (;;) {
    ret = dl.run();
    if (ret == Downloader::DS_success) {
      return 1;
    } else if (ret == Downloader::DS_write) {
      cerr << "bytes per second: " << dl.get_bytes_per_second() << endl;
    } else if (ret < 0)
      return 0;
  }
  return 0;
}
