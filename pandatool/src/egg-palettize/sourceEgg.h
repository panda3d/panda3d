// Filename: sourceEgg.h
// Created by:  drose (02Sep99)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOURCEGG_H
#define SOURCEGG_H

#include <pandatoolbase.h>

#include <eggData.h>
#include <luse.h>


class Texture;
class AttribFile;
class EggPalettize;
class EggTexture;
class EggGroup;
    
class SourceEgg : public EggData {
public:
  class TextureRef;

  SourceEgg();

  TextureRef &add_texture(Texture *texture, bool repeats, bool alpha);
  void get_textures(AttribFile &af, EggPalettize *prog);

  void mark_texture_flags();
  void update_trefs();

  bool needs_rebuild(bool force_redo_all, 
			bool eggs_include_images) const;

  void write_pi(ostream &out) const;


  class TextureRef {
  public:
    TextureRef(Texture *texture, bool repeats, bool alpha);

    Texture *_texture;
    bool _repeats;
    bool _alpha;

    EggTexture *_eggtex;
  };

private:  
  void get_uv_range(EggGroupNode *group, EggTexture *tref,
		    bool &any_uvs, TexCoordd &min_uv, TexCoordd &max_uv);

  typedef vector<TextureRef> TexRefs;
  TexRefs _texrefs;

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
