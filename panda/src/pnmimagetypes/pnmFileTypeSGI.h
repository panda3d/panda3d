/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeSGI.h
 * @author drose
 * @date 2000-06-17
 */

#ifndef PNMFILETYPESGI_H
#define PNMFILETYPESGI_H

#include "pandabase.h"

#ifdef HAVE_SGI_RGB

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

/**
 * For reading and writing SGI RGB files.
 */
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeSGI : public PNMFileType {
public:
  PNMFileTypeSGI();

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

    virtual bool supports_read_row() const;
    virtual bool read_row(xel *array, xelval *alpha, int x_size, int y_size);

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
    Writer(PNMFileType *type, std::ostream *file, bool owns_file);
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
    void write_channels(ScanLine channel[], void (*put)(std::ostream *, short));
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

#endif  // HAVE_SGI_RGB

#endif
