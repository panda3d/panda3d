// Filename: bamFile.h
// Created by:  drose (02Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BAMFILE_H
#define BAMFILE_H

#include <pandabase.h>

#include <datagramInputFile.h>
#include <datagramOutputFile.h>

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
public:
  BamFile();
  ~BamFile();

  bool open_read(const Filename &filename, bool report_errors = true);
  TypedWritable *read_object();
  bool is_eof() const;
  bool resolve();

  bool open_write(const Filename &filename, bool report_errors = true);
  bool write_object(const TypedWritable *object);

  void close();
  INLINE bool is_valid_read() const;
  INLINE bool is_valid_write() const;

  int get_file_major_ver();
  int get_file_minor_ver();

  int get_current_major_ver();
  int get_current_minor_ver();

private:
  DatagramInputFile _din;
  DatagramOutputFile _dout;
  BamReader *_reader;
  BamWriter *_writer;
};

#include "bamFile.I"

#endif
