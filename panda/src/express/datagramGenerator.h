/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramGenerator.h
 * @author jason
 * @date 2000-06-07
 */

#ifndef DATAGRAMGENERATOR_H
#define DATAGRAMGENERATOR_H

#include "pandabase.h"

#include "datagram.h"

class SubfileInfo;
class FileReference;
class Filename;
class VirtualFile;

/**
 * This class defines the abstract interace to any source of datagrams,
 * whether it be from a file or from the net.
 */
class EXPCL_PANDA_EXPRESS DatagramGenerator {
PUBLISHED:
  INLINE DatagramGenerator();
  virtual ~DatagramGenerator();

  virtual bool get_datagram(Datagram &data) = 0;
  virtual bool save_datagram(SubfileInfo &info);
  virtual bool is_eof() = 0;
  virtual bool is_error() = 0;

  virtual const Filename &get_filename();
  virtual time_t get_timestamp() const;
  virtual const FileReference *get_file();
  virtual VirtualFile *get_vfile();
  virtual std::streampos get_file_pos();
};

#include "datagramGenerator.I"

#endif
