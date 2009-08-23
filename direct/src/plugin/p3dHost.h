// Filename: p3dHost.h
// Created by:  drose (21Aug09)
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

#ifndef P3DHOST_H
#define P3DHOST_H

#include "p3d_plugin_common.h"

#include <map>

class FileSpec;
class P3DInstanceManager;
class P3DPackage;

////////////////////////////////////////////////////////////////////
//       Class : P3DHost
// Description : Represents a particular download host serving up
//               Panda3D packages.
////////////////////////////////////////////////////////////////////
class P3DHost {
private:
  P3DHost(const string &host_url);
  ~P3DHost();

public:
  inline const string &get_host_dir() const;
  inline const string &get_host_url() const;
  inline const string &get_host_url_prefix() const;
  inline const string &get_descriptive_name() const;

  inline bool has_contents_file() const;
  bool read_contents_file();
  bool read_contents_file(const string &contents_filename);

  P3DPackage *get_package(const string &package_name, 
                          const string &package_version);
  bool get_package_desc_file(FileSpec &desc_file, 
                             string &package_platform,
                             const string &package_name,
                             const string &package_version);

private:
  void determine_host_dir();

  static string standardize_filename(const string &filename);
  static bool copy_file(const string &from_filename, const string &to_filename);
  static inline char encode_hexdigit(int c);

private:
  string _host_dir;
  string _host_url;
  string _host_url_prefix;
  string _descriptive_name;
  TiXmlElement *_xcontents;

  typedef map<string, P3DPackage *> Packages;
  Packages _packages;

  friend class P3DInstanceManager;
};

#include "p3dHost.I"

#endif
