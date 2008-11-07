// Filename: pnmFileType.h
// Created by:  drose (15Jun00)
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

#ifndef PNMFILETYPE_H
#define PNMFILETYPE_H

#include "pandabase.h"

#include "pnmimage_base.h"

#include "typedObject.h"
#include "typedWritable.h"

class PNMReader;
class PNMWriter;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : PNMFileType
// Description : This is the base class of a family of classes that
//               represent particular image file types that PNMImage
//               supports.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGE PNMFileType : public TypedWritable {
protected:
  PNMFileType();

public:
  virtual ~PNMFileType();

PUBLISHED:
  virtual string get_name() const=0;

  virtual int get_num_extensions() const;
  virtual string get_extension(int n) const;
  MAKE_SEQ(get_extensions, get_num_extensions, get_extension);
  virtual string get_suggested_extension() const;

public:
  virtual bool has_magic_number() const;
  virtual bool matches_magic_number(const string &magic_number) const;

  virtual PNMReader *make_reader(istream *file, bool owns_file = true,
                                 const string &magic_number = string());
  virtual PNMWriter *make_writer(ostream *file, bool owns_file = true);

protected:
  static void init_pnm();

private:
  static bool _did_init_pnm;


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);

protected:
  static TypedWritable *make_PNMFileType(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "PNMFileType",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif

