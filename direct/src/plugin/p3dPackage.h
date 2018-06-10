/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dPackage.h
 * @author drose
 * @date 2009-06-12
 */

#ifndef P3DPACKAGE_H
#define P3DPACKAGE_H

#include "p3d_plugin_common.h"
#include "p3dFileDownload.h"
#include "p3dPatchfileReader.h"
#include "fileSpec.h"
#include "get_tinyxml.h"
#include <deque>

class P3DHost;
class P3DInstance;
class P3DTemporaryFile;

/**
 * This corresponds to a downloadable, patchable package, and all its
 * constituent files.  For instance, a particular version of the Panda3D
 * runtime, which consists of a bunch of dll's downloaded in a single tar
 * file, is a package.
 *
 * The core API is responsible for managing these packages on disk,
 * downloading new versions when needed, and removing stale versions to limit
 * disk space waste.
 */
class P3DPackage {
private:
  P3DPackage(P3DHost *host,
             const std::string &package_name,
             const std::string &package_version,
             const std::string &package_platform,
             const std::string &alt_host);
  ~P3DPackage();

public:
  inline bool get_info_ready() const;
  inline size_t get_download_size() const;

  void activate_download();
  inline bool get_ready() const;
  inline bool get_failed() const;
  inline P3DHost *get_host() const;
  inline const std::string &get_package_dir() const;
  inline const std::string &get_package_name() const;
  inline const std::string &get_package_version() const;
  inline const std::string &get_package_platform() const;
  inline const std::string &get_package_display_name() const;
  std::string get_formatted_name() const;
  inline const TiXmlElement *get_xconfig() const;

  inline const std::string &get_desc_file_pathname() const;
  inline const std::string &get_desc_file_dirname() const;
  inline std::string get_archive_file_pathname() const;

  void add_instance(P3DInstance *inst);
  void remove_instance(P3DInstance *inst);

  void mark_used();
  void uninstall();

  TiXmlElement *make_xml();

private:
  typedef std::vector<FileSpec> Extracts;

  enum DownloadType {
    DT_contents_file,
    DT_redownload_contents_file,
    DT_desc_file,
    DT_install_step,
  };

  typedef std::vector<std::string> TryUrls;

  class Download : public P3DFileDownload {
  public:
    Download(P3DPackage *package, DownloadType dtype,
             const FileSpec &file_spec);
    Download(const Download &copy);

  protected:
    virtual void download_progress();
    virtual void download_finished(bool success);

  public:
    void resume_download_finished(bool success);

  public:
    // URL's to try downloading from, in reverse order.
    TryUrls _try_urls;

  private:
    P3DPackage *_package;
    DownloadType _dtype;

    // FileSpec to validate the download against.
    FileSpec _file_spec;
  };

  enum InstallToken {
    IT_step_complete,
    IT_step_failed,
    IT_continue,
    IT_needs_callback,
    IT_terminate,
  };

  class InstallStep {
  public:
    InstallStep(P3DPackage *package, size_t bytes, double factor);
    virtual ~InstallStep();

    virtual InstallToken do_step(bool download_finished) = 0;
    virtual void output(std::ostream &out) = 0;

    inline double get_effort() const;
    inline double get_progress() const;
    inline void report_step_progress();

    P3DPackage *_package;
    size_t _bytes_needed;
    size_t _bytes_done;
    double _bytes_factor;
  };

  class InstallStepDownloadFile : public InstallStep {
  public:
    InstallStepDownloadFile(P3DPackage *package, const FileSpec &file);
    virtual ~InstallStepDownloadFile();

    virtual InstallToken do_step(bool download_finished);
    virtual void output(std::ostream &out);

    std::string _urlbase;
    std::string _pathname;
    FileSpec _file;
    Download *_download;
  };

  class InstallStepThreaded : public InstallStep {
  public:
    InstallStepThreaded(P3DPackage *package, size_t bytes, double factor);
    virtual ~InstallStepThreaded();

    virtual InstallToken do_step(bool download_finished);

    THREAD_CALLBACK_DECLARATION(InstallStepThreaded, thread_main);
    void thread_main();
    virtual InstallToken thread_step()=0;
    void thread_set_bytes_done(size_t bytes_done);
    void thread_add_bytes_done(size_t bytes_done);

