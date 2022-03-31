/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureAttrib.h
 * @author drose
 * @date 2002-02-21
 */

#ifndef TEXTUREATTRIB_H
#define TEXTUREATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "texture.h"
#include "textureStage.h"
#include "updateSeq.h"
#include "ordered_vector.h"
#include "vector_int.h"
#include "epvector.h"

/**
 * Indicates the set of TextureStages and their associated Textures that
 * should be applied to (or removed from) a node.
 */
class EXPCL_PANDA_PGRAPH TextureAttrib : public RenderAttrib {
protected:
  INLINE TextureAttrib();
  INLINE TextureAttrib(const TextureAttrib &copy);

PUBLISHED:
  // These methods are used to create a simple, single-textured layer.  For
  // multitexture, use the multitexture interfaces, further below.
  static CPT(RenderAttrib) make(Texture *tex);
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  INLINE bool is_off() const;
  INLINE Texture *get_texture() const;

  // The following methods define the new multitexture mode for TextureAttrib.
  // Each TextureAttrib can add or remove individual texture stages from the
  // complete set of textures that are to be applied; this is similar to the
  // mechanism of LightAttrib.
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make_all_off();

  INLINE int get_num_on_stages() const;
  INLINE TextureStage *get_on_stage(int n) const;
  MAKE_SEQ(get_on_stages, get_num_on_stages, get_on_stage);
  INLINE int get_num_on_ff_stages() const;
  INLINE TextureStage *get_on_ff_stage(int n) const;
  MAKE_SEQ(get_on_ff_stages, get_num_on_ff_stages, get_on_ff_stage);
  INLINE int get_ff_tc_index(int n) const;
  INLINE bool has_on_stage(TextureStage *stage) const;
  INLINE Texture *get_on_texture(TextureStage *stage) const;
  INLINE const SamplerState &get_on_sampler(TextureStage *stage) const;
  INLINE int get_on_stage_override(TextureStage *stage) const;

  int find_on_stage(const TextureStage *stage) const;

  MAKE_SEQ_PROPERTY(on_stages, get_num_on_stages, get_on_stage);

  MAKE_MAP_PROPERTY(textures, has_on_stage, get_on_texture);
  MAKE_MAP_KEYS_SEQ(textures, get_num_on_stages, get_on_stage);

  MAKE_MAP_PROPERTY(samplers, has_on_stage, get_on_sampler);
  MAKE_MAP_KEYS_SEQ(samplers, get_num_on_stages, get_on_stage);

  INLINE int get_num_off_stages() const;
  INLINE TextureStage *get_off_stage(int n) const;
  MAKE_SEQ(get_off_stages, get_num_off_stages, get_off_stage);
  INLINE bool has_off_stage(TextureStage *stage) const;
  INLINE bool has_all_off() const;

  MAKE_SEQ_PROPERTY(off_stages, get_num_off_stages, get_off_stage);

  INLINE bool is_identity() const;

  CPT(RenderAttrib) add_on_stage(TextureStage *stage, Texture *tex, int override = 0) const;
  CPT(RenderAttrib) add_on_stage(TextureStage *stage, Texture *tex,
                                 const SamplerState &sampler, int override = 0) const;
  CPT(RenderAttrib) remove_on_stage(TextureStage *stage) const;
  CPT(RenderAttrib) add_off_stage(TextureStage *stage, int override = 0) const;
  CPT(RenderAttrib) remove_off_stage(TextureStage *stage) const;
  CPT(RenderAttrib) unify_texture_stages(TextureStage *stage) const;
  CPT(RenderAttrib) replace_texture(Texture *tex, Texture *new_tex) const;

public:
  CPT(TextureAttrib) filter_to_max(int max_texture_stages) const;

  virtual bool lower_attrib_can_override() const;
  virtual void output(std::ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  INLINE void check_sorted() const;
  void sort_on_stages();

private:
  class StageNode {
  public:
    INLINE StageNode(const TextureStage *stage,
                     unsigned int implicit_sort = 0,
                     int override = 0);

    SamplerState _sampler;
    PT(TextureStage) _stage;
    PT(Texture) _texture;
    bool _has_sampler;
    int _ff_tc_index;
    unsigned int _implicit_sort;
    int _override;
  };

  class CompareTextureStagePriorities {
  public:
    INLINE bool operator () (const TextureAttrib::StageNode *a, const TextureAttrib::StageNode *b) const;
  };

  class CompareTextureStageSort {
  public:
    INLINE bool operator () (const TextureAttrib::StageNode *a, const TextureAttrib::StageNode *b) const;
  };

  class CompareTextureStagePointer {
  public:
    INLINE bool operator () (const TextureAttrib::StageNode &a, const TextureAttrib::StageNode &b) const;
  };

  typedef ov_set<StageNode, CompareTextureStagePointer, epvector<StageNode> > Stages;
  Stages _on_stages;  // set of all "on" stages, indexed by pointer.

  typedef pvector<StageNode *> RenderStages;
  RenderStages _render_stages;      // all "on" stages, sorted in render order.
  RenderStages _render_ff_stages;   // fixed-function stages only, in render order.
  unsigned int _next_implicit_sort;

  Stages _off_stages;
  bool _off_all_stages;

  typedef pmap< int, CPT(TextureAttrib) > Filtered;
  Filtered _filtered;

  UpdateSeq _sort_seq;
  UpdateSeq _filtered_seq;

  static CPT(RenderAttrib) _empty_attrib;
  static CPT(RenderAttrib) _all_off_attrib;

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
    register_type(_type_handle, "TextureAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 30, new TextureAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "textureAttrib.I"

#endif
