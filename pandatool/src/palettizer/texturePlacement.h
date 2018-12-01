/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePlacement.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef TEXTUREPLACEMENT_H
#define TEXTUREPLACEMENT_H

#include "pandatoolbase.h"

#include "omitReason.h"
#include "texturePosition.h"

#include "typedWritable.h"
#include "luse.h"

#include "pset.h"

class TextureImage;
class DestTextureImage;
class PaletteGroup;
class PaletteImage;
class PalettePage;
class TextureProperties;
class TextureReference;
class PNMImage;

/**
 * This corresponds to a particular assignment of a TextureImage with a
 * PaletteGroup, and specifically describes which PaletteImage (if any), and
 * where on the PaletteImage, the TextureImage has been assigned to.
 */
class TexturePlacement : public TypedWritable {
private:
  TexturePlacement();

public:
  TexturePlacement(TextureImage *texture, PaletteGroup *group);
  ~TexturePlacement();

  const std::string &get_name() const;
  TextureImage *get_texture() const;
  const TextureProperties &get_properties() const;
  PaletteGroup *get_group() const;

  void add_egg(TextureReference *reference);
  void remove_egg(TextureReference *reference);
  void mark_eggs_stale();

  void set_dest(DestTextureImage *dest);
  DestTextureImage *get_dest() const;

  bool determine_size();
  bool is_size_known() const;
  OmitReason get_omit_reason() const;
  int get_x_size() const;
  int get_y_size() const;
  double get_uv_area() const;

  bool is_placed() const;
  PaletteImage *get_image() const;
  PalettePage *get_page() const;
  int get_placed_x() const;
  int get_placed_y() const;
  int get_placed_x_size() const;
  int get_placed_y_size() const;
  double get_placed_uv_area() const;

  void place_at(PaletteImage *image, int x, int y);
  void force_replace();
  void omit_solitary();
  void not_solitary();
  bool intersects(int x, int y, int x_size, int y_size);

  void compute_tex_matrix(LMatrix3d &transform);

  void write_placed(std::ostream &out, int indent_level = 0);

  bool is_filled() const;
  void mark_unfilled();
  void fill_image(PNMImage &image);
  void fill_swapped_image(PNMImage &image, int index);
  void flag_error_image(PNMImage &image);

  typedef pvector<TextureImage *> TextureSwaps;
  TextureSwaps _textureSwaps;

private:
  void compute_size_from_uvs(const LTexCoordd &min_uv, const LTexCoordd &max_uv);

  TextureImage *_texture;
  PaletteGroup *_group;
  PaletteImage *_image;
  DestTextureImage *_dest;

  bool _has_uvs;
  bool _size_known;
  TexturePosition _position;

  bool _is_filled;
  TexturePosition _placed;
  OmitReason _omit_reason;

  typedef pset<TextureReference *> References;
  References _references;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_TexturePlacement(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // This value is only filled in while reading from the bam file; don't use
  // it otherwise.
  int _num_references;
  int _num_textureSwaps;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "TexturePlacement",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};


// This is an STL object to sort an array of TexturePlacement pointers in
// order from biggest to smallest.
class SortPlacementBySize {
public:
  bool operator ()(TexturePlacement *a, TexturePlacement *b) const;
};

#endif
