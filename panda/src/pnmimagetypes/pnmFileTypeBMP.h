// Filename: pnmFileTypeBMP.h
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PNMFILETYPEBMP_H
#define PNMFILETYPEBMP_H

#include <pandabase.h>

#include <pnmFileType.h>
#include <pnmReader.h>
#include <pnmWriter.h>

////////////////////////////////////////////////////////////////////
// 	 Class : PNMFileTypeBMP
// Description : For reading and writing Windows BMP files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMFileTypeBMP : public PNMFileType {
public:
  PNMFileTypeBMP();

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

    virtual int read_data(xel *array, xelval *alpha);

  private:
    unsigned long	pos;
    
    unsigned long offBits;
    
    unsigned short  cBitCount;
    int             indexed;
    int             classv;
    
    pixval R[256];	/* reds */
    pixval G[256];	/* greens */
    pixval B[256];	/* blues */
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

#endif

  
