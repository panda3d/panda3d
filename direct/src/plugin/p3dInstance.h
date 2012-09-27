// Filename: p3dInstance.h
// Created by:  drose (29May09)
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

#ifndef P3DINSTANCE_H
#define P3DINSTANCE_H

#include "p3d_plugin_common.h"
#include "p3dFileDownload.h"
#include "p3dFileParams.h"
#include "p3dWindowParams.h"
#include "p3dReferenceCount.h"
#include "p3dSplashWindow.h"
#include "p3dTemporaryFile.h"
#include "p3dMultifileReader.h"
#include "get_tinyxml.h"

#ifdef __APPLE__
#include "subprocessWindowBuffer.h"
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <deque>
#include <map>

class P3DSession;
class P3DSplashWindow;
class P3DDownload;
class P3DPackage;
class P3DObject;
class P3DMainObject;
class P3DTemporaryFile;

////////////////////////////////////////////////////////////////////
//       Class : P3DInstance
// Description : This is an instance of a Panda3D window, as seen in
//               the parent-level process.
////////////////////////////////////////////////////////////////////
class P3DInstance : public P3D_instance, public P3DReferenceCount {
public:
  P3DInstance(P3D_request_ready_func *func, 
              const P3D_token tokens[], size_t num_tokens, 
              int argc, const char *argv[], void *user_data);
  ~P3DInstance();
  void cleanup();

  void set_p3d_url(const string &p3d_url);
  void set_p3d_filename(const string &p3d_filename, const int &p3d_offset = 0);
  int make_p3d_stream(const string &p3d_url);
  inline const P3DFileParams &get_fparams() const;

  void set_wparams(const P3DWindowParams &wparams);
  inline const P3DWindowParams &get_wparams() const;

  P3D_object *get_panda_script_object() const;
  void set_browser_script_object(P3D_object *object);

  bool has_request();
  P3D_request *get_request();
  void bake_requests();
  void add_raw_request(TiXmlDocument *doc);
  void add_baked_request(P3D_request *request);
  static void finish_request(P3D_request *request, bool handled);

  bool feed_url_stream(int unique_id,
                       P3D_result_code result_code,
                       int http_status_code, 
                       size_t total_expected_data,
                       const unsigned char *this_data, 
                       size_t this_data_size);

  bool handle_event(const P3D_event_data &event);

  inline int get_instance_id() const;
  inline const string &get_session_key() const;
  const string &get_log_pathname() const;
  inline const string &get_session_platform() const;

  inline P3DSession *get_session() const;

  inline P3D_request_ready_func *get_request_ready_func() const;

  void add_package(const string &name, const string &version, 
                   const string &seq, P3DHost *host);
  void add_package(P3DPackage *package);
  void remove_package(P3DPackage *package);
  bool get_packages_info_ready() const;
  bool get_packages_ready() const;
  bool get_packages_failed() const;
  
  inline bool is_trusted() const;
  inline bool get_matches_script_origin() const;
  int start_download(P3DDownload *download, bool add_request = true);
  inline bool is_started() const;
  inline bool is_failed() const;
  void request_stop_sub_thread();
  void request_stop_main_thread();
  void request_refresh();
  void request_callback(P3D_callback_func *func, void *data);

  TiXmlElement *make_xml();
  void splash_button_clicked_sub_thread();
  void splash_button_clicked_main_thread();
  void auth_button_clicked();
  void play_button_clicked();

  void auth_finished_sub_thread();
  void auth_finished_main_thread();

  bool uninstall_packages();
  bool uninstall_host();

private:
  class ImageDownload : public P3DFileDownload {
  public:
    ImageDownload(P3DInstance *inst, int index);
  protected:
    virtual void download_finished(bool success);
  private:
    P3DInstance *_inst;
    int _index;
  };
  class InstanceDownload : public P3DFileDownload {
  public:
    InstanceDownload(P3DInstance *inst);
  protected:
    virtual void download_progress();
    virtual void download_finished(bool success);
  private:
    P3DInstance *_inst;
  };

  // The different kinds of image files we download for the splash
  // window.
  enum ImageType {
    // Also update _image_type_names when you update this list.
    IT_download,
    IT_unauth,
    IT_ready,
    IT_failed,
    IT_launch,
    IT_active,
    IT_auth_ready,
    IT_auth_rollover,
    IT_auth_click,
    IT_play_ready,
    IT_play_rollover,
    IT_play_click,
    IT_none,                // Must be the last value
    IT_num_image_types,     // Not a real value
  };

  void priv_set_p3d_filename(const string &p3d_filename, const int &p3d_offset = -1);
  void determine_p3d_basename(const string &p3d_url);

  bool check_matches_origin(const string &origin_match);
  bool check_matches_origin_one(const string &origin_match);
  bool check_matches_hostname(const string &orig, const string &match);
  void separate_components(vector<string> &components, const string &str);
  bool check_matches_component(const string &orig, const string &match);

  void check_p3d_signature();
  void mark_p3d_untrusted();
  void mark_p3d_trusted();
  void scan_app_desc_file(TiXmlDocument *doc);
  void add_panda3d_package();
  void add_packages();
  string find_alt_host_url(const string &host_url, const string &alt_host);
  void get_host_info(P3DHost *host);
  string get_start_dir_suffix() const;

  void send_browser_script_object();
  P3D_request *make_p3d_request(TiXmlElement *xrequest);
  void handle_notify_request(const string &message);
  void handle_script_request(const string &operation, P3D_object *object, 
                             const string &property_name, P3D_object *value,
                             bool needs_response, int unique_id);

