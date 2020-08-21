/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file palettePage.h
 * @author drose
 * @date 2000-12-01
 */

#ifndef PALETTEPAGE_H
#define PALETTEPAGE_H

#include "pandatoolbase.h"

#include "textureProperties.h"

#include "namable.h"
#include "typedWritable.h"

class PaletteGroup;
class PaletteImage;
class TexturePlacement;

/**
 * This is a particular collection of textures, within a PaletteGroup, that
 * all share the same TextureProperties.  The textures on the same page may
 * therefore all be placed on the same set of PaletteImages together.
 */
class PalettePage : public TypedWritable, public Namable {
private:
  PalettePage();

public:
  PalettePage(PaletteGroup *group, const TextureProperties &properties);

  PaletteGroup *get_group() const;
  const TextureProperties &get_properties() const;

  void assign(TexturePlacement *placement);
  void place_all();
  void place(TexturePlacement *placement);
  void unplace(TexturePlacement *placement);

  void write_image_info(std::ostream &out, int indent_level = 0) const;
  void optimal_resize();
  void reset_images();
  void setup_shadow_images();
  void update_images(bool redo_all);

private:
  PaletteGroup *_group;
  TextureProperties _properties;

  typedef pvector<TexturePlacement *> Assigned;
  Assigned _assigned;

  typedef pvector<PaletteImage *> Images;
  Images _images;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_PalettePage(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // This value is only filled in while reading from the bam file; don't use
  // it otherwise.
  int _num_images;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    Namable::init_type();
    register_type(_type_handle, "PalettePage",
                  TypedWritable::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#endif