    THREAD _thread;
    LOCK _thread_lock;
    bool _thread_started;
    InstallToken _thread_token;
    size_t _thread_bytes_done;
  };

  class InstallStepUncompressFile : public InstallStepThreaded {
  public:
    InstallStepUncompressFile(P3DPackage *package, const FileSpec &source,
                              const FileSpec &target, bool verify_target);
    virtual InstallToken thread_step();
    virtual void output(std::ostream &out);

    FileSpec _source;
    FileSpec _target;
    bool _verify_target;
  };

  class InstallStepUnpackArchive : public InstallStepThreaded {
  public:
    InstallStepUnpackArchive(P3DPackage *package, size_t unpack_size);
    virtual InstallToken thread_step();
    virtual void output(std::ostream &out);
  };

  class InstallStepApplyPatch : public InstallStepThreaded {
  public:
    InstallStepApplyPatch(P3DPackage *package,
                          const FileSpec &patchfile,
                          const FileSpec &source,
                          const FileSpec &target);
    virtual InstallToken thread_step();
    virtual void output(std::ostream &out);

    P3DPatchfileReader _reader;
  };

  typedef std::deque<InstallStep *> InstallPlan;
  typedef std::deque<InstallPlan> InstallPlans;
  InstallPlans _install_plans;

  bool _computed_plan_size;
  double _total_plan_size;
  double _total_plan_completed;
  double _download_progress;
  double _current_step_effort;

  void begin_info_download();
  void download_contents_file();
  void contents_file_download_finished(bool success);
  void redownload_contents_file(Download *download);
  void contents_file_redownload_finished(bool success);
  void host_got_contents_file();

  void download_desc_file();
  void desc_file_download_finished(bool success);
  void got_desc_file(TiXmlDocument *doc, bool freshly_downloaded);

  void clear_install_plans();
  void build_install_plans(TiXmlDocument *doc);
  void follow_install_plans(bool download_finished, bool plan_failed);
  static void st_callback(void *self);
  void request_callback();

  void report_progress(InstallStep *step);
  void report_info_ready();
  void report_done(bool success);
  Download *start_download(DownloadType dtype, const std::string &urlbase,
                           const std::string &pathname, const FileSpec &file_spec);
  void set_active_download(Download *download);
  void set_saved_download(Download *download);

  bool is_extractable(FileSpec &file, const std::string &filename) const;
  bool instance_terminating(P3DInstance *instance);
  void set_fullname();

public:
  class RequiredPackage {
  public:
    inline RequiredPackage(const std::string &package_name,
                           const std::string &package_version,
                           const std::string &package_seq,
                           P3DHost *host);
    std::string _package_name;
    std::string _package_version;
    std::string _package_seq;
    P3DHost *_host;
  };
  typedef std::vector<RequiredPackage> Requires;
  Requires _requires;

private:
  P3DHost *_host;
  int _host_contents_iseq;

  std::string _package_name;
  std::string _package_version;
  std::string _package_platform;
  bool _per_platform;
  int _patch_version;
  std::string _alt_host;
  bool _package_solo;
  std::string _package_display_name;
  std::string _package_fullname;
  std::string _package_dir;
  TiXmlElement *_xconfig;

  P3DTemporaryFile *_temp_contents_file;

  FileSpec _desc_file;
  std::string _desc_file_dirname;
  std::string _desc_file_basename;
  std::string _desc_file_pathname;

  bool _info_ready;
  bool _allow_data_download;
  bool _ready;
  bool _failed;
  Download *_active_download;
  Download *_saved_download;

  typedef std::vector<P3DInstance *> Instances;
  Instances _instances;

  FileSpec _compressed_archive;
  FileSpec _uncompressed_archive;

  size_t _unpack_size;
  Extracts _extracts;
  bool _updated;

  static const double _download_factor;
  static const double _uncompress_factor;
  static const double _unpack_factor;
  static const double _patch_factor;

  friend class Download;
  friend class InstallStep;
  friend class P3DMultifileReader;
  friend class P3DHost;
};

#include "p3dPackage.I"

#endif
