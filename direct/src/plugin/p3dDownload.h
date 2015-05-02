// Filename: p3dDownload.h
// Created by:  drose (11Jun09)
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

#ifndef P3DDOWNLOAD_H
#define P3DDOWNLOAD_H

#include "p3d_plugin_common.h"
#include "p3dReferenceCount.h"
class P3DInstance;

#include <time.h>

////////////////////////////////////////////////////////////////////
//       Class : P3DDownload
// Description : This represents a request to download a single file
//               from a URL, with no particular destination.  It is
//               intended to be used as an abstract base class; to use
//               it, subclass it and redefine the appropriate callback
//               methods.
////////////////////////////////////////////////////////////////////
class P3DDownload : public P3DReferenceCount {
public:
  P3DDownload();
  P3DDownload(const P3DDownload &copy);
  virtual ~P3DDownload();

  void set_url(const string &url);
  inline const string &get_url() const;

  inline void set_instance(P3DInstance *instance);
  inline P3DInstance *get_instance() const;

  inline double get_download_progress() const;
  inline bool is_download_progress_known() const;
  inline bool get_download_finished() const;
  inline bool get_download_success() const;
  inline bool get_download_terminated() const;
  inline size_t get_total_data() const;
  inline void set_total_expected_data(size_t expected_data);

  void cancel();
  void clear();

public:
  // These are intended to be called only by P3DInstance.
  inline void set_download_id(int download_id);
  inline int get_download_id() const;

  bool feed_url_stream(P3D_result_code result_code,
                       int http_status_code, 
                       size_t total_expected_data,
                       const unsigned char *this_data, 
                       size_t this_data_size);  

protected:
  virtual bool receive_data(const unsigned char *this_data,
                            size_t this_data_size);
  virtual void download_progress();
  virtual void download_finished(bool success);

protected:
  P3D_result_code _status;
  int _http_status_code;

  size_t _total_data;
  size_t _total_expected_data;
  time_t _last_reported_time;
  bool _progress_known;

private:
  bool _canceled;
  int _download_id;
  string _url;
  P3DInstance *_instance;
};

#include "p3dDownload.I"

#endif
