/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texMatrixAttrib.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef TEXMATRIXATTRIB_H
#define TEXMATRIXATTRIB_H

#include "pandabase.h"

#include "geom.h"
#include "renderAttrib.h"
#include "textureStage.h"
#include "transformState.h"
#include "pointerTo.h"

class FactoryParams;

/**
 * Applies a transform matrix to UV's before they are rendered.
 */
class EXPCL_PANDA_PGRAPH TexMatrixAttrib : public RenderAttrib {
protected:
  INLINE TexMatrixAttrib();
  INLINE TexMatrixAttrib(const TexMatrixAttrib &copy);

public:
  virtual ~TexMatrixAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make(const LMatrix4 &mat);
  static CPT(RenderAttrib) make(TextureStage *stage, const TransformState *transform);
  static CPT(RenderAttrib) make_default();

  CPT(RenderAttrib) add_stage(TextureStage *stage, const TransformState *transform, int override = 0) const;
  CPT(RenderAttrib) remove_stage(TextureStage *stage) const;

  bool is_empty() const;
  bool has_stage(TextureStage *stage) const;

  int get_num_stages() const;
  TextureStage *get_stage(int n) const;
  MAKE_SEQ(get_stages, get_num_stages, get_stage);

  const LMatrix4 &get_mat() const;
  const LMatrix4 &get_mat(TextureStage *stage) const;

  CPT(TransformState) get_transform(TextureStage *stage) const;
  INLINE int get_override(TextureStage *stage) const;

  INLINE int get_geom_rendering(int geom_rendering) const;

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  INLINE void check_stage_list() const;
  void rebuild_stage_list();

private:
  class StageNode {
  public:
    INLINE StageNode(const TextureStage *stage);

    INLINE bool operator < (const StageNode &other) const;

    PT(TextureStage) _stage;
    CPT(TransformState) _transform;
    int _override;
  };

  class CompareTextureStagePointer {
  public:
    INLINE bool operator () (const TexMatrixAttrib::StageNode &a, const TexMatrixAttrib::StageNode &b) const;
  };

  typedef ov_set<StageNode, CompareTextureStagePointer> Stages;
  Stages _stages;

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
    register_type(_type_handle, "TexMatrixAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new TexMatrixAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "texMatrixAttrib.I"

#endif
