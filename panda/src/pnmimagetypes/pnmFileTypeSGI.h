// Filename: pnmFileTypeSGI.h
// Created by:  drose (17Jun00)
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

#ifndef PNMFILETYPESGI_H
#define PNMFILETYPESGI_H

#include "pandabase.h"

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypeSGI
// Description : For reading and writing SGI RGB files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypeSGI : public PNMFileType {
public:
  PNMFileTypeSGI();

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

    virtual bool supports_read_row() const;
    virtual bool read_row(xel *array, xelval *alpha);

    typedef struct {
      long start;     /* offset in file */
      long length;    /* length of compressed scanline */
    } TabEntry;

  private:
    TabEntry *table;
    long table_start;
    int current_row;
    int bpc;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, ostream *file, bool owns_file);
    virtual ~Writer();

    virtual bool supports_write_row() const;
    virtual bool write_header();
    virtual bool write_row(xel *array, xelval *alpha);

    typedef struct {
      long start;     /* offset in file */
      long length;    /* length of compressed scanline */
    } TabEntry;

    typedef short ScanElem;
    typedef struct {
      ScanElem *  data;
      long        length;
    } ScanLine;

  private:
    TabEntry &Table(int chan) {
      return table[chan * _y_size + current_row];
    }

    void write_rgb_header(const char *imagename);
    void write_table();
    void write_channels(ScanLine channel[], void (*put)(ostream *, short));
    void build_scanline(ScanLine output[], xel *row_data, xelval *alpha_data);
    ScanElem *compress(ScanElem *temp, ScanLine &output);
    int rle_compress(ScanElem *inbuf, int size);

    TabEntry *table;
    long table_start;
    int current_row;
    int bpc;
    int dimensions;
    int new_maxval;

    ScanElem *rletemp;
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeSGI(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeSGI",
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


