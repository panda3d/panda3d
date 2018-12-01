/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda3dBase.h
 * @author rdb
 * @date 2009-12-07
 */

#ifndef PANDA3DBASE_H
#define PANDA3DBASE_H

// This program must link with Panda for HTTPClient support.  This means it
// probably should be built with LINK_ALL_STATIC defined, so we won't have to
// deal with confusing .dll or .so files that might compete on the disk with
// the dynamically-loaded versions.  There's no competition in memory address
// space, though, because p3d_plugin--the only file we dynamically link in--
// doesn't itself link with Panda.

#include "pandabase.h"
#include "p3d_plugin.h"
#include "httpChannel.h"
#include "ramfile.h"
#include "fileSpec.h"
#include "pset.h"
#include "vector_string.h"

/**
 * Base for creating a standalone application that invokes the panda3d plugin
 * to launch .p3d files.
 */
class Panda3DBase {
public:
  Panda3DBase(bool console_environment);

  void run_main_loop();

protected:
  void run_getters();
  void handle_request(P3D_request *request);
  void make_parent_window();

  P3D_instance *
  create_instance(const std::string &p3d, bool start_instance,
                  char **args, int num_args, int p3d_offset = 0);
  void delete_instance(P3D_instance *instance);

  bool read_p3d_info(const Filename &p3d_filename, int p3d_offset = 0);
  bool parse_token(const char *arg);
  bool parse_int_pair(const char *arg, int &x, int &y);
  std::string lookup_token(const std::string &keyword) const;
  static int compare_seq(const std::string &seq_a, const std::string &seq_b);
  static int compare_seq_int(const char *&num_a, const char *&num_b);
  static bool is_url(const std::string &param);

  void report_downloading_package(P3D_instance *instance);
  void report_download_complete(P3D_instance *instance);

  inline bool time_to_exit();

#ifdef __APPLE__
  static pascal void st_timer_callback(EventLoopTimerRef timer, void *user_data);
  void timer_callback(EventLoopTimerRef timer);
#endif

protected:
  std::string _host_url;
  std::string _root_dir;
  std::string _host_dir;
  std::string _start_dir;
  std::string _log_dirname;
  std::string _log_basename;
  std::string _this_platform;
  std::string _coreapi_platform;
  P3D_verify_contents _verify_contents;
  time_t _contents_expiration;

  P3D_window_type _window_type;
  P3D_window_handle _parent_window;
  int _win_x, _win_y;
  int _win_width, _win_height;
  bool _got_win_size;
  bool _exit_with_last_instance;

  bool _reporting_download;
  bool _enable_security;
  bool _console_environment;
  bool _prepend_filename_to_args;

  typedef pset<P3D_instance *> Instances;
  Instances _instances;

  typedef pvector<P3D_token> Tokens;
  Tokens _tokens;

  // This nested class keeps track of active URL requests.
  class URLGetter {
  public:
    URLGetter(P3D_instance *instance, int unique_id,
              const URLSpec &url, const std::string &post_data);

    bool run();
    inline P3D_instance *get_instance();

  private:
    P3D_instance *_instance;
    int _unique_id;
    URLSpec _url;
    std::string _post_data;

    PT(HTTPChannel) _channel;
    Ramfile _rf;
    size_t _bytes_sent;
  };

  typedef pset<URLGetter *> URLGetters;
  URLGetters _url_getters;
};

#include "panda3dBase.I"

#endif
