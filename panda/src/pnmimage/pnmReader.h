// Filename: pnmReader.h
// Created by:  drose (14Jun00)
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

#ifndef PNMREADER_H
#define PNMREADER_H

#include "pandabase.h"

#include "pnmImageHeader.h"


////////////////////////////////////////////////////////////////////
//       Class : PNMReader
// Description : This is an abstract base class that defines the
//               interface for reading image files of various types.
//               Any particular image file type that can be read must
//               define a class that inherits from PNMReader to read
//               it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMReader : public PNMImageHeader {
protected:
  INLINE PNMReader(PNMFileType *type, istream *file, bool owns_file);

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
  istream *_file;
  bool _is_valid;
};

#include "pnmReader.I"

#endif
