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
class TypedWriteable;
class Filename;

////////////////////////////////////////////////////////////////////
// 	 Class : BamFile
// Description : The principle public interface to reading and writing
//               Bam disk files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BamFile {
public:
  BamFile();
  ~BamFile();

  bool open_read(const Filename &filename, bool report_errors = true);
  TypedWriteable *read_object();
  bool resolve();

  bool open_write(const Filename &filename, bool report_errors = true);
  bool write_object(const TypedWriteable *object);

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
