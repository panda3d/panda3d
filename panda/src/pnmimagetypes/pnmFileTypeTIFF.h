// Filename: pnmFileTypeTIFF.h
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PNMFILETYPETIFF_H
#define PNMFILETYPETIFF_H

#include <pandabase.h>

#include <pnmFileType.h>
#include <pnmReader.h>
#include <pnmWriter.h>

#define TIFF_COLORMAP_MAXCOLORS 1024

////////////////////////////////////////////////////////////////////
// 	 Class : PNMFileTypeTIFF
// Description : For reading and writing TIFF files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypeTIFF : public PNMFileType {
public:
  PNMFileTypeTIFF();

  virtual string get_name() const;

  virtual int get_num_extensions() const;
  virtual string get_extension(int n) const;
  virtual string get_suggested_extension() const;

  virtual bool has_magic_number() const;
  virtual bool matches_magic_number(const string &magic_number) const;

  virtual PNMReader *make_reader(FILE *file, bool owns_file = true,
				 const string &magic_number = string());
  virtual PNMWriter *make_writer(FILE *file, bool owns_file = true);

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, FILE *file, bool owns_file, string magic_number);
    virtual ~Reader();

    virtual bool supports_read_row() const;
    virtual bool read_row(xel *array, xelval *alpha);

  private:
    unsigned short photomet;
    unsigned short bps, spp;
    xel colormap[TIFF_COLORMAP_MAXCOLORS];
    
    int current_row;
    struct tiff *tif;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, FILE *file, bool owns_file);

    virtual int write_data(xel *array, xelval *alpha);
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeTIFF(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeTIFF",
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

  
