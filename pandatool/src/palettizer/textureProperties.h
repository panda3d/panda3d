/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureProperties.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef TEXTUREPROPERTIES_H
#define TEXTUREPROPERTIES_H

#include "pandatoolbase.h"

#include "eggTexture.h"
#include "typedWritable.h"

class PNMFileType;
class FactoryParams;

/**
 * This is the set of characteristics of a texture that, if different from
 * another texture, prevent the two textures from sharing a PaletteImage.  It
 * includes properties such as mipmapping, number of channels, etc.
 */
class TextureProperties : public TypedWritable {
public:
  TextureProperties();
  TextureProperties(const TextureProperties &copy);
  void operator = (const TextureProperties &copy);

  void clear_basic();

  bool has_num_channels() const;
  int get_num_channels() const;
  void set_num_channels(int num_channels);
  void force_grayscale();
  void force_nonalpha();
  bool uses_alpha() const;

  std::string get_string() const;
  void update_properties(const TextureProperties &other);
  void fully_define();

  void update_egg_tex(EggTexture *egg_tex) const;
  bool egg_properties_match(const TextureProperties &other) const;

  bool operator < (const TextureProperties &other) const;
  bool operator == (const TextureProperties &other) const;
  bool operator != (const TextureProperties &other) const;

  EggTexture::Format _format;
  bool _force_format;  // true when format has been explicitly specified
  bool _generic_format; // true if 'generic' keyword, meaning rgba8 -> rgba.
  bool _keep_format;   // true if 'keep-format' keyword.
  EggTexture::FilterType _minfilter, _magfilter;
  EggTexture::QualityLevel _quality_level;
  int _anisotropic_degree;
  PNMFileType *_color_type;
  PNMFileType *_alpha_type;

private:
  static std::string get_format_string(EggTexture::Format format);
  static std::string get_filter_string(EggTexture::FilterType filter_type);
  static std::string get_anisotropic_degree_string(int aniso_degree);
  static std::string get_quality_level_string(EggTexture::QualityLevel quality_level);
  static std::string get_type_string(PNMFileType *color_type,
                                PNMFileType *alpha_type);

  static EggTexture::Format union_format(EggTexture::Format a,
                                         EggTexture::Format b);

  static EggTexture::FilterType union_filter(EggTexture::FilterType a,
                                             EggTexture::FilterType b);
  static EggTexture::QualityLevel union_quality_level(EggTexture::QualityLevel a,
                                                      EggTexture::QualityLevel b);

  bool _got_num_channels;
  int _num_channels;
  int _effective_num_channels;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
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
