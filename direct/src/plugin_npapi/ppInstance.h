// Filename: ppInstance.h
// Created by:  drose (19Jun09)
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

#ifndef PPINSTANCE_H
#define PPINSTANCE_H

#include "nppanda3d_common.h"
#include "fileSpec.h"
#include "get_tinyxml.h"
#include "p3d_lock.h"

#include <vector>

class PPToplevelObject;
class PPDownloadRequest;

////////////////////////////////////////////////////////////////////
//       Class : PPInstance
// Description : This represents a single instance of the Panda3D
//               plugin, via the NPAPI interface.  This instance
//               brokers the communication with the P3D Core API, as
//               defined in the plugin directory.
////////////////////////////////////////////////////////////////////
class PPInstance {
public:
  PPInstance(NPMIMEType pluginType, NPP instance, uint16_t mode, 
             int16_t argc, char *argn[], char *argv[], NPSavedData *saved,
             P3D_window_handle_type window_handle_type,
             P3D_event_type event_type);
  ~PPInstance();

  void begin();

  inline NPP get_npp_instance() const;

  inline const NPWindow *get_window() const;
  void set_window(NPWindow *window);
  NPError new_stream(NPMIMEType type, NPStream *stream, 
                     bool seekable, uint16_t *stype);
  void stop_outstanding_streams();

  int32_t write_ready(NPStream *stream);
  int write_stream(NPStream *stream, int offset, int len, void *buffer);
  NPError destroy_stream(NPStream *stream, NPReason reason);
  void url_notify(const char *url, NPReason reason, void *notifyData);
  void stream_as_file(NPStream *stream, const char *fname);

  bool handle_request(P3D_request *request);
  static void generic_browser_call();

  bool handle_event(void *event);

  NPObject *get_panda_script_object();

  void p3dobj_to_variant(NPVariant *result, P3D_object *object);
  P3D_object *variant_to_p3dobj(const NPVariant *variant);

  static void output_np_variant(ostream &out, const NPVariant &result);

private:
  void find_host(TiXmlElement *xcontents);
  void read_xhost(TiXmlElement *xhost);
  void add_mirror(string mirror_url);
  void choose_random_mirrors(vector<string> &result, int num_mirrors);

  static void request_ready(P3D_instance *instance);

  void start_download(const string &url, PPDownloadRequest *req);
  void downloaded_file(PPDownloadRequest *req, const string &filename);
  static string get_filename_from_url(const string &url);
  void feed_file(PPDownloadRequest *req, const string &filename);

  void open_p3d_temp_file();
  void send_p3d_temp_file_data();

  bool read_contents_file(const string &contents_filename, bool fresh_download);
  void get_core_api();
  void downloaded_plugin(const string &filename);
  void do_load_plugin();

  void create_instance();
  void send_window();
  void cleanup_window();
  bool copy_file(const string &from_filename, const string &to_filename);

  string lookup_token(const string &keyword) const;
  static int compare_seq(const string &seq_a, const string &seq_b);
  static int compare_seq_int(const char *&num_a, const char *&num_b);

  void set_failed();

  static void handle_request_loop();
  static void browser_sync_callback(void *);

#ifdef _WIN32
  static LONG 
  window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif  // _WIN32

  class EventAuxData {
  public:
    wstring _characters;
    wstring _characters_im;
    wstring _text;
  };
#ifdef MACOSX_HAS_EVENT_MODELS
  static void copy_cocoa_event(P3DCocoaEvent *p3d_event, 
                               NPCocoaEvent *np_event,
                               EventAuxData &aux_data);
  static const wchar_t *make_ansi_string(wstring &result, NPNSString *ns_string);
#endif  // MACOSX_HAS_EVENT_MODELS

#ifdef __APPLE__
  static void timer_callback(CFRunLoopTimerRef timer, void *info);
#endif  // __APPLE__

private:
  NPP _npp_instance;
  unsigned int _npp_mode;
  P3D_window_handle_type _window_handle_type;
  P3D_event_type _event_type;
  typedef vector<P3D_token> Tokens;
  Tokens _tokens;

  string _root_dir;
  string _download_url_prefix;
  typedef vector<string> Mirrors;
  Mirrors _mirrors;

  // A list of URL's that we will attempt to download the core API
  // from.
  typedef vector<string> CoreUrls;
  CoreUrls _core_urls;

  string _coreapi_set_ver;
  FileSpec _coreapi_dll;
  time_t _contents_expiration;
  bool _failed;
  bool _started;

  bool _got_instance_url;
  string _instance_url;
  bool _opened_p3d_temp_file;
  bool _finished_p3d_temp_file;
  size_t _p3d_temp_file_current_size;
  size_t _p3d_temp_file_total_size;
  ofstream _p3d_temp_file;
  string _p3d_temp_filename;
  int _p3d_instance_id;
 
  // We need to keep a list of the NPStream objects that the instance
  // owns, because Safari (at least) won't automatically delete all of
  // the outstanding streams when the instance is destroyed.
  typedef vector<NPStream *> Streams;
  Streams _streams;

  // This class is used for feeding local files (accessed via a
  // "file://" url) into the core API.
  class StreamingFileData {
  public:
    StreamingFileData(PPDownloadRequest *req, const string &filename,
                      P3D_instance *p3d_inst);
    ~StreamingFileData();

    bool is_done() const;

  private:
    void thread_run();
    THREAD_CALLBACK_DECLARATION(PPInstance::StreamingFileData, thread_run);

  private:
    bool _thread_done;
    bool _thread_continue;

    P3D_instance *_p3d_inst;
    int _user_id;
    string _filename;
    ifstream _file;
    size_t _file_size;
    size_t _total_count;

    THREAD _thread;
  };

  typedef vector<StreamingFileData *> FileDatas;
  static FileDatas _file_datas;
  
  bool _got_window;
  NPWindow _window;
#ifdef _WIN32
  LONG_PTR _orig_window_proc;
#endif  // _WIN32

#ifdef __APPLE__
  CFRunLoopRef _run_loop_main;
  CFRunLoopTimerRef _request_timer;
  LOCK _timer_lock;
#endif  // __APPLE__

  bool _python_window_open;

  PPToplevelObject *_script_object;

  P3D_instance *_p3d_inst;
};

#include "ppInstance.I"

#endif
