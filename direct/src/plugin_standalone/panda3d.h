/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file panda3d.h
 * @author drose
 * @date 2009-06-30
 */

#ifndef PANDA3D_H
#define PANDA3D_H

#include "dtoolbase.h"
#ifdef _WIN32
#include <winsock2.h>
#endif

#include "panda3dBase.h"
#include "p3d_plugin.h"
#include "httpChannel.h"
#include "ramfile.h"
#include "fileSpec.h"
#include "pset.h"
#include "vector_string.h"

/**
 * A standalone program that invokes the Panda3D plugin to launch .p3d files.
 */
class Panda3D : public Panda3DBase {
public:
  Panda3D(bool console_environment);

  int run_command_line(int argc, char *argv[]);

protected:
  bool post_arg_processing();
  bool get_plugin();
  bool download_contents_file(const Filename &contents_filename);
  bool read_contents_file(const Filename &contents_filename, bool fresh_download);
  void find_host(TiXmlElement *xcontents);
  void read_xhost(TiXmlElement *xhost);
  void add_mirror(std::string mirror_url);
  void choose_random_mirrors(vector_string &result, int num_mirrors);
  bool get_core_api();
  bool download_core_api();

  void usage();

protected:
  std::string _super_mirror_url;
  std::string _host_url_prefix;
  std::string _download_url_prefix;
  std::string _super_mirror_url_prefix;
  typedef pvector<std::string> Mirrors;
  Mirrors _mirrors;

  std::string _coreapi_set_ver;
  FileSpec _coreapi_dll;
};

#include "panda3d.I"

#endif
