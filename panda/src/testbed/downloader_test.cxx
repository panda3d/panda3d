// Filename: downloader_test.cxx
// Created by:  
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "eventHandler.h"
#include "chancfg.h"
#include "downloader.h"
#include "framework.h"
#include "decompressor.h"
#include "extractor.h"

Downloader downloader;
Decompressor decompressor;
Extractor extractor;
Filename src_file;
Filename dest_file;

void event_y(CPT_Event) {
  downloader.request_download(src_file, dest_file, "");
}

void event_u(CPT_Event) {
  downloader.request_download("/oldfoobar.mfz", "/fit/people/mike/download/oldfoobar.mfz", "");
}

void event_i(CPT_Event) {
  downloader.request_download("/foobar.mfz", "/fit/people/mike/download/partfoobar.mfz", "", 0, 99, 151);
}

void event_o(CPT_Event) {
  downloader.request_download("/foobar.mfz", "/fit/people/mike/download/partfoobar.mfz", "", 100, 151, 151);
}

void event_p(CPT_Event) {
  decompressor.request_decompress("/fit/people/mike/download/dload.mf.pz", "");
}

void event_la(CPT_Event) {
  extractor.request_extract("/fit/people/mike/download/dload.mf", "");
}

void loader_keys(EventHandler& eh) {
  eh.add_hook("y", event_y);
  eh.add_hook("u", event_u);
  eh.add_hook("i", event_i);
  eh.add_hook("o", event_o);
  eh.add_hook("p", event_p);
  eh.add_hook("[", event_la);
}

int main(int argc, char *argv[]) {

  if (argc < 4) {
    cerr << "Usage: download <server> <source file> <dest file>" << endl;
    return 0;
  }

  string server_name = argv[1];
  src_file = argv[2];
  dest_file = argv[3];
  argc -= 3;
  argv += 3;

  define_keys = &loader_keys;
  downloader.create_thread();
  downloader.connect_to_server(server_name);
  decompressor.create_thread();
  extractor.create_thread();
  return framework_main(argc, argv);
}
