// Filename: portalNode.h
// Created by: masad (13May04)
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

#ifndef PORTALNODE_H
#define PORTALNODE_H

#include "pandabase.h"

#include "portalMask.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : PortalNode 
//       Description : A node in the scene graph that can hold a 
//                     Portal Polygon, which is a rectangle. Other 
//                     types of polygons are not supported for
//                     now. It also holds a PT(PandaNode) Cell that 
//                     this portal is connected to
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PortalNode : public PandaNode {
PUBLISHED:
  PortalNode(const string &name);
  PortalNode(const string &name, LPoint3f pos, float scale=10.0);

protected:
  PortalNode(const PortalNode &copy);

public:
  virtual ~PortalNode();
  virtual PandaNode *make_copy() const;
  virtual bool preserve_name() const;
  virtual void xform(const LMatrix4f &mat);
  virtual PandaNode *combine_with(PandaNode *other); 

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_portal_mask(PortalMask mask);
  INLINE void set_from_portal_mask(PortalMask mask);
  INLINE void set_into_portal_mask(PortalMask mask);
  INLINE PortalMask get_from_portal_mask() const;
  INLINE PortalMask get_into_portal_mask() const;

  INLINE void set_portal_geom(bool flag);
  INLINE bool get_portal_geom() const;

  INLINE void clear_vertices();
  INLINE void add_vertex(const LPoint3f &vertex);

  INLINE int get_num_vertices() const;
  INLINE const LPoint3f &get_vertex(int n) const;

  INLINE void set_cell_in(const NodePath &cell);
  INLINE NodePath get_cell_in() const;

  INLINE void set_cell_out(const NodePath &cell);
  INLINE NodePath get_cell_out() const;

  INLINE void set_visible(bool value);
  INLINE bool is_visible();

  INLINE void set_open(bool value);
  INLINE bool is_open();

  //  void draw () const;

protected:
  virtual BoundingVolume *recompute_bound();
  virtual BoundingVolume *recompute_internal_bound();

private:
  CPT(RenderState) get_last_pos_state();

  // This data is not cycled, for now.  We assume the collision
  // traversal will take place in App only.  Perhaps we will revisit
  // this later.
  PortalMask _from_portal_mask;
  PortalMask _into_portal_mask;

  enum Flags {
    F_portal_geom = 0x0001,
    // Presently only 8 bits are written to the bam file.
  };
  int _flags;

  typedef pvector<LPoint3f> Vertices;
  Vertices _vertices;

  NodePath _cell_in;  // This is the cell it resides in
  NodePath _cell_out;  // This is the cell it leads out to

  bool _visible;
  bool _open;

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
