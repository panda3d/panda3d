// Filename: sourceTextureImage.h
// Created by:  drose (28Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SOURCETEXTUREIMAGE_H
#define SOURCETEXTUREIMAGE_H

#include "pandatoolbase.h"

#include "imageFile.h"

class TextureImage;
class PNMImageHeader;

////////////////////////////////////////////////////////////////////
//       Class : SourceTextureImage
// Description : This is a texture image reference as it appears in an
//               egg file: the source image of the texture.
////////////////////////////////////////////////////////////////////
class SourceTextureImage : public ImageFile {
private:
  SourceTextureImage();

public:
  SourceTextureImage(TextureImage *texture, const Filename &filename,
                     const Filename &alpha_filename, int alpha_file_channel);

  TextureImage *get_texture() const;

  void increment_egg_count();
  int get_egg_count() const;

  bool get_size();
  bool read_header();
  void set_header(const PNMImageHeader &header);

private:
  TextureImage *_texture;
  int _egg_count;
  bool _read_header;
  bool _successfully_read_header;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_SourceTextureImage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ImageFile::init_type();
    register_type(_type_handle, "SourceTextureImage",
                  ImageFile::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &
operator << (ostream &out, const SourceTextureImage &source) {
  source.output_filename(out);
  return out;
}

#endif

