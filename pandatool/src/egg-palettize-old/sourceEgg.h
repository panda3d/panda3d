// Filename: sourceEgg.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOURCEGG_H
#define SOURCEGG_H

#include <pandatoolbase.h>

#include "paletteGroup.h"

#include <eggData.h>
#include <luse.h>

class PTexture;
class TexturePacking;
class AttribFile;
class EggPalettize;
class EggTexture;
class EggGroup;
class TextureEggRef;
    
class SourceEgg : public EggData {
public:
  SourceEgg(AttribFile *attrib_file);

  TextureEggRef *add_texture(PTexture *texture, TexturePacking *packing,
			     bool repeats, bool alpha);
  void get_textures(EggPalettize *prog);

  void add_group(PaletteGroup *group);
  void require_groups(PaletteGroup *preferred, const PaletteGroups &groups);
  void all_textures_assigned();

  void mark_texture_flags();
  void update_trefs();

  bool needs_rebuild(bool force_redo_all, 
		     bool eggs_include_images) const;

  bool matched_anything() const;
  void set_matched_anything(bool matched_anything);

  void write_pi(ostream &out) const;


  typedef set<TextureEggRef *> TexRefs;
  TexRefs _texrefs;

private:  
  void get_uv_range(EggGroupNode *group, EggTexture *tref,
		    bool &any_uvs, TexCoordd &min_uv, TexCoordd &max_uv);


  AttribFile *_attrib_file;
  bool _matched_anything;

  PaletteGroups _groups;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggData::init_type();
    register_type(_type_handle, "SourceEgg",
                  EggData::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif
