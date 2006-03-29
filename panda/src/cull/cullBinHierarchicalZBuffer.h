// Filename: cullBinHierarchicalZBuffer.h
// Created by:  drose (24Mar06)
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

#ifndef CULLBINHIERARCHICALZBUFFER_H
#define CULLBINHIERARCHICALZBUFFER_H

#include "pandabase.h"

#include "cullBin.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pointerTo.h"
#include "boundingSphere.h"
#include "config_cull.h"
#include "occlusionQueryContext.h"
#include "pdeque.h"
#include "pStatCollector.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinHierarchicalZBuffer
// Description : This cull bin uses a hierarchical Z-buffer algorithm
//               to attempt to further eliminate geometry that is
//               obscured behind walls, etc.  It imposes some
//               significant overhead over most of the other kinds of
//               CullBins, and requires additional support from the
//               graphics card, so it is most appropriate for scenes
//               in which there might be a large amount of geometry,
//               within the viewing frustum, but obscured behind large
//               objects or walls.
//
//               This code is still experimental.  Use with caution.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBinHierarchicalZBuffer : public CullBin {
public:
  INLINE CullBinHierarchicalZBuffer(const string &name, GraphicsStateGuardianBase *gsg);
  virtual ~CullBinHierarchicalZBuffer();

  static CullBin *make_bin(const string &name, GraphicsStateGuardianBase *gsg);
  virtual PT(CullBin) make_next() const;

  virtual void add_object(CullableObject *object);
  virtual void finish_cull();
  virtual void draw();

private:
  void draw_next();

  static CPT(Geom) get_octree_solid_test();
  static CPT(Geom) get_octree_wireframe_viz();

  static CPT(RenderState) get_octree_solid_test_state();

  // We keep the original list of CullableObjects here in the bin
  // class, so we can delete them on destruction.  This allows us to
  // duplicate pointers between different OctreeNodes.
  typedef pvector<CullableObject *> ObjectPointers;
  ObjectPointers _object_pointers;

  class ObjectData {
  public:
    INLINE ObjectData(CullableObject *object, BoundingSphere *bounds);
    INLINE ObjectData(const ObjectData &copy);
    INLINE void operator = (const ObjectData &copy);
    
    CullableObject *_object;
    PT(BoundingSphere) _bounds;
  };

  typedef pvector<ObjectData> Objects;

  enum OctreeCorners {
    OC_x   = 0x01,
    OC_y   = 0x02,
    OC_z   = 0x04,
  };

  class OctreeNode {
  public:
    OctreeNode();
    OctreeNode(float mid_x, float mid_y, float mid_z, float half_side);
    ~OctreeNode();

    void make_initial_bounds();
    void group_objects();
    PT(OcclusionQueryContext) occlusion_test(CullBinHierarchicalZBuffer &bin);
    void draw(CullBinHierarchicalZBuffer &bin);
    void draw_wireframe(CullBinHierarchicalZBuffer &bin);

    INLINE void initial_assign(const ObjectData &object_data);

    INLINE int get_total_num_objects() const;

  private:
    INLINE void reassign(const ObjectData &object_data);
    INLINE void assign_to_corner(int index, const ObjectData &object_data);
    void multi_assign(const ObjectData &object_data);
    void make_corner(int index);

    int _total_num_objects;
    Objects _objects;
    OctreeNode *_corners[8];
    LPoint3f _mid;
    float _half_side;
  };

  OctreeNode _root;

  // During draw(), we maintain a list of OctreeNodes that have been
  // tested and have yet to pass the occlusion query and be drawn (or
  // fail and be omitted).
  class PendingNode {
  public:
    OctreeNode *_octree_node;
    PT(OcclusionQueryContext) _query;
  };
  typedef pdeque<PendingNode> PendingNodes;
  PendingNodes _pending_nodes;

  PStatCollector _draw_occlusion_pcollector;
  static PStatCollector _wait_occlusion_pcollector;
  static PStatCollector _geoms_occluded_pcollector;

  static PT(Geom) _octree_solid_test;
  static PT(Geom) _octree_wireframe_viz;
  static CPT(RenderState) _octree_solid_test_state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinHierarchicalZBuffer",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class OctreeNode;
};

#include "cullBinHierarchicalZBuffer.I"

#endif


  
