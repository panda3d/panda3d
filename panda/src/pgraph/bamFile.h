// Filename: bamFile.h
// Created by:  drose (02Jul00)
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

#ifndef BAMFILE_H
#define BAMFILE_H

#include "pandabase.h"
#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "bamEnums.h"

class BamReader;
class BamWriter;
class TypedWritable;
class Filename;

////////////////////////////////////////////////////////////////////
//       Class : BamFile
// Description : The principle public interface to reading and writing
//               Bam disk files.  See also BamReader and BamWriter,
//               the more general implementation of this class.
//
//               Bam files are most often used to store scene graphs
//               or subgraphs, and by convention they are given
//               filenames ending in the extension ".bam" when they
//               are used for this purpose.  However, a Bam file may
//               store any arbitrary list of TypedWritable objects;
//               in this more general usage, they are given filenames
//               ending in ".boo" to differentiate them from the more
//               common scene graph files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH BamFile : public BamEnums {
PUBLISHED:
  BamFile();
  ~BamFile();

  bool open_read(const Filename &bam_filename, bool report_errors = true);
  bool open_read(istream &in, const string &bam_filename = "stream",
                 bool report_errors = true);

  TypedWritable *read_object();

  bool is_eof() const;
  bool resolve();

  PT(PandaNode) read_node(bool report_errors = true);

  bool open_write(const Filename &bam_filename, bool report_errors = true);
  bool open_write(ostream &out, const string &bam_filename = "stream",
                  bool report_errors = true);
  bool write_object(const TypedWritable *object);

  void close();
  INLINE bool is_valid_read() const;
  INLINE bool is_valid_write() const;

  int get_file_major_ver();
  int get_file_minor_ver();
  BamEndian get_file_endian() const;
  bool get_file_stdfloat_double() const;

  int get_current_major_ver();
  int get_current_minor_ver();

  BamReader *get_reader();
  BamWriter *get_writer();

private:
  bool continue_open_read(const string &bam_filename, bool report_errors);
  bool continue_open_write(const string &bam_filename, bool report_errors);

  string _bam_filename;
  DatagramInputFile _din;
  DatagramOutputFile _dout;
  BamReader *_reader;
  BamWriter *_writer;
};

#include "bamFile.I"

#endif
