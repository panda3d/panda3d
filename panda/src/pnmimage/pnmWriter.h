// Filename: pnmWriter.h
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

#ifndef PNMWRITER_H
#define PNMWRITER_H

#include "pandabase.h"

#include "pnmImageHeader.h"

////////////////////////////////////////////////////////////////////
//       Class : PNMWriter
// Description : This is an abstract base class that defines the
//               interface for writing image files of various types.
//               Any particular image file type that can be written
//               must define a class that inherits from PNMWriter to
//               write it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMWriter : public PNMImageHeader {
protected:
  INLINE PNMWriter(PNMFileType *type, ostream *file, bool owns_file);

public:

  // It is important to delete the PNMWriter class after successfully
  // writing the data.  Failing to do this may result in some data not
  // getting flushed!
  virtual ~PNMWriter();

  INLINE PNMFileType *get_type() const;

  INLINE void set_color_type(ColorType type);
  INLINE void set_num_channels(int num_channels);
  INLINE void set_maxval(xelval maxval);
  INLINE void set_x_size(int x_size);
  INLINE void set_y_size(int y_size);

  INLINE void copy_header_from(const PNMImageHeader &header);

  virtual int write_data(xel *array, xelval *alpha);
  virtual bool supports_write_row() const;
  virtual bool write_header();
  virtual bool write_row(xel *array, xelval *alpha);

  virtual bool supports_stream_write() const;

protected:
  PNMFileType *_type;
  bool _owns_file;
  ostream *_file;
};

#include "pnmWriter.I"

#endif
