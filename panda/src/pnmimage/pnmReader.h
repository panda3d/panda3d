// Filename: pnmReader.h
// Created by:  drose (14Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PNMREADER_H
#define PNMREADER_H

#include <pandabase.h>

#include "pnmImageHeader.h"


////////////////////////////////////////////////////////////////////
// 	 Class : PNMReader
// Description : This is an abstract base class that defines the
//               interface for reading image files of various types.
//               Any particular image file type that can be read must
//               define a class that inherits from PNMReader to read
//               it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMReader : public PNMImageHeader {
protected:
  INLINE PNMReader(PNMFileType *type, FILE *file, bool owns_file);

public:
  virtual ~PNMReader();

  INLINE PNMFileType *get_type() const;

  virtual int read_data(xel *array, xelval *alpha);
  virtual bool supports_read_row() const;
  virtual bool read_row(xel *array, xelval *alpha);

  virtual bool supports_stream_read() const;

  INLINE bool is_valid() const;

protected:
  PNMFileType *_type;
  bool _owns_file;
  FILE *_file;
  bool _is_valid;
};

#include "pnmReader.I"

#endif
