// Filename: extractor.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef EXTRACTOR_H
#define EXTRACTOR_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <filename.h>
#include <buffer.h>
#include <multifile.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : Extractor 
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Extractor {
PUBLISHED:
  enum ExtractorStatus {
    EX_ok = 2,
    EX_success = 1,
    EX_error_abort = -1,
    EX_error_write = -2,
    EX_error_empty = -3,
  };

  Extractor(void);
  Extractor(PT(Buffer) buffer);
  virtual ~Extractor(void);

  int initiate(Filename &source_file, const Filename &rel_path = "");
  int run(void);

  bool extract(Filename &source_file, const Filename &rel_path = "");

  INLINE float get_progress(void) const;

private:
  void init(PT(Buffer) buffer);
  void cleanup(void);

  bool _initiated;
  PT(Buffer) _buffer;

  ifstream _read_stream;
  int _source_file_length;
  Multifile *_mfile;
  int _total_bytes_read;
  bool _read_all_input;
  bool _handled_all_input;
  int _source_buffer_length;
  Filename _source_file;
  Filename _rel_path;
};

#include "extractor.I"

#endif
