// Filename: pnmFileTypePNG.h
// Created by:  drose (16Mar04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PNMFILETYPEPNG_H
#define PNMFILETYPEPNG_H

#include "pandabase.h"

#ifdef HAVE_PNG

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

#include <png.h>

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypePNG
// Description : For reading and writing PNG files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypePNG : public PNMFileType {
public:
  PNMFileTypePNG();

  virtual string get_name() const;

  virtual int get_num_extensions() const;
  virtual string get_extension(int n) const;
  virtual string get_suggested_extension() const;

  virtual bool has_magic_number() const;
  virtual bool matches_magic_number(const string &magic_number) const;

  virtual PNMReader *make_reader(istream *file, bool owns_file = true,
                                 const string &magic_number = string());
  virtual PNMWriter *make_writer(ostream *file, bool owns_file = true);

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number);
    virtual ~Reader();

    virtual int read_data(xel *array, xelval *alpha_data);

  private:
    void free_png();
    static void png_read_data(png_structp png_ptr, png_bytep data, 
                              png_size_t length);

    static void png_error(png_structp png_ptr, png_const_charp error_msg);
    static void png_warning(png_structp png_ptr, png_const_charp warning_msg);

    png_structp _png;
    png_infop _info;

    // We need a jmp_buf to support libpng's fatal error handling, in
    // which the error handler must not immediately leave libpng code,
    // but must return to the caller in Panda.
    jmp_buf _jmpbuf;
  };

  /*
  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, ostream *file, bool owns_file);

    virtual int write_data(xel *array, xelval *alpha);

  private:
    static void png_write_data(png_structp png_ptr, png_bytep data, 
                               png_size_t length);
    static void png_flush_data(png_structp png_ptr);
  };
  */

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypePNG(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypePNG",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_PNG

#endif
