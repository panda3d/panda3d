// Filename: p3dFileDownload.h
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

#ifndef P3DFILEDOWNLOAD_H
#define P3DFILEDOWNLOAD_H

#include "p3d_plugin_common.h"
#include "p3dDownload.h"

#include <fstream>

////////////////////////////////////////////////////////////////////
//       Class : P3DFileDownload
// Description : This is a specialization on P3DDownload that
//               specifically writes a disk file.
////////////////////////////////////////////////////////////////////
class P3DFileDownload : public P3DDownload {
public:
  P3DFileDownload();
  P3DFileDownload(const P3DFileDownload &copy);

  bool set_filename(const string &filename);
  inline const string &get_filename() const;

protected:
  virtual bool open_file();
  void close_file();
  virtual bool receive_data(const unsigned char *this_data,
                            size_t this_data_size);
  virtual void download_finished(bool success);

protected:
  ofstream _file;

private:
  string _filename;
};

#include "p3dFileDownload.I"

#endif
