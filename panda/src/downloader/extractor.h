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
#include <tokenBoard.h>
#include <buffer.h>
#include <multifile.h>
#include "asyncUtility.h"

class ExtractorToken;

////////////////////////////////////////////////////////////////////
//       Class : Extractor 
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Extractor : public AsyncUtility {
public:
  Extractor(void);
  Extractor(PT(Buffer) buffer);
  ~Extractor(void);

  int request_extract(const Filename &source_file,
			 const string &event_name);

  bool extract(Filename &source_file);

private:
  void init(PT(Buffer) buffer);
  virtual bool process_request(void);

  typedef TokenBoard<ExtractorToken> ExtractorTokenBoard;
  ExtractorTokenBoard *_token_board;

  PT(Buffer) _buffer;
};

#endif
