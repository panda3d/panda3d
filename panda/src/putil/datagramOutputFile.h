/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramOutputFile.h
 * @author drose
 * @date 2000-10-30
 */

#ifndef DATAGRAMOUTPUTFILE_H
#define DATAGRAMOUTPUTFILE_H

#include "pandabase.h"

#include "datagramSink.h"
#include "filename.h"
#include "fileReference.h"
#include "virtualFile.h"
#include "virtualFileSystem.h"
#include "config_putil.h"

/**
 * This class can be used to write a binary file that consists of an arbitrary
 * header followed by a number of datagrams.
 */
class EXPCL_PANDA_PUTIL DatagramOutputFile : public DatagramSink {
PUBLISHED:
  INLINE DatagramOutputFile();
  INLINE ~DatagramOutputFile();

  bool open(const FileReference *file);
  INLINE bool open(const Filename &filename);
  bool open(std::ostream &out, const Filename &filename = Filename());

  void close();

  bool write_header(const std::string &header);
  virtual bool put_datagram(const Datagram &data);
  virtual bool copy_datagram(SubfileInfo &result, const Filename &filename);
  virtual bool copy_datagram(SubfileInfo &result, const SubfileInfo &source);
  virtual bool is_error();
  virtual void flush();

public:
  virtual const Filename &get_filename();
  virtual const FileReference *get_file();
  virtual std::streampos get_file_pos();

  INLINE std::ostream &get_stream();

PUBLISHED:
  MAKE_PROPERTY(stream, get_stream);

private:
  bool _wrote_first_datagram;
  bool _error;
  CPT(FileReference) _file;
  PT(VirtualFile) _vfile;
  std::ostream *_out;
  bool _owns_out;
  Filename _filename;
};

#include "datagramOutputFile.I"

#endif
