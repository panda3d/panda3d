// Filename: panda3d.h
// Created by:  drose (30Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PANDA3D_H
#define PANDA3D_H

// This program must link with Panda for HTTPClient support.  This
// means it probably should be built with LINK_ALL_STATIC defined, so
// we won't have to deal with confusing .dll or .so files that might
// compete on the disk with the dynamically-loaded versions.  There's
// no competition in memory address space, though, because
// p3d_plugin--the only file we dynamically link in--doesn't itself
// link with Panda.

#include "pandabase.h"
#include "p3d_plugin.h"
#include "httpChannel.h"
#include "ramfile.h"
#include "fileSpec.h"
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : Panda3D
// Description : A standalone program that invokes the Panda3D plugin
//               to launch .p3d files.
////////////////////////////////////////////////////////////////////
class Panda3D {
public:
  Panda3D();

  int run(int argc, char *argv[]);

private:
  bool get_plugin(const string &download_url, const string &this_platform,
                  bool verify_contents);
  bool read_contents_file(Filename contents_filename,
                          const string &download_url,
                          const string &this_platform, bool verify_contents);
  bool get_core_api(const Filename &contents_filename, 
                    const string &download_url, const string &this_platform,
                    bool verify_contents, TiXmlElement *xplugin);
  void run_getters();
  void handle_request(P3D_request *request);
  void make_parent_window(P3D_window_handle &parent_window, 
                          int win_width, int win_height);

  P3D_instance *
  create_instance(const string &p3d, P3D_window_type window_type,
                  int win_x, int win_y, int win_width, int win_height,
                  P3D_window_handle parent_window,
                  char **args, int num_args);
  void delete_instance(P3D_instance *instance);

  void usage();
  bool parse_token(char *arg);
  bool parse_int_pair(char *arg, int &x, int &y);

  void report_downloading_package(P3D_instance *instance);
  void report_download_complete(P3D_instance *instance);

#ifdef __APPLE__
  static pascal void st_timer_callback(EventLoopTimerRef timer, void *user_data);
  void timer_callback(EventLoopTimerRef timer);
#endif

private:
  string _root_dir;
  string _log_dirname;
  string _log_basename;
  FileSpec _core_api_dll;
  bool _reporting_download;

  typedef pset<P3D_instance *> Instances;
  Instances _instances;

  typedef pvector<P3D_token> Tokens;
  Tokens _tokens;

  // This nested class keeps track of active URL requests.
  class URLGetter {
  public:
    URLGetter(P3D_instance *instance, int unique_id,
              const URLSpec &url, const string &post_data);
    
    bool run();
    inline P3D_instance *get_instance();
    
  private:
    P3D_instance *_instance;
    int _unique_id;
    URLSpec _url;
    string _post_data;
    
    PT(HTTPChannel) _channel;
    Ramfile _rf;
    size_t _bytes_sent;
  };
  
  typedef pset<URLGetter *> URLGetters;
  URLGetters _url_getters;
};

#include "panda3d.I"

#endif

