#include <pandabase.h>
#include <downloader.h>
#include <error_utils.h>
#include <decompressor.h>

int
main(int argc, char *argv[]) {

  //if (argc < 4) {
  if (argc < 3) {
    //cerr << "Usage: test_downloader <server> <source file> <dest file>"
    cerr << "Usage: test_downloader <server> <source file>"
      << endl;
    return 0;
  }

  string server_name = argv[1];
  Filename src_file = argv[2];
  //Filename dest_file = argv[3];

  Downloader dl;
  if (dl.connect_to_server(server_name) == false)
    return 0;

  //int ret = dl.initiate(src_file, dest_file);
  int ret = dl.initiate(src_file);
  if (ret < 0)
    return 0;

  //for (;;) {
  bool done = false;
  while (!done) {
    ret = dl.run();
    if (ret == EU_success) {
      cerr << "bytes per second: " << dl.get_bytes_per_second() << endl;
      //return 1;
      done = true;
    } else if (ret == EU_write) {
      cerr << "bytes per second: " << dl.get_bytes_per_second() << endl;
    } else if (ret < 0) {
      cerr << "error!" << endl;
      return 0;
    }
  }

  cerr << "download to memory complete" << endl;
  Ramfile rfile;
  dl.get_ramfile(rfile);
  cerr << "ram file length: " << rfile._data.length() << endl;
  Decompressor dc;
  dc.decompress(rfile);
  cerr << "ram file length: " << rfile._data.length() << endl;

  return 0;
}
