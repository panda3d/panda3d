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
    ES_ok = 2,
    ES_success = 1,
    ES_error = -1,
    ES_error_write = -2,
  };

  Extractor(void);
  Extractor(PT(Buffer) buffer);
  virtual ~Extractor(void);

  int initiate(Filename &source_file, const Filename &rel_path = "");
  int run(void);

  bool extract(Filename &source_file, const Filename &rel_path = "");

private:
  void init(PT(Buffer) buffer);

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

#endif
