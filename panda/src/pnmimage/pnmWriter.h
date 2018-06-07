/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmWriter.h
 * @author drose
 * @date 2000-06-14
 */

#ifndef PNMWRITER_H
#define PNMWRITER_H

#include "pandabase.h"

#include "pnmImageHeader.h"
class PfmFile;

/**
 * This is an abstract base class that defines the interface for writing image
 * files of various types.  Any particular image file type that can be written
 * must define a class that inherits from PNMWriter to write it.
 */
class EXPCL_PANDA_PNMIMAGE PNMWriter : public PNMImageHeader {
protected:
  INLINE PNMWriter(PNMFileType *type, std::ostream *file, bool owns_file);

public:

  // It is important to delete the PNMWriter class after successfully writing
  // the data.  Failing to do this may result in some data not getting
  // flushed!
  virtual ~PNMWriter();

  INLINE PNMFileType *get_type() const;

  INLINE void set_color_type(ColorType type);
  INLINE void set_num_channels(int num_channels);
  INLINE void set_maxval(xelval maxval);
  INLINE void set_x_size(int x_size);
  INLINE void set_y_size(int y_size);

  INLINE void copy_header_from(const PNMImageHeader &header);

  virtual bool supports_floating_point();
  virtual bool supports_integer();
  virtual bool write_pfm(const PfmFile &pfm);

  virtual int write_data(xel *array, xelval *alpha);
  virtual bool supports_write_row() const;
  virtual bool supports_grayscale() const;
  virtual bool write_header();
  virtual bool write_row(xel *array, xelval *alpha);

  virtual bool supports_stream_write() const;

  INLINE bool is_valid() const;

protected:
  PNMFileType *_type;
  bool _owns_file;
  std::ostream *_file;
  bool _is_valid;
};

#include "pnmWriter.I"

#endif
