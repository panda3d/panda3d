/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureReference.h
 * @author drose
 * @date 2000-11-28
 */

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

/**
 * This is the particular reference of a texture filename by an egg file.  It
 * also includes information about the way in which the egg file uses the
 * texture; e.g.  does it repeat.
 */
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
  const std::string &get_tref_name() const;

  bool operator < (const TextureReference &other) const;

  bool has_uvs() const;
  const LTexCoordd &get_min_uv() const;
  const LTexCoordd &get_max_uv() const;

  EggTexture::WrapMode get_wrap_u() const;
  EggTexture::WrapMode get_wrap_v() const;

  bool is_equivalent(const TextureReference &other) const;

  void set_placement(TexturePlacement *placement);
  void clear_placement();
  TexturePlacement *get_placement() const;

  void mark_egg_stale();
  void update_egg();
  void apply_properties_to_source();

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;


private:
  bool get_uv_range(EggGroupNode *group, Palettizer::RemapUV remap);
  void update_uv_range(EggGroupNode *group, Palettizer::RemapUV remap);

  bool get_geom_uvs(EggPrimitive *geom,
                    LTexCoordd &geom_min_uv, LTexCoordd &geom_max_uv);
  void translate_geom_uvs(EggPrimitive *geom, const LTexCoordd &trans) const;
  void collect_nominal_uv_range();
  static void collect_uv(bool &any_uvs, LTexCoordd &min_uv, LTexCoordd &max_uv,
                         const LTexCoordd &got_min_uv,
                         const LTexCoordd &got_max_uv);
  static LVector2d translate_uv(const LTexCoordd &min_uv,
                                const LTexCoordd &max_uv);

  EggFile *_egg_file;
  EggTexture *_egg_tex;
  EggData *_egg_data;

  std::string _tref_name;
  LMatrix3d _tex_mat, _inv_tex_mat;
  SourceTextureImage *_source_texture;
  TexturePlacement *_placement;

  bool _uses_alpha;

  bool _any_uvs;
  LTexCoordd _min_uv, _max_uv;
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

INLINE std::ostream &
operator << (std::ostream &out, const TextureReference &ref) {
  ref.output(out);
  return out;
}

#endif
