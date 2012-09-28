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
#include "fileSpec.h"
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
  P3DHost(const string &host_url, const string &host_dir = "");
  ~P3DHost();

public:
  inline bool has_host_dir() const;
  inline const string &get_host_dir() const;
  inline const string &get_host_url() const;
  inline const string &get_host_url_prefix() const;
  inline const string &get_download_url_prefix() const;
  inline const string &get_descriptive_name() const;

  P3DHost *get_alt_host(const string &alt_host);

  inline bool has_contents_file() const;
  bool has_current_contents_file(P3DInstanceManager *inst_mgr) const;
  inline int get_contents_iseq() const;
  inline bool check_contents_hash(const string &pathname) const;

  bool read_contents_file();
  bool read_contents_file(const string &contents_filename, bool fresh_download);
  void read_xhost(TiXmlElement *xhost);

  P3DPackage *get_package(const string &package_name, 
                          const string &package_version,
                          const string &package_platform,
                          const string &package_seq,
                          const string &alt_host = "");
  bool choose_suitable_platform(string &selected_platform,
                                bool &per_platform,
                                const string &package_name,
                                const string &package_version,
                                const string &package_platform);
  bool get_package_desc_file(FileSpec &desc_file, 
                             string &package_seq,
                             bool &package_solo,
                             const string &package_name,
                             const string &package_version,
                             const string &package_platform);

  void forget_package(P3DPackage *package, const string &alt_host = "");
  void migrate_package_host(P3DPackage *package, const string &alt_host, P3DHost *new_host);

  void choose_random_mirrors(vector<string> &result, int num_mirrors);
  void add_mirror(string mirror_url);

  void uninstall();

private:
  void determine_host_dir(const string &host_dir_basename);

  static string standardize_filename(const string &filename);
  static bool copy_file(const string &from_filename, const string &to_filename);
  static bool save_xml_file(TiXmlDocument *doc, const string &to_filename);
  static int compare_seq(const string &seq_a, const string &seq_b);
  static int compare_seq_int(const char *&num_a, const char *&num_b);

private:
  string _host_dir;
  string _host_url;
  string _host_url_prefix;
  string _download_url_prefix;
  string _descriptive_name;
  TiXmlElement *_xcontents;
  time_t _contents_expiration;
  int _contents_iseq;
  FileSpec _contents_spec;

  typedef vector<string> Mirrors;
  Mirrors _mirrors;

  typedef map<string, string> AltHosts;
  AltHosts _alt_hosts;

  typedef vector<P3DPackage *> PlatformPackages;
  typedef map<string, PlatformPackages> PackageMap;
  typedef map<string, PackageMap> Packages;
  Packages _packages;
  typedef vector<P3DPackage *> FailedPackages;
  FailedPackages _failed_packages;

  friend class P3DInstanceManager;
};

#include "p3dHost.I"

#endif
