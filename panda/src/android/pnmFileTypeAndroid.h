// Filename: pnmFileTypeAndroid.h
// Created by:  rdb (11Jan13)
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

#ifndef PNMFILETYPEANDROID_H
#define PNMFILETYPEANDROID_H

#ifdef ANDROID

#include "pandabase.h"

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

#include <jni.h>

////////////////////////////////////////////////////////////////////
//       Class : PNMFileTypeAndroid
// Description : Wrapper class around the Android Bitmap mechanism
//               to allow loading images on Android without needing
//               libpng or libjpeg.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeAndroid : public PNMFileType {
public:
  PNMFileTypeAndroid();

  virtual string get_name() const;

  virtual int get_num_extensions() const;
  virtual string get_extension(int n) const;

  virtual bool has_magic_number() const;

  virtual PNMReader *make_reader(istream *file, bool owns_file = true,
                                 const string &magic_number = string());

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, istream *file, bool owns_file, string magic_number);
    virtual ~Reader();

    virtual void prepare_read();
    virtual int read_data(xel *array, xelval *alpha);

  private:
    // It is assumed that the Reader is only used within a single thread.
    JNIEnv *_env;
    jobject _bitmap;
    int _sample_size;
    uint32_t _stride;
    int32_t _format;
  };

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeAndroid(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeAndroid",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // ANDROID

#endif
