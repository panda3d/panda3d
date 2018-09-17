/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file portalNode.h
 * @author masad
 * @date 2004-05-13
 */

#ifndef PORTALNODE_H
#define PORTALNODE_H

#include "pandabase.h"

#include "portalMask.h"
#include "pandaNode.h"
#include "planeNode.h"
#include "nodePath.h"
#include "pvector.h"

/**
 * A node in the scene graph that can hold a Portal Polygon, which is a
 * rectangle.  Other types of polygons are not supported for now.  It also
 * holds a PT(PandaNode) Cell that this portal is connected to
 */
class EXPCL_PANDA_PGRAPH PortalNode : public PandaNode {
PUBLISHED:
  explicit PortalNode(const std::string &name);
  explicit PortalNode(const std::string &name, LPoint3 pos, PN_stdfloat scale=10.0);

protected:
  PortalNode(const PortalNode &copy);

public:
  virtual ~PortalNode();
  virtual PandaNode *make_copy() const;
  virtual bool preserve_name() const;
  virtual void xform(const LMatrix4 &mat);
  virtual PandaNode *combine_with(PandaNode *other);

  virtual void enable_clipping_planes();

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE void set_portal_mask(PortalMask mask);
  INLINE void set_from_portal_mask(PortalMask mask);
  INLINE void set_into_portal_mask(PortalMask mask);
  INLINE PortalMask get_from_portal_mask() const;
  INLINE PortalMask get_into_portal_mask() const;

  INLINE void set_portal_geom(bool flag);
  INLINE bool get_portal_geom() const;

  INLINE void clear_vertices();
  INLINE void add_vertex(const LPoint3 &vertex);

  INLINE int get_num_vertices() const;
  INLINE const LPoint3 &get_vertex(int n) const;
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);

  INLINE void set_cell_in(const NodePath &cell);
  INLINE NodePath get_cell_in() const;

  INLINE void set_cell_out(const NodePath &cell);
  INLINE NodePath get_cell_out() const;

  INLINE void set_clip_plane(bool value);
  INLINE bool is_clip_plane();

  INLINE void set_visible(bool value);
  INLINE bool is_visible();

  INLINE void set_max_depth(int value);
  INLINE int get_max_depth();

  INLINE void set_open(bool value);
  INLINE bool is_open();

  // void draw () const;

  MAKE_PROPERTY(into_portal_mask, get_into_portal_mask, set_into_portal_mask);
  MAKE_PROPERTY(from_portal_mask, get_from_portal_mask, set_from_portal_mask);
  MAKE_PROPERTY(portal_geom, get_portal_geom, set_portal_geom);
  MAKE_SEQ_PROPERTY(vertices, get_num_vertices, get_vertex);
  MAKE_PROPERTY(cell_in, get_cell_in, set_cell_in);
  MAKE_PROPERTY(cell_out, get_cell_out, set_cell_out);
  MAKE_PROPERTY(clip_plane, is_clip_plane, set_clip_plane);
  MAKE_PROPERTY(visible, is_visible, set_visible);
  MAKE_PROPERTY(max_depth, get_max_depth, set_max_depth);
  MAKE_PROPERTY(open, is_open, set_open);

protected:
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

private:
  CPT(RenderState) get_last_pos_state();

  // This data is not cycled, for now.  We assume the collision traversal will
  // take place in App only.  Perhaps we will revisit this later.
  PortalMask _from_portal_mask;
  PortalMask _into_portal_mask;

  enum Flags {
    F_portal_geom = 0x0001,
    // Presently only 8 bits are written to the bam file.
  };
  int _flags;

  typedef pvector<LPoint3> Vertices;
  Vertices _vertices;

  NodePath _cell_in;  // This is the cell it resides in
  NodePath _cell_out;  // This is the cell it leads out to

  // enable plane clipping on this portal
  bool _clip_plane;
  PT(PlaneNode) _left_plane_node;
  PT(PlaneNode) _right_plane_node;
  PT(PlaneNode) _top_plane_node;
  PT(PlaneNode) _bottom_plane_node;
  CPT(RenderState) _clip_state;

  bool _visible;
  bool _open;
  int _max_depth;

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
    PandaNode::init_type();
    register_type(_type_handle, "PortalNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "portalNode.I"

#endif
