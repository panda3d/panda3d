/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramTexture.h
 * @author rdb
 * @date 2014-12-11
 */

#ifndef PARAMTEXTURE_H
#define PARAMTEXTURE_H

#include "pandabase.h"
#include "paramValue.h"
#include "samplerState.h"
#include "texture.h"

/**
 * A class object for storing a pointer to a Texture along with a sampler
 * state that indicates how to to sample the given texture.
 */
class EXPCL_PANDA_GOBJ ParamTextureSampler : public ParamValueBase {
protected:
  INLINE ParamTextureSampler() {};

PUBLISHED:
  INLINE ParamTextureSampler(Texture *tex, const SamplerState &sampler);

  INLINE virtual TypeHandle get_value_type() const;
  INLINE Texture *get_texture() const;
  INLINE const SamplerState &get_sampler() const;

  MAKE_PROPERTY(texture, get_texture);
  MAKE_PROPERTY(sampler, get_sampler);

  virtual void output(std::ostream &out) const;

private:
  PT(Texture) _texture;
  SamplerState _sampler;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "ParamTextureSampler",
                  ParamValueBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

/**
 * A class object for storing a pointer to a Texture along with a set of
 * properties that indicates which image to bind to a shader input.
 *
 * This class is useful for binding texture images to a shader, which is a
 * fairly esoteric feature.
 */
class EXPCL_PANDA_GOBJ ParamTextureImage : public ParamValueBase {
protected:
  INLINE ParamTextureImage() {};

  enum AccessFlags {
    A_read    = 0x01,
    A_write   = 0x02,
    A_layered = 0x04,
  };

PUBLISHED:
  INLINE ParamTextureImage(Texture *tex, bool read, bool write, int z=-1, int n=0);

  INLINE virtual TypeHandle get_value_type() const;

  INLINE Texture *get_texture() const;
  INLINE bool has_read_access() const;
  INLINE bool has_write_access() const;
  INLINE bool get_bind_layered() const;
  INLINE int get_bind_level() const;
  INLINE int get_bind_layer() const;

  MAKE_PROPERTY(texture, get_texture);
  MAKE_PROPERTY(read_access, has_read_access);
  MAKE_PROPERTY(write_access, has_write_access);
  MAKE_PROPERTY(bind_level, get_bind_level);
  MAKE_PROPERTY2(bind_layer, get_bind_layered, get_bind_layer);

  virtual void output(std::ostream &out) const;

private:
  PT(Texture) _texture;
  int _access : 4;
  int _bind_level : 8;
  int _bind_layer : 20;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "ParamTextureImage",
                  ParamValueBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "paramTexture.I"

#endif
