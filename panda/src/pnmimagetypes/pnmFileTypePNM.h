// Filename: pnmFileTypePNM.h
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PNMFILETYPEPNM_H
#define PNMFILETYPEPNM_H

#include <pandabase.h>

#include <pnmFileType.h>
#include <pnmReader.h>
#include <pnmWriter.h>

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypePNM
// Description : For reading and writing basic PNM files--*.pbm,
//               *.ppm, *.pnm.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypePNM : public PNMFileType {
public:
  PNMFileTypePNM();

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

    virtual bool supports_read_row() const;
    virtual bool read_row(xel *array, xelval *alpha);

  private:
    int _ftype;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, FILE *file, bool owns_file);

    virtual bool supports_write_row() const;
    virtual bool write_header();
    virtual bool write_row(xel *array, xelval *alpha);

  private:
    int _pnm_format;
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypePNM(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypePNM",
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

  
