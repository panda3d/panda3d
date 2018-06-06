/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypePNG.h
 * @author drose
 * @date 2004-03-16
 */

#ifndef PNMFILETYPEPNG_H
#define PNMFILETYPEPNG_H

#include "pandabase.h"

#ifdef HAVE_PNG

// Must be first.
#include <png.h>

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

/**
 * For reading and writing PNG files.
 */
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypePNG : public PNMFileType {
public:
  PNMFileTypePNG();

  virtual std::string get_name() const;

  virtual int get_num_extensions() const;
  virtual std::string get_extension(int n) const;
  virtual std::string get_suggested_extension() const;

  virtual bool has_magic_number() const;
  virtual bool matches_magic_number(const std::string &magic_number) const;

  virtual PNMReader *make_reader(std::istream *file, bool owns_file = true,
                                 const std::string &magic_number = std::string());
  virtual PNMWriter *make_writer(std::ostream *file, bool owns_file = true);

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, std::istream *file, bool owns_file, std::string magic_number);
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

    // We need a jmp_buf to support libpng's fatal error handling, in which
    // the error handler must not immediately leave libpng code, but must
    // return to the caller in Panda.
    jmp_buf _jmpbuf;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, std::ostream *file, bool owns_file);
    virtual ~Writer();

    virtual int write_data(xel *array, xelval *alpha);

  private:
    void free_png();
    static int make_png_bit_depth(int bit_depth);
    static void png_write_data(png_structp png_ptr, png_bytep data,
                               png_size_t length);
    static void png_flush_data(png_structp png_ptr);

    static void png_error(png_structp png_ptr, png_const_charp error_msg);
    static void png_warning(png_structp png_ptr, png_const_charp warning_msg);

    png_structp _png;
    png_infop _info;

    // We need a jmp_buf to support libpng's fatal error handling, in which
    // the error handler must not immediately leave libpng code, but must
    // return to the caller in Panda.
    jmp_buf _jmpbuf;
  };

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
