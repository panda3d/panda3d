// Filename: textureProperties.h
// Created by:  drose (28Nov00)
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

#ifndef TEXTUREPROPERTIES_H
#define TEXTUREPROPERTIES_H

#include <pandatoolbase.h>

#include <eggTexture.h>
#include <typedWritable.h>

class PNMFileType;

////////////////////////////////////////////////////////////////////
//       Class : TextureProperties
// Description : This is the set of characteristics of a texture that,
//               if different from another texture, prevent the two
//               textures from sharing a PaletteImage.  It includes
//               properties such as mipmapping, number of channels,
//               etc.
////////////////////////////////////////////////////////////////////
class TextureProperties : public TypedWritable {
public:
  TextureProperties();
  TextureProperties(const TextureProperties &copy);
  void operator = (const TextureProperties &copy);

  bool has_num_channels() const;
  int get_num_channels() const;
  bool uses_alpha() const;

  string get_string() const;
  void update_properties(const TextureProperties &other);
  void fully_define();

  void update_egg_tex(EggTexture *egg_tex) const;

  bool operator < (const TextureProperties &other) const;
  bool operator == (const TextureProperties &other) const;
  bool operator != (const TextureProperties &other) const;

  bool _got_num_channels;
  int _num_channels;
  EggTexture::Format _format;
  EggTexture::FilterType _minfilter, _magfilter;
  PNMFileType *_color_type;
  PNMFileType *_alpha_type;

private:
  static string get_format_string(EggTexture::Format format);
  static string get_filter_string(EggTexture::FilterType filter_type);
  static string get_type_string(PNMFileType *color_type,
                                PNMFileType *alpha_type);

  static EggTexture::Format union_format(EggTexture::Format a,
                                         EggTexture::Format b);
  static EggTexture::FilterType union_filter(EggTexture::FilterType a,
                                             EggTexture::FilterType b);
  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(vector_typedWritable &p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_TextureProperties(const FactoryParams &params);

public:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "TextureProperties",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif

