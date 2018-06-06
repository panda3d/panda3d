/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dHost.h
 * @author drose
 * @date 2009-08-21
 */

#ifndef P3DHOST_H
#define P3DHOST_H

#include "p3d_plugin_common.h"
#include "fileSpec.h"
#include <map>

class FileSpec;
class P3DInstanceManager;
class P3DPackage;

/**
 * Represents a particular download host serving up Panda3D packages.
 */
class P3DHost {
private:
  P3DHost(const std::string &host_url, const std::string &host_dir = "");
  ~P3DHost();

public:
  inline bool has_host_dir() const;
  inline const std::string &get_host_dir() const;
  inline const std::string &get_host_url() const;
  inline const std::string &get_host_url_prefix() const;
  inline const std::string &get_download_url_prefix() const;
  inline const std::string &get_descriptive_name() const;

  P3DHost *get_alt_host(const std::string &alt_host);

  inline bool has_contents_file() const;
  bool has_current_contents_file(P3DInstanceManager *inst_mgr) const;
  inline int get_contents_iseq() const;
  inline bool check_contents_hash(const std::string &pathname) const;

  bool read_contents_file();
  bool read_contents_file(const std::string &contents_filename, bool fresh_download);
  void read_xhost(TiXmlElement *xhost);

  P3DPackage *get_package(const std::string &package_name,
                          const std::string &package_version,
                          const std::string &package_platform,
                          const std::string &package_seq,
                          const std::string &alt_host = "");
  bool choose_suitable_platform(std::string &selected_platform,
                                bool &per_platform,
                                const std::string &package_name,
                                const std::string &package_version,
                                const std::string &package_platform);
  bool get_package_desc_file(FileSpec &desc_file,
                             std::string &package_seq,
                             bool &package_solo,
                             const std::string &package_name,
                             const std::string &package_version,
                             const std::string &package_platform);

  void forget_package(P3DPackage *package, const std::string &alt_host = "");
  void migrate_package_host(P3DPackage *package, const std::string &alt_host, P3DHost *new_host);

  void choose_random_mirrors(std::vector<std::string> &result, int num_mirrors);
  void add_mirror(std::string mirror_url);

  void uninstall();

private:
  void determine_host_dir(const std::string &host_dir_basename);

  static std::string standardize_filename(const std::string &filename);
  static bool copy_file(const std::string &from_filename, const std::string &to_filename);
  static bool save_xml_file(TiXmlDocument *doc, const std::string &to_filename);
  static int compare_seq(const std::string &seq_a, const std::string &seq_b);
  static int compare_seq_int(const char *&num_a, const char *&num_b);

private:
  std::string _host_dir;
  std::string _host_url;
  std::string _host_url_prefix;
  std::string _download_url_prefix;
  std::string _descriptive_name;
  TiXmlElement *_xcontents;
  time_t _contents_expiration;
  int _contents_iseq;
  FileSpec _contents_spec;

  typedef std::vector<std::string> Mirrors;
  Mirrors _mirrors;

  typedef std::map<std::string, std::string> AltHosts;
  AltHosts _alt_hosts;

  typedef std::vector<P3DPackage *> PlatformPackages;
  typedef std::map<std::string, PlatformPackages> PackageMap;
  typedef std::map<std::string, PackageMap> Packages;
  Packages _packages;
  typedef std::vector<P3DPackage *> FailedPackages;
  FailedPackages _failed_packages;

  friend class P3DInstanceManager;
};

#include "p3dHost.I"

#endif
