/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paletteGroup.h
 * @author drose
 * @date 2000-11-28
 */

#ifndef PALETTEGROUP_H
#define PALETTEGROUP_H

#include "pandatoolbase.h"

#include "paletteGroups.h"
#include "textureProperties.h"

#include "namable.h"
#include "typedWritable.h"

#include "pset.h"
#include "pvector.h"
#include "vector_string.h"

class EggFile;
class TexturePlacement;
class PalettePage;
class TextureImage;
class TxaFile;

/**
 * This is the highest level of grouping for TextureImages.  Textures are
 * assigned to one or several PaletteGroups based on the information in the
 * .txa file; each PaletteGroup is conceptually a collection of textures that
 * are to be moved around (into texture memory, downloaded, etc.) in one big
 * chunk.  It is the set of all textures that may be displayed together at any
 * given time.
 */
class PaletteGroup : public TypedWritable, public Namable {
public:
  PaletteGroup();

  void set_dirname(const std::string &dirname);
  bool has_dirname() const;
  const std::string &get_dirname() const;

  void clear_depends();
  void group_with(PaletteGroup *other);
  const PaletteGroups &get_groups() const;

  void get_placements(pvector<TexturePlacement *> &placements) const;
  void get_complete_placements(pvector<TexturePlacement *> &placements) const;

  void reset_dependency_level();
  void set_dependency_level(int level);
  bool set_dependency_order();
  int get_dependency_level() const;
  int get_dependency_order() const;
  int get_dirname_order() const;

  void set_margin_override(const int override);
  int get_margin_override() const;
  bool has_margin_override() const;

  bool is_preferred_over(const PaletteGroup &other) const;

  void increment_egg_count();
  int get_egg_count() const;

  PalettePage *get_page(const TextureProperties &properties);

  TexturePlacement *prepare(TextureImage *texture);

  void unplace(TexturePlacement *placement);

  void place_all();
  void update_unknown_textures(const TxaFile &txa_file);

  void write_image_info(std::ostream &out, int indent_level = 0) const;
  void optimal_resize();
  void reset_images();
  void setup_shadow_images();
  void update_images(bool redo_all);

  void add_texture_swap_info(const std::string sourceTextureName, const vector_string &swapTextures);
  bool is_none_texture_swap() const;

private:
  std::string _dirname;
  int _egg_count;
  PaletteGroups _dependent;
  int _dependency_level;
  int _dependency_order;
  int _dirname_order;

  typedef pset<TexturePlacement *> Placements;
  Placements _placements;

  typedef pmap<TextureProperties, PalettePage *> Pages;
  Pages _pages;

  typedef pmap<std::string, vector_string> TextureSwapInfo;
  TextureSwapInfo _textureSwapInfo;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);
  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_PaletteGroup(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // These values are only filled in while reading from the bam file; don't
  // use them otherwise.
  int _num_placements;
  int _num_pages;
  bool _has_margin_override;
  int _margin_override;
  pvector<PalettePage *> _load_pages;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    Namable::init_type();
    register_type(_type_handle, "PaletteGroup",
                  TypedWritable::get_class_type(),
                  Namable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;

  friend class PaletteGroups;
};

#endif
