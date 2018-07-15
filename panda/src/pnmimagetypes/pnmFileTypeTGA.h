/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeTGA.h
 * @author drose
 * @date 2001-04-27
 */

#ifndef PNMFILETYPETGA_H
#define PNMFILETYPETGA_H

#include "pandabase.h"

#ifdef HAVE_TGA

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"
#include "ppmcmap.h"
#include "pnmimage_base.h"

struct ImageHeader;


/**
 * For reading and writing Targa image files.
 */
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeTGA : public PNMFileType {
public:
  PNMFileTypeTGA();

  virtual std::string get_name() const;

  virtual int get_num_extensions() const;
  virtual std::string get_extension(int n) const;
  virtual std::string get_suggested_extension() const;

  virtual PNMReader *make_reader(std::istream *file, bool owns_file = true,
                                 const std::string &magic_number = std::string());
  virtual PNMWriter *make_writer(std::ostream *file, bool owns_file = true);

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, std::istream *file, bool owns_file, std::string magic_number);
    virtual ~Reader();

    virtual int read_data(xel *array, xelval *alpha);

  private:
    void readtga ( std::istream* ifp, struct ImageHeader* tgaP, const std::string &magic_number );
    void get_map_entry ( std::istream* ifp, pixel* Value, int Size,
                         gray* Alpha);
    void get_pixel ( std::istream* ifp, pixel* dest, int Size, gray* alpha_p);
    unsigned char getbyte ( std::istream* ifp );

    int rows, cols, rlencoded, mapped;
    struct ImageHeader *tga_head;
    pixel *ColorMap;
    gray *AlphaMap;
    int RLE_count, RLE_flag;

    pixval Red, Grn, Blu;
    pixval Alpha;
    unsigned int l;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, std::ostream *file, bool owns_file);
    virtual ~Writer();

    virtual int write_data(xel *array, xelval *alpha);

  private:
    void writetga ( struct ImageHeader* tgaP, char* id );
    void put_map_entry ( pixel* valueP, int size, pixval maxval );
    void compute_runlengths ( int cols, pixel* pixelrow, int* runlength );
    void put_pixel ( pixel* pP, int imgtype, pixval maxval, colorhash_table cht );
    void put_mono ( pixel* pP, pixval maxval );
    void put_map ( pixel* pP, colorhash_table cht );
    void put_rgb ( pixel* pP, pixval maxval );

    int rle_flag;
    int rows, cols;
    struct ImageHeader *tgaHeader;
    colorhist_vector chv;
    colorhash_table cht;
    int ncolors;
    int *runlength;
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeTGA(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeTGA",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_TGA

#endif
