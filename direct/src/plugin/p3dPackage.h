// Filename: p3dPackage.h
// Created by:  drose (12Jun09)
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

#ifndef P3DPACKAGE_H
#define P3DPACKAGE_H

#include "p3d_plugin_common.h"
#include "p3dFileDownload.h"
#include "fileSpec.h"
#include "get_tinyxml.h"

class P3DHost;
class P3DInstance;
class P3DTemporaryFile;

////////////////////////////////////////////////////////////////////
//       Class : P3DPackage
// Description : This corresponds to a downloadable, patchable
//               package, and all its constituent files.  For
//               instance, a particular version of the Panda3D
//               runtime, which consists of a bunch of dll's
//               downloaded in a single tar file, is a package.
//
//               The core API is responsible for managing these
//               packages on disk, downloading new versions when
//               needed, and removing stale versions to limit disk
//               space waste.
////////////////////////////////////////////////////////////////////
class P3DPackage {
private:
  P3DPackage(P3DHost *host,
             const string &package_name, 
             const string &package_version);
  ~P3DPackage();

public:
  inline bool get_info_ready() const;
  inline size_t get_download_size() const;

  void activate_download();
  inline bool get_ready() const;
  inline bool get_failed() const;
  inline P3DHost *get_host() const;
  inline const string &get_package_dir() const;
  inline const string &get_package_name() const;
  inline const string &get_package_version() const;
  inline const string &get_package_display_name() const;
  inline const TiXmlElement *get_xconfig() const;

  inline const string &get_desc_file_pathname() const;
  inline string get_archive_file_pathname() const;

  void add_instance(P3DInstance *inst);
  void remove_instance(P3DInstance *inst);

  TiXmlElement *make_xml();

private:
  enum DownloadType {
    DT_contents_file,
    DT_desc_file,
    DT_compressed_archive
  };

  class Download : public P3DFileDownload {
  public:
    Download(P3DPackage *package, DownloadType dtype);

  protected:
    virtual void download_progress();
    virtual void download_finished(bool success);

  private:
    P3DPackage *_package;
    DownloadType _dtype;
  };

  void begin_info_download();
  void download_contents_file();
  void contents_file_download_finished(bool success);

  void download_desc_file();
  void desc_file_download_finished(bool success);
  void got_desc_file(TiXmlDocument *doc, bool freshly_downloaded);

  void begin_data_download();
  void download_compressed_archive(bool allow_partial);
  void compressed_archive_download_progress(double progress);
  void compressed_archive_download_finished(bool success);

  void uncompress_archive();
  void extract_archive();

  void report_progress(double progress);
  void report_info_ready();
  void report_done(bool success);
  void start_download(DownloadType dtype, const string &url, 
                      const string &pathname, bool allow_partial);

  bool is_extractable(const string &filename) const;

private:
  P3DHost *_host;

  string _package_name;
  string _package_version;
  string _package_platform;
  bool _package_solo;
  string _package_display_name;
  string _package_fullname;
  string _package_dir;
  TiXmlElement *_xconfig;

  P3DTemporaryFile *_temp_contents_file;

  string _desc_file_url;
  string _desc_file_basename;
  string _desc_file_pathname;

  bool _info_ready;
  size_t _download_size;
  bool _allow_data_download;
  bool _ready;
  bool _failed;
  Download *_active_download;
  bool _partial_download;

  typedef vector<P3DInstance *> Instances;
  Instances _instances;

  FileSpec _compressed_archive;
  FileSpec _uncompressed_archive;

  typedef vector<FileSpec> Extracts;
  Extracts _extracts;

  friend class Download;
  friend class P3DMultifileReader;

  friend class P3DHost;
};

#include "p3dPackage.I"

#endif