  void set_failed();
  void make_splash_window();
  void set_background_image(ImageType image_type);
  void set_button_image(ImageType image_type);
  void report_package_info_ready(P3DPackage *package);
  void consider_start_download();
  void ready_to_install();
  void start_next_download();
  void mark_download_complete();
  void ready_to_start();
  void report_instance_progress(double progress, bool is_progress_known,
                                size_t received_data);
  void report_package_progress(P3DPackage *package, double progress);
  void report_package_done(P3DPackage *package, bool success);
  void set_install_label(const string &install_label);

  void paint_window();

#ifdef __APPLE__
  bool get_framebuffer_osx_port();
  bool get_framebuffer_osx_cgcontext();
  void paint_window_osx_port();
  void paint_window_osx_cgcontext(CGContextRef context);
#endif  //  __APPLE__

  bool handle_event_osx_event_record(const P3D_event_data &event);
  bool handle_event_osx_cocoa(const P3D_event_data &event);
  void add_carbon_modifier_flags(unsigned int &swb_flags, int modifiers);
  void add_cocoa_modifier_flags(unsigned int &swb_flags, int modifiers);

  void send_notify(const string &message);

#ifdef __APPLE__
  void alloc_swbuffer();
  void free_swbuffer();
  static void timer_callback(CFRunLoopTimerRef timer, void *info);
#endif  // __APPLE__

  P3D_request_ready_func *_func;
  P3D_object *_dom_object;
  P3DMainObject *_main_object;
  string _p3d_basename;
  string _origin_protocol;
  string _origin_hostname;
  string _origin_port;

  // We need a list of previous time reports so we can average the
  // predicted download time over the past few seconds.
  class TimeReport {
  public:
    double _total;
    double _report_time;
  };
  typedef deque<TimeReport> TimeReports;
  TimeReports _time_reports;
  double _total_time_reports;

  P3DTemporaryFile *_temp_p3d_filename;

  // For downloading the various images used by the splash window.
  P3DPackage *_image_package;
  static const char *_image_type_names[IT_num_image_types];

  class ImageFile {
  public:
    inline ImageFile();
    inline ~ImageFile();
    inline void cleanup();

    bool _use_standard_image;
    P3DTemporaryFile *_temp_filename;
    string _filename;
    P3DSplashWindow::ImagePlacement _image_placement;
  };
  ImageFile _image_files[IT_num_image_types];
  ImageType _current_background_image;
  ImageType _current_button_image;

  bool _got_fparams;
  P3DFileParams _fparams;
  P3DMultifileReader _mf_reader;

  bool _got_wparams;
  P3DWindowParams _wparams;

  bool _p3d_trusted;
  TiXmlElement *_xpackage;

  // Holds the list of certificates that are pre-approved by the
  // plugin vendor.
  P3DPackage *_certlist_package;
  
  // For downloading the p3dcert authorization program.
  P3DPackage *_p3dcert_package;

  int _instance_id;
  string _session_key;
  string _log_basename;
  string _session_platform;
  string _prc_name;
  string _start_dir;
  bool _hidden;
  bool _matches_run_origin;
  bool _matches_script_origin;
  bool _allow_python_dev;
  bool _keep_user_env;
  bool _auto_install;
  bool _auto_start;
  bool _stop_on_ready;
  bool _auth_button_clicked;
  bool _failed;

  P3DSession *_session;
  P3DAuthSession *_auth_session;
  string _log_pathname;

#ifdef __APPLE__
  // On OSX, we have to get a copy of the framebuffer data back from
  // the child process, and draw it to the window, here in the parent
  // process.  Crazy!
  int _shared_fd;
  size_t _shared_mmap_size;
  string _shared_filename;
  SubprocessWindowBuffer *_swbuffer;
  char *_reversed_buffer;
  CFDataRef _buffer_data;
  CGDataProviderRef _data_provider;
  CGColorSpaceRef _buffer_color_space;
  CGImageRef _buffer_image;

  bool _mouse_active;
  unsigned int _modifiers;

  CFRunLoopTimerRef _frame_timer;
#endif  // __APPLE__

  P3DSplashWindow *_splash_window;
  string _install_label;
  bool _instance_window_opened;
  bool _instance_window_attached;
  bool _stuff_to_download;

  // Keep track of when the download was started, for reporting
  // purposes.  These members are used both for the instance download,
  // and for the later package download.
#ifdef _WIN32
  int _start_dl_tick;
#else
  struct timeval _start_dl_timeval;
#endif

  // This is set false initially, but true if the instance download
  // continues for more than a couple of seconds.
  bool _show_dl_instance_progress;

  typedef vector<P3DPackage *> Packages;
  Packages _packages;
  Packages _downloading_packages;
  int _download_package_index;
  size_t _prev_downloaded;
  size_t _total_download_size;
  size_t _total_downloaded;
  bool _packages_specified;
  bool _download_started;
  bool _download_complete;
  bool _instance_started;

  // We keep the _panda3d pointer separately because it's so
  // important, but it's in the above vector also.
  P3DPackage *_panda3d_package;

  typedef map<int, P3DDownload *> Downloads;
  Downloads _downloads;

  // The _raw_requests queue might be filled up by the read thread, so
  // we protect it in a lock.
  LOCK _request_lock;
  typedef deque<TiXmlDocument *> RawRequests;
  RawRequests _raw_requests;
  bool _requested_stop;

  // The _baked_requests queue is only touched in the main thread; no
  // lock needed.
  typedef deque<P3D_request *> BakedRequests;
  BakedRequests _baked_requests;

  friend class P3DSession;
  friend class P3DAuthSession;
  friend class ImageDownload;
  friend class InstanceDownload;
  friend class P3DWindowParams;
  friend class P3DPackage;
};

#include "p3dInstance.I"

#endif
