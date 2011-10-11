// Filename: texturePosition.h
// Created by:  drose (04Dec00)
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

#ifndef TEXTUREPOSITION_H
#define TEXTUREPOSITION_H

#include "pandatoolbase.h"

#include "typedWritable.h"
#include "luse.h"
#include "eggTexture.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : TexturePosition
// Description : This represents a particular position of a texture
//               within a PaletteImage.  There is only one of these
//               per TexturePlacement, but it exists as a separate
//               structure so the TexturePlacement can easily consider
//               repositioning the texture.
////////////////////////////////////////////////////////////////////
class TexturePosition : public TypedWritable {
public:
  TexturePosition();
  TexturePosition(const TexturePosition &copy);
  void operator = (const TexturePosition &copy);

  int _margin;
  int _x, _y;
  int _x_size, _y_size;

  LTexCoordd _min_uv;
  LTexCoordd _max_uv;

  EggTexture::WrapMode _wrap_u;
  EggTexture::WrapMode _wrap_v;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);

protected:
  static TypedWritable *make_TexturePosition(const FactoryParams &params);

public:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "TexturePosition",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif

