/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeStbImage.h
 * @author rdb
 * @date 2016-03-31
 */

#ifndef PNMFILETYPESTBIMAGE_H
#define PNMFILETYPESTBIMAGE_H

#include "pandabase.h"

#ifdef HAVE_STB_IMAGE

#include "pnmFileType.h"
#include "pnmReader.h"
#include "pnmWriter.h"

/**
 * For reading images via the public domain stb_image.h library.  This is used
 * when compiling without support for more specific libraries that are more
 * full-featured, such as libpng or libjpeg.
 */
class EXPCL_PANDA_PNMIMAGETYPES PNMFileTypeStbImage : public PNMFileType {
public:
  PNMFileTypeStbImage();

  virtual std::string get_name() const;

  virtual int get_num_extensions() const;
  virtual std::string get_extension(int n) const;

  virtual bool has_magic_number() const;
  virtual bool matches_magic_number(const std::string &magic_number) const;

  virtual PNMReader *make_reader(std::istream *file, bool owns_file = true,
                                 const std::string &magic_number = std::string());

public:
  static void register_with_read_factory();

protected:
  static TypedWritable *make_PNMFileTypeStbImage(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PNMFileType::init_type();
    register_type(_type_handle, "PNMFileTypeStbImage",
                  PNMFileType::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif  // HAVE_STB_IMAGE

#endif
