// Filename: httpBackup.h
// Created by:  drose (29Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef HTTPBACKUP_H
#define HTTPBACKUP_H

#include "pandaappbase.h"
#include "programBase.h"
#include "backupCatalog.h"

#include "httpClient.h"
#include "urlSpec.h"

////////////////////////////////////////////////////////////////////
//       Class : HTTPBackup
// Description : This program is designed to run periodically as a
//               background task, e.g. via a cron job.  It fetches the
//               latest copy of a document from an HTTP server and
//               stores it, along with an optional number of previous
//               versions, in a local directory so that it may be
//               backed up to tape.
//
//               If the copy on disk is already the same as the latest
//               copy available on the HTTP server, this program does
//               nothing.
////////////////////////////////////////////////////////////////////
class HTTPBackup : public ProgramBase {
public:
  HTTPBackup();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  static bool dispatch_url(const string &opt, const string &arg, void *var);

public:
  void run();

private:
  bool fetch_latest();
  bool cleanup_old();
  void check_unique(string &filename);

private:
  URLSpec _proxy;
  bool _got_proxy;

  URLSpec _url;
  Filename _dirname;
  Filename _catalog_name;
  string _document_name;
  string _version_append;
  bool _always_download;

  double _max_keep_days;
  bool _got_max_keep_days;
  double _min_keep_days;
  int _max_keep_versions;
  bool _got_max_keep_versions;
  int _min_keep_versions;
  double _check_days;
  bool _got_check_days;

  HTTPDate _max_keep_date;
  HTTPDate _min_keep_date;
  HTTPDate _now;

  HTTPClient _http;
  BackupCatalog _catalog;
};

#endif
