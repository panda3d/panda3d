// Filename: textureReference.h
// Created by:  drose (28Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREREFERENCE_H
#define TEXTUREREFERENCE_H

#include "pandatoolbase.h"

#include "textureProperties.h"
#include "palettizer.h"

#include "luse.h"
#include "typedWritable.h"

class TextureImage;
class SourceTextureImage;
class Filename;
class EggFile;
class EggData;
class EggTexture;
class EggGroupNode;
class EggPrimitive;
class TexturePlacement;

////////////////////////////////////////////////////////////////////
//       Class : TextureReference
// Description : This is the particular reference of a texture
//               filename by an egg file.  It also includes
//               information about the way in which the egg file uses
//               the texture; e.g. does it repeat.
////////////////////////////////////////////////////////////////////
class TextureReference : public TypedWritable {
public:
  TextureReference();
  ~TextureReference();

  void from_egg(EggFile *egg_file, EggData *data, EggTexture *egg_tex);
  void from_egg_quick(const TextureReference &other);
  void release_egg_data();
  void rebind_egg_data(EggData *data, EggTexture *egg_tex);

  EggFile *get_egg_file() const;
  SourceTextureImage *get_source() const;
  TextureImage *get_texture() const;
  const string &get_tref_name() const;

  bool operator < (const TextureReference &other) const;

  bool has_uvs() const;
  const TexCoordd &get_min_uv() const;
  const TexCoordd &get_max_uv() const;

  EggTexture::WrapMode get_wrap_u() const;
  EggTexture::WrapMode get_wrap_v() const;

  bool is_equivalent(const TextureReference &other) const;

  void set_placement(TexturePlacement *placement);
  void clear_placement();
  TexturePlacement *get_placement() const;

  void mark_egg_stale();
  void update_egg();
  void apply_properties_to_source();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;


private:
  bool get_uv_range(EggGroupNode *group, Palettizer::RemapUV remap);
  void update_uv_range(EggGroupNode *group, Palettizer::RemapUV remap);

  bool get_geom_uvs(EggPrimitive *geom,
                    TexCoordd &geom_min_uv, TexCoordd &geom_max_uv);
  void translate_geom_uvs(EggPrimitive *geom, const TexCoordd &trans) const;
  void collect_nominal_uv_range();
  static void collect_uv(bool &any_uvs, TexCoordd &min_uv, TexCoordd &max_uv,
                         const TexCoordd &got_min_uv,
                         const TexCoordd &got_max_uv);
  static LVector2d translate_uv(const TexCoordd &min_uv,
                                const TexCoordd &max_uv);

  EggFile *_egg_file;
  EggTexture *_egg_tex;
  EggData *_egg_data;

  string _tref_name;
  LMatrix3d _tex_mat, _inv_tex_mat;
  SourceTextureImage *_source_texture;
  TexturePlacement *_placement;

  bool _uses_alpha;

  bool _any_uvs;
  TexCoordd _min_uv, _max_uv;
  EggTexture::WrapMode _wrap_u, _wrap_v;

  TextureProperties _properties;

  // The TypedWritable interface follows.
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *writer, Datagram &datagram);
  virtual int complete_pointers(TypedWritable **p_list,
                                BamReader *manager);

protected:
  static TypedWritable *make_TextureReference(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "TextureReference",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &
operator << (ostream &out, const TextureReference &ref) {
  ref.output(out);
  return out;
}

#endif


