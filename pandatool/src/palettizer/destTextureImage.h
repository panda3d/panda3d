// Filename: destTextureImage.h
// Created by:  drose (05Dec00)
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

#ifndef DESTTEXTUREIMAGE_H
#define DESTTEXTUREIMAGE_H

#include "pandatoolbase.h"

#include "imageFile.h"

class TexturePlacement;
class TextureImage;

////////////////////////////////////////////////////////////////////
//       Class : DestTextureImage
// Description : This represents a texture filename as it has been
//               resized and copied to the map directory (e.g. for an
//               unplaced texture).
////////////////////////////////////////////////////////////////////
class DestTextureImage : public ImageFile {
private:
  DestTextureImage();

public:
  DestTextureImage(TexturePlacement *placement);

  void copy(TextureImage *texture);
  void copy_if_stale(const DestTextureImage *other, TextureImage *texture);

private:
  static int to_power_2(int value);


  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);

protected:
  static TypedWritable *make_DestTextureImage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageFile::init_type();
    register_type(_type_handle, "DestTextureImage",
                  ImageFile::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &
operator << (ostream &out, const DestTextureImage &dest) {
  dest.output_filename(out);
  return out;
}

#endif

