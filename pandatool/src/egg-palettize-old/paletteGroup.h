// Filename: paletteGroup.h
// Created by:  drose (06Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PALETTEGROUP_H
#define PALETTEGROUP_H

#include <pandatoolbase.h>

#include <namable.h>

#include <vector>
#include <set>

class Palette;
class TexturePacking;
class AttribFile;
class PaletteGroup;

typedef set<PaletteGroup *> PaletteGroups;

////////////////////////////////////////////////////////////////////
// 	 Class : PaletteGroup
// Description : A named collection of textures.  This is a group of
//               textures that are to be palettized together as a
//               unit; all of these textures are expected to be loaded
//               into texture memory at one time.  The PaletteGroup
//               consists of a number of discrete Palette images, as
//               many as are necessary to represent all of the
//               textures within the PaletteGroup that are to be
//               palettized.
//
//               A PaletteGroup is also associated with one or more
//               other PaletteGroups, which are assumed to be
//               available in texture memory whenever this
//               PaletteGroup is.  If a texture is to be placed on
//               this PaletteGroup that already exists on one of the
//               dependent PaletteGroups, it is not placed; instead,
//               it is referenced directly from the other
//               PaletteGroup.  This allows an intelligent sharing of
//               textures between palettes with a minimum of wasted
//               space.
////////////////////////////////////////////////////////////////////
class PaletteGroup : public Namable {
public:
  PaletteGroup(const string &name);
  ~PaletteGroup();

  int get_num_parents() const;
  PaletteGroup *get_parent(int n) const;
  void add_parent(PaletteGroup *parent);

  const string &get_dirname() const;
  void set_dirname(const string &dirname);

  string get_full_dirname(AttribFile *attrib_file) const;

  bool pack_texture(TexturePacking *packing, AttribFile *attrib_file);
  bool generate_palette_images();
  void optimal_resize();
  void finalize_palettes();

  void add_palette(Palette *palette);

  void remove_palette_files();
  void clear_palettes();

  static void complete_groups(PaletteGroups &groups);
  void add_ancestors(PaletteGroups &groups);

  void write_pi(ostream &out) const;
  void write_palettes_pi(ostream &out) const;

private:
  typedef vector<PaletteGroup *> Parents;
  Parents _parents;

  typedef vector<Palette *> Palettes;
  Palettes _palettes;

  string _dirname;
};


#endif

