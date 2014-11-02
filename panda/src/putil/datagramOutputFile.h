// Filename: datagramOutputFile.h
// Created by:  drose (30Oct00)
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

#ifndef DATAGRAMOUTPUTFILE_H
#define DATAGRAMOUTPUTFILE_H

#include "pandabase.h"

#include "datagramSink.h"
#include "filename.h"
#include "fileReference.h"
#include "virtualFile.h"
#include "virtualFileSystem.h"
#include "config_util.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramOutputFile
// Description : This class can be used to write a binary file that
//               consists of an arbitrary header followed by a number
//               of datagrams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL DatagramOutputFile : public DatagramSink {
public:
  INLINE DatagramOutputFile();
  INLINE ~DatagramOutputFile();

  bool open(const FileReference *file);
  INLINE bool open(const Filename &filename);
  bool open(ostream &out, const Filename &filename = Filename());
  INLINE ostream &get_stream();

  void close();

  bool write_header(const string &header);
  virtual bool put_datagram(const Datagram &data);
  virtual bool copy_datagram(SubfileInfo &result, const Filename &filename);
  virtual bool copy_datagram(SubfileInfo &result, const SubfileInfo &source);
  virtual bool is_error();
  virtual void flush();

  virtual const Filename &get_filename();
  virtual const FileReference *get_file();
  virtual streampos get_file_pos();

private:
  bool _wrote_first_datagram;
  bool _error;
  CPT(FileReference) _file;
  PT(VirtualFile) _vfile;
  ostream *_out;
  bool _owns_out;
  Filename _filename;
};

#include "datagramOutputFile.I"

#endif
