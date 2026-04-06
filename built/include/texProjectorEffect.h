/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texProjectorEffect.h
 * @author drose
 * @date 2004-07-25
 */

#ifndef TEXPROJECTOREFFECT_H
#define TEXPROJECTOREFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"

class LensNode;

/**
 * This effect automatically applies a computed texture matrix to the
 * specified texture stage, according to the relative position of two
 * specified nodes.
 *
 * The relative transform from the "from" node to the "to" node is applied
 * directly to the texture matrix each frame.  If the "to" node happens to be
 * a LensNode, its lens projection matrix is applied as well.
 *
 * This can be used to apply a number of special effects.  Fundamentally, it
 * may simply be used to provide a separate PandaNode that may be adjusted
 * (e.g.  via a LerpInterval) in order to easily apply a linear transformation
 * to an object's texture coordinates (rather than having to explicitly call
 * NodePath.set_tex_transform() each frame).
 *
 * In a more sophisticated case, the TexProjectorEffect is particularly useful
 * in conjunction with a TexGenAttrib that specifies a mode of
 * M_world_position (which copies the world position of each vertex to the
 * texture coordinates).  Then the TexProjector can be used to convert these
 * world coordinates to the relative coordinates of a particular node, causing
 * (for instance) a texture to appear to follow a node around as it moves
 * through the world.  With a LensNode, you can project a texture onto the
 * walls, for instance to apply a flashlight effect or an image-based shadow.
 */
class EXPCL_PANDA_PGRAPH TexProjectorEffect : public RenderEffect {
protected:
  INLINE TexProjectorEffect();
  INLINE TexProjectorEffect(const TexProjectorEffect &copy);

public:
  virtual ~TexProjectorEffect();

PUBLISHED:
  static CPT(RenderEffect) make();

  CPT(RenderEffect) add_stage(TextureStage *stage, const NodePath &from, const NodePath &to, int lens_index = 0) const;
  CPT(RenderEffect) remove_stage(TextureStage *stage) const;

  bool is_empty() const;
  bool has_stage(TextureStage *stage) const;

  NodePath get_from(TextureStage *stage) const;
  NodePath get_to(TextureStage *stage) const;
  int get_lens_index(TextureStage *stage) const;

public:
  virtual void output(std::ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  class StageDef {
  public:
    INLINE StageDef();
    INLINE void set_from(const NodePath &from);
    void set_to(const NodePath &to);
    INLINE void set_lens_index(int lens_index);

    INLINE int compare_to(const StageDef &other) const;

    NodePath _from;
    NodePath _to;
    LensNode *_to_lens_node;
    int _lens_index;
  };

  typedef pmap<PT(TextureStage), StageDef> Stages;
  Stages _stages;

  static CPT(RenderEffect) _empty_effect;

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
    RenderEffect::init_type();
    register_type(_type_handle, "TexProjectorEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "texProjectorEffect.I"

#endif
