// Filename: paletteGroup.h
// Created by:  drose (28Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PALETTEGROUP_H
#define PALETTEGROUP_H

#include <pandatoolbase.h>

#include "paletteGroups.h"
#include "textureProperties.h"

#include <namable.h>
#include <typedWriteable.h>

#include <set>

class EggFile;
class TexturePlacement;
class PalettePage;
class TextureImage;

////////////////////////////////////////////////////////////////////
// 	 Class : PaletteGroup
// Description : This is the highest level of grouping for
//               TextureImages.  Textures are assigned to one or
//               several PaletteGroups based on the information in the
//               .txa file; each PaletteGroup is conceptually a
//               collection of textures that are to be moved around
//               (into texture memory, downloaded, etc.) in one big
//               chunk.  It is the set of all textures that may be
//               displayed together at any given time.
////////////////////////////////////////////////////////////////////
class PaletteGroup : public TypedWriteable, public Namable {
public:
  PaletteGroup();

  void set_dirname(const string &dirname);
  bool has_dirname() const;
  const string &get_dirname() const;

  void clear_depends();
  void group_with(PaletteGroup *other);
  const PaletteGroups &get_groups() const;

  void set_dependency_level(int level);
  int get_dependency_level() const;

  void increment_egg_count();
  int get_egg_count() const;

  PalettePage *get_page(const TextureProperties &properties);

  TexturePlacement *prepare(TextureImage *texture);

  void unplace(TexturePlacement *placement);

  void place_all();

  void write_image_info(ostream &out, int indent_level = 0) const;
  void update_images();

private:
  string _dirname;
  int _egg_count;
  PaletteGroups _dependent;
  int _dependency_level;

  typedef set<TexturePlacement *> Placements;
  Placements _placements;

  typedef map<TextureProperties, PalettePage *> Pages;
  Pages _pages;


  // The TypedWriteable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram); 
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);
  virtual void finalize();

protected:
  static TypedWriteable *make_PaletteGroup(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  // These values are only filled in while reading from the bam file;
  // don't use them otherwise.
  int _num_placements;
  int _num_pages;
  vector<PalettePage *> _load_pages;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteable::init_type();
    Namable::init_type();
    register_type(_type_handle, "PaletteGroup",
		  TypedWriteable::get_class_type(),
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

