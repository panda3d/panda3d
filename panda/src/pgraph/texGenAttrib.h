/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texGenAttrib.h
 * @author masad
 * @date 2004-06-21
 */

#ifndef TEXGENATTRIB_H
#define TEXGENATTRIB_H

#include "pandabase.h"

#include "geom.h"
#include "renderAttrib.h"
#include "textureStage.h"
#include "texture.h"
#include "pointerTo.h"
#include "nodePath.h"

/**
 * Computes texture coordinates for geometry automatically based on vertex
 * position and/or normal.  This can be used to implement reflection and/or
 * refraction maps, for instance to make shiny surfaces, as well as other
 * special effects such as projective texturing.
 */
class EXPCL_PANDA_PGRAPH TexGenAttrib : public RenderAttrib {
PUBLISHED:
  // We inherit the definition of our Mode enumerated type from RenderAttrib.
  // Normally, Mode would be defined here, but we define it in the base class
  // instead as a hack to avoid a problem with circular includes.
  typedef RenderAttrib::TexGenMode Mode;

protected:
  INLINE TexGenAttrib();
  INLINE TexGenAttrib(const TexGenAttrib &copy);

public:
  virtual ~TexGenAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make(TextureStage *stage, Mode mode);
  static CPT(RenderAttrib) make_default();

  CPT(RenderAttrib) add_stage(TextureStage *stage, Mode mode) const;
  CPT(RenderAttrib) add_stage(TextureStage *stage, Mode mode, const LTexCoord3 &constant_value) const;
  CPT(RenderAttrib) remove_stage(TextureStage *stage) const;

  bool is_empty() const;
  bool has_stage(TextureStage *stage) const;
  Mode get_mode(TextureStage *stage) const;
  bool has_gen_texcoord_stage(TextureStage *stage) const;
  const LTexCoord3 &get_constant_value(TextureStage *stage) const;

  INLINE int get_geom_rendering(int geom_rendering) const;

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  class ModeDef;
  void filled_stages();
  void record_stage(TextureStage *stage, ModeDef &mode_def);

  class ModeDef {
  public:
    INLINE ModeDef();
    INLINE int compare_to(const ModeDef &other) const;
    Mode _mode;
    std::string _source_name;
    NodePath _light;
    LTexCoord3 _constant_value;
  };
  typedef pmap<PT(TextureStage), ModeDef> Stages;
  Stages _stages;

  // This is a set of TextureStage pointers for which texture coordinates will
  // not be needed from the Geom.  It's redundant; it's almost the same set
  // that is listed in _stages, above.  It's just here as an optimization
  // during rendering.
  typedef pset<TextureStage *> NoTexCoordStages;
  NoTexCoordStages _no_texcoords;

  // This element is only used during reading from a bam file.  It has no
  // meaningful value any other time.
  pvector<Mode> _read_modes;

  int _num_point_sprites;

  // _point_geom_rendering is the GeomRendering bits that are added by the
  // TexGenAttrib if there are any points in the Geom.  _geom_rendering is the
  // GeomRendering bits that are added regardless of the kind of Geom it is.
  int _point_geom_rendering;
  int _geom_rendering;

  static CPT(RenderAttrib) _empty_attrib;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "TexGenAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new TexGenAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "texGenAttrib.I"

#endif
