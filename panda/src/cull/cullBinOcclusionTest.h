// Filename: cullBinOcclusionTest.h
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

#ifndef CULLBINOCCLUSIONTEST_H
#define CULLBINOCCLUSIONTEST_H

#include "pandabase.h"

#include "cullBin.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pointerTo.h"
#include "boundingSphere.h"
#include "config_cull.h"
#include "occlusionQueryContext.h"
#include "pStatCollector.h"
#include "pdeque.h"
#include "ordered_vector.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinOcclusionTest
// Description : This cull bin uses hardware-supported occlusion tests
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
class EXPCL_PANDA CullBinOcclusionTest : public CullBin {
protected:
  INLINE CullBinOcclusionTest(const CullBinOcclusionTest &copy);
public:
  INLINE CullBinOcclusionTest(const string &name, GraphicsStateGuardianBase *gsg);
  virtual ~CullBinOcclusionTest();

  static CullBin *make_bin(const string &name, GraphicsStateGuardianBase *gsg);
  virtual PT(CullBin) make_next() const;

  virtual void add_object(CullableObject *object);
  virtual void finish_cull(SceneSetup *scene_setup);
  virtual void draw(Thread *current_thread);

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

  static const LPoint3f _corner_points[8];

  // This class is used to build a table of Geoms (and their
  // associated net transform) that we have determined to be visible
  // during the draw pass.  This will be useful information for next
  // frame's draw.
  class VisibleGeom {
  public:
    INLINE VisibleGeom(const Geom *geom, const TransformState *net_transform);
    INLINE bool operator < (const VisibleGeom &other) const;

    CPT(Geom) _geom;
    CPT(TransformState) _net_transform;
    CPT(BoundingVolume) _bounds;
  };
  typedef ov_set<VisibleGeom> VisibleGeoms;

  class OctreeNode {
  public:
    OctreeNode();
    OctreeNode(float mid_x, float mid_y, float mid_z, float half_side);
    ~OctreeNode();

    void make_initial_bounds();
    void group_objects();
    void compute_distance(const LMatrix4f &world_mat,
                          CullBinOcclusionTest &bin);

    PT(OcclusionQueryContext) occlusion_test(CullBinOcclusionTest &bin,
                                             Thread *current_thread);
    int draw_previous(CullBinOcclusionTest &bin, Thread *current_thread);
    int draw(CullBinOcclusionTest &bin, Thread *current_thread);
    void draw_wireframe(CullBinOcclusionTest &bin, Thread *current_thread);
    void record_visible_geoms(VisibleGeoms &visible_geoms);
    INLINE void initial_assign(const ObjectData &object_data);

    void output(ostream &out) const;

  private:
    INLINE void reassign(const ObjectData &object_data);
    INLINE void assign_to_corner(int index, const ObjectData &object_data);
    void multi_assign(const ObjectData &object_data);
    void make_corner(int index);
    LPoint3f get_corner_point(int index);

  private:
    Objects _objects;
    OctreeNode *_corners[8];
    LPoint3f _mid;
    float _half_side;
    float _distance;
    bool _is_visible;
  };

  OctreeNode _root;
  int _num_objects;
  int _corners_front_to_back[8];
  float _near_distance;

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

  // The pointer to this class is preserved from one frame to the next
  // (unlike the CullBin itself, which is recreated anew each frame).
  // It keeps the data from the previous draw operation.
  class PrevDrawData : public ReferenceCount {
  public:
    VisibleGeoms _visible_geoms;
    Mutex _visible_lock;
  };
  PT(PrevDrawData) _prev_draw;

  PStatCollector _draw_occlusion_pcollector;
  static PStatCollector _wait_occlusion_pcollector;
  static PStatCollector _occlusion_previous_pcollector;
  static PStatCollector _occlusion_passed_pcollector;
  static PStatCollector _occlusion_failed_pcollector;

  static PT(Geom) _octree_solid_test;
  static PT(Geom) _octree_wireframe_viz;
  static CPT(RenderState) _octree_solid_test_state;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinOcclusionTest",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class OctreeNode;
  friend class GraphicsEngine;

  friend ostream &operator << (ostream &out, OctreeNode &node);
};

INLINE ostream &
operator << (ostream &out, CullBinOcclusionTest::OctreeNode &node) {
  node.output(out);
  return out;
}

#include "cullBinOcclusionTest.I"

#endif


  
