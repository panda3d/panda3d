// Filename: cullBinOcclusionTest.I
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


////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::Copy Constructor
//       Access: Protected
//  Description: This constructor is used in make_new().  It
//               constructs a new, empty CullBin for rendering the
//               next frame's data.  However, it preserves the
//               _prev_draw pointer so that there will be frame
//               continuity.
////////////////////////////////////////////////////////////////////
INLINE CullBinOcclusionTest::
CullBinOcclusionTest(const CullBinOcclusionTest &copy) :
  CullBin(copy),
  _draw_occlusion_pcollector(copy._draw_occlusion_pcollector)
{
  nassertv(_gsg->get_supports_occlusion_query());

  _num_objects = 0;
  _prev_draw = copy._prev_draw;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinOcclusionTest::
CullBinOcclusionTest(const string &name, GraphicsStateGuardianBase *gsg,
                     const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_occlusion_test, gsg, draw_region_pcollector),
  _draw_occlusion_pcollector(_draw_this_pcollector, "Occlusion")
{
  nassertv(_gsg->get_supports_occlusion_query());

  _num_objects = 0;
  _prev_draw = new PrevDrawData;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinOcclusionTest::ObjectData::
ObjectData(CullableObject *object, BoundingSphere *bounds) :
  _object(object),
  _bounds(bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::ObjectData::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinOcclusionTest::ObjectData::
ObjectData(const CullBinOcclusionTest::ObjectData &copy) :
  _object(copy._object),
  _bounds(copy._bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::ObjectData::Copy Assignment
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CullBinOcclusionTest::ObjectData::
operator = (const CullBinOcclusionTest::ObjectData &copy) {
  _object = copy._object;
  _bounds = copy._bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::VisibleGeom::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinOcclusionTest::VisibleGeom::
VisibleGeom(const Geom *geom, const TransformState *net_transform) :
  _geom(geom),
  _net_transform(net_transform),
  _bounds(geom->get_bounds())
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::VisibleGeom::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool CullBinOcclusionTest::VisibleGeom::
operator < (const CullBinOcclusionTest::VisibleGeom &other) const {
  if (_geom != other._geom) {
    return _geom < other._geom;
  }
  if (_net_transform != other._net_transform) {
    return _net_transform < other._net_transform;
  }
  return _bounds < other._bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::initial_assign
//       Access: Public
//  Description: Assigns the object to the node, in preparation for
//               calling group_objects().
////////////////////////////////////////////////////////////////////
INLINE void CullBinOcclusionTest::OctreeNode::
initial_assign(const CullBinOcclusionTest::ObjectData &object_data) {
  _objects.push_back(object_data);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::reassign
//       Access: Public
//  Description: After determining that the object bisects one of the
//               major planes of the node, reassigns it back to the
//               same node.
////////////////////////////////////////////////////////////////////
INLINE void CullBinOcclusionTest::OctreeNode::
reassign(const CullBinOcclusionTest::ObjectData &object_data) {
  if (object_data._bounds->get_radius() / _half_side >= octree_multiassign_ratio) {
    // The object is large enough to keep in this node.
    _objects.push_back(object_data);
  } else {
    multi_assign(object_data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinOcclusionTest::OctreeNode::assign_to_corner
//       Access: Private
//  Description: Assigns the object to the octree node in the
//               indicated corner.
////////////////////////////////////////////////////////////////////
INLINE void CullBinOcclusionTest::OctreeNode::
assign_to_corner(int index, const CullBinOcclusionTest::ObjectData &object_data) {
  nassertv(index >= 0 && index < 8);
  if (_corners[index] == NULL) {
    make_corner(index);
  }
  _corners[index]->initial_assign(object_data);
}
