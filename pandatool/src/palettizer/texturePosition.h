// Filename: texturePosition.h
// Created by:  drose (04Dec00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREPOSITION_H
#define TEXTUREPOSITION_H

#include <pandatoolbase.h>

#include <typedWritable.h>
#include <luse.h>
#include <eggTexture.h>

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

  TexCoordd _min_uv;
  TexCoordd _max_uv;

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

