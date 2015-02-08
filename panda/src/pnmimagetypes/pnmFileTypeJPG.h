// Filename: pnmFileTypeJPG.h
// Created by:  mike (17Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PNMFILETYPEJPG_H
#define PNMFILETYPEJPG_H

#include "pandabase.h"

#ifdef HAVE_JPEG

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>  // we need to include this before jpeglib.
#endif

#ifdef HAVE_PNG
// If we are going to be including png.h (in the unrelated file
// pnmFileTypePNG.h), be sure to include it before including setjmp.h.
// Ugly hack due to png weirdness with setjmp.
#include <png.h>
#endif


extern "C" {
#include <stdio.h>  // jpeglib requires this to be included first.
#include <jpeglib.h>
#include <setjmp.h>
}

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypeJPG
// Description : For reading and writing Jpeg files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeJPG : public PNMFileType {
public:
  PNMFileTypeJPG();

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
    ~Reader();

    virtual void prepare_read();
    virtual int read_data(xel *array, xelval *alpha);

  private:
    struct jpeg_decompress_struct _cinfo;
    struct my_error_mgr {
      struct jpeg_error_mgr pub;
      jmp_buf setjmp_buffer;
    };
    typedef struct my_error_mgr *_my_error_ptr;
    struct my_error_mgr _jerr;
    unsigned long       pos;

    unsigned long offBits;

    unsigned short  cBitCount;
    int             indexed;
    int             classv;

    pixval R[256];      /* reds */
    pixval G[256];      /* greens */
    pixval B[256];      /* blues */

    bool _is_valid;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, ostream *file, bool owns_file);

    virtual int write_data(xel *array, xelval *alpha);
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeJPG(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeJPG",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_JPEG

#endif

