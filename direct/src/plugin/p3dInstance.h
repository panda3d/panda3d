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
#include "get_tinyxml.h"

#include <deque>
#include <map>

class P3DSession;
class P3DSplashWindow;
class P3DDownload;
class P3DPackage;
class P3DObject;

////////////////////////////////////////////////////////////////////
//       Class : P3DInstance
// Description : This is an instance of a Panda3D window, as seen in
//               the parent-level process.
////////////////////////////////////////////////////////////////////
class P3DInstance : public P3D_instance {
public:
  P3DInstance(P3D_request_ready_func *func, void *user_data);
  ~P3DInstance();

  void set_fparams(const P3DFileParams &fparams);
  inline const P3DFileParams &get_fparams() const;

  void set_wparams(const P3DWindowParams &wparams);
  inline const P3DWindowParams &get_wparams() const;

  P3DObject *get_panda_script_object() const;
  void set_browser_script_object(P3D_object *object);

  bool has_request();
  P3D_request *get_request();
  void add_request(P3D_request *request);
  void finish_request(P3D_request *request, bool handled);

  bool feed_url_stream(int unique_id,
                       P3D_result_code result_code,
                       int http_status_code, 
                       size_t total_expected_data,
                       const unsigned char *this_data, 
                       size_t this_data_size);

  inline int get_instance_id() const;
  inline const string &get_session_key() const;
  inline const string &get_python_version() const;

  inline P3D_request_ready_func *get_request_ready_func() const;

  void add_package(P3DPackage *package);
  
  void start_download(P3DDownload *download);
  inline bool is_started() const;
  void request_stop();

  TiXmlElement *make_xml();

private:
  class SplashDownload : public P3DFileDownload {
  public:
    SplashDownload(P3DInstance *inst);

  protected:
    virtual void download_finished(bool success);

  private:
    P3DInstance *_inst;
  };

  void send_browser_script_object();
  void handle_notify_request(P3D_request *request);
  void handle_script_request(P3D_request *request);
  void make_splash_window();
  void install_progress(P3DPackage *package, double progress);

  P3D_request_ready_func *_func;
  P3D_object *_browser_script_object;

  bool _got_fparams;
  P3DFileParams _fparams;

  bool _got_wparams;
  P3DWindowParams _wparams;

  int _instance_id;
  string _session_key;
  string _python_version;
  P3DSession *_session;
  P3DSplashWindow *_splash_window;
  bool _instance_window_opened;

  typedef vector<P3DPackage *> Packages;
  Packages _packages;

  typedef map<int, P3DDownload *> Downloads;
  Downloads _downloads;

  LOCK _request_lock;
  typedef deque<P3D_request *> Requests;
  Requests _pending_requests;
  bool _requested_stop;

  static int _next_instance_id;

  friend class P3DSession;
  friend class SplashDownload;
};

#include "p3dInstance.I"

#endif
