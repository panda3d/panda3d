// Filename: pnmFileTypePfm.h
// Created by:  drose (17Jun00)
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

#ifndef PNMFILETYPEPFM_H
#define PNMFILETYPEPFM_H

#include "pandabase.h"

#include "pnmFileType.h"
#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypePfm
// Description : For reading and writing PFM files using the basic
//               PNMImage interface, as if they were basic RGB files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGE PNMFileTypePfm : public PNMFileType {
public:
  PNMFileTypePfm();

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
    
    virtual bool is_floating_point();
    virtual bool read_pfm(PfmFile &pfm);

  private:
    PN_float32 _scale;
  };

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, ostream *file, bool owns_file);

    virtual bool supports_floating_point();
    virtual bool supports_integer();
    virtual bool write_pfm(const PfmFile &pfm);
  };


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypePfm(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypePfm",
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


