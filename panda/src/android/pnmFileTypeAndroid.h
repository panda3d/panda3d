/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeAndroid.h
 * @author rdb
 * @date 2013-01-11
 */

#ifndef PNMFILETYPEANDROID_H
#define PNMFILETYPEANDROID_H

#ifdef ANDROID

#include "pandabase.h"

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

#include <jni.h>

/**
 * Wrapper class around the Android Bitmap mechanism to allow loading images
 * on Android without needing libpng or libjpeg.
 */
class PNMFileTypeAndroid : public PNMFileType {
public:
  enum CompressFormat : jint {
    CF_jpeg = 0,
    CF_png = 1,
    CF_webp = 2,
  };

  PNMFileTypeAndroid(CompressFormat format);

  virtual std::string get_name() const;

  virtual int get_num_extensions() const;
  virtual std::string get_extension(int n) const;

  virtual bool has_magic_number() const;

  virtual PNMReader *make_reader(std::istream *file, bool owns_file = true,
                                 const std::string &magic_number = std::string());
  virtual PNMWriter *make_writer(std::ostream *file, bool owns_file = true);

public:
  class Reader : public PNMReader {
  public:
    Reader(PNMFileType *type, std::istream *file, bool owns_file, std::string magic_number);
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

  class Writer : public PNMWriter {
  public:
    Writer(PNMFileType *type, std::ostream *file, bool owns_file,
           CompressFormat format);

    virtual int write_data(xel *array, xelval *alpha);
    virtual bool supports_grayscale() const;

  private:
    CompressFormat _format;
  };

private:
  CompressFormat _format;
};

#endif  // ANDROID

#endif
