/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeBMP.h
 * @author drose
 * @date 2000-06-17
 */

#ifndef PNMFILETYPEBMP_H
#define PNMFILETYPEBMP_H

#include "pandabase.h"

#ifdef HAVE_BMP

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

/**
 * For reading and writing Windows BMP files.
 */
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeBMP : public PNMFileType {
public:
  PNMFileTypeBMP();

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

    virtual int read_data(xel *array, xelval *alpha);

  private:
    unsigned long       pos;

    unsigned long offBits;

    unsigned short  cBitCount;
    int             indexed;
    int             classv;

    pixval R[256];      /* reds */
    pixval G[256];      /* greens */
    pixval B[256];      /* blues */
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, std::ostream *file, bool owns_file);

    virtual int write_data(xel *array, xelval *alpha);
    virtual bool supports_grayscale() const;
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeBMP(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeBMP",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_BMP

#endif
