// Filename: pnmFileTypeJPG2000.h
// Created by:  mike (17Jun00)
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

#ifndef PNMFILETYPEJPG2000_H
#define PNMFILETYPEJPG2000_H

#include "pandabase.h"

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

#if defined(_WIN32)
#include <windows.h>  // we need to include this before jpeglib.
#endif

#include "typedef.h"  // jasper requires this first.
#include <jasper/jasper.h>

// Undo jasper-inflicted damage.
#undef bool

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypeJPG2000
// Description : For reading and writing Jpeg2000 files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypeJPG2000 : public PNMFileType {
public:
  PNMFileTypeJPG2000();

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
    ~Reader(void);

    virtual int read_data(xel *array, xelval *alpha);

  private:
/*
    // struct jpeg_decompress_struct _cinfo;
    jas_image_t *image;
    jas_stream_t *in;
    jas_stream_t *out;
*/  


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
  static TypedWritable *make_PNMFileTypeJPG2000(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeJPG2000",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif


