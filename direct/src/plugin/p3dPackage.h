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

////////////////////////////////////////////////////////////////////
//       Class : P3DPackage
// Description : This corresponds to a downloadable, patchable
//               package, and all its constituent files.  For
//               instance, a particular version of the Panda3D
//               runtime, which consists of a bunch of dll's
//               downloaded in a single tar file, is a package.
//
//               The plugin is responsible for managing these packages
//               on disk, downloading new versions when needed, and
//               removing stale versions to limit disk space waste.
////////////////////////////////////////////////////////////////////
class P3DPackage {
public:
  P3DPackage(const string &package_name, const string &package_version,
             const string &package_platform,
             const string &package_display_name);
  ~P3DPackage();

  class Callback {
  public:
    virtual ~Callback();
    virtual void install_progress(P3DPackage *package, double progress);
    virtual void package_ready(P3DPackage *package, bool success);
  };

  inline bool get_ready() const;
  inline bool get_failed() const;
  inline const string &get_package_dir() const;
  inline const string &get_package_name() const;
  inline const string &get_package_version() const;
  inline const string &get_package_display_name() const;

  void set_callback(Callback *callback);
  void cancel_callback(Callback *callback);

private:
  enum DownloadType {
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

  void download_desc_file();
  void desc_file_download_finished(bool success);
  void got_desc_file(TiXmlDocument *doc, bool freshly_downloaded);

  void download_compressed_archive(bool allow_partial);
  void compressed_archive_download_progress(double progress);
  void compressed_archive_download_finished(bool success);

  void uncompress_archive();
  void extract_archive();

  void report_progress(double progress);
  void report_done(bool success);
  void start_download(DownloadType dtype, const string &url, 
                      const string &pathname, bool allow_partial);

  static inline int decode_hexdigit(char c);
  static inline char encode_hexdigit(int c);

  static bool decode_hex(unsigned char *dest, const char *source, size_t size);
  static void encode_hex(char *dest, const unsigned char *source, size_t size);
  static void stream_hex(ostream &out, const unsigned char *source, size_t size);

private:
  string _package_name;
  string _package_version;
  string _package_platform;
  string _package_display_name;
  string _package_fullname;
  string _package_dir;

  string _desc_file_basename;
  string _desc_file_pathname;

  bool _ready;
  bool _failed;
  Download *_active_download;
  bool _partial_download;

  typedef vector<Callback *> Callbacks;
  Callbacks _callbacks;

  enum { hash_size = 16 };

  class FileSpec {
  public:
    FileSpec();
    void load_xml(TiXmlElement *element);

    bool quick_verify(const string &package_dir) const;
    bool full_verify(const string &package_dir) const;

    bool check_hash(const string &pathname) const;

    string _filename;
    size_t _size;
    time_t _timestamp;
    unsigned char _hash[hash_size];
    bool _got_hash;
  };

  FileSpec _compressed_archive;
  FileSpec _uncompressed_archive;

  typedef vector<FileSpec> Components;
  Components _components;

  friend class Download;
  friend class P3DMultifileReader;
};

#include "p3dPackage.I"

#endif
