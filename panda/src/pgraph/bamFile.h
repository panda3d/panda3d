// Filename: bamFile.h
// Created by:  drose (02Jul00)
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

#ifndef BAMFILE_H
#define BAMFILE_H

#include "pandabase.h"

#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "pandaNode.h"
#include "pointerTo.h"

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
class EXPCL_PANDA BamFile {
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

  int get_current_major_ver();
  int get_current_minor_ver();

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
