// Filename: cullBinHierarchicalZBuffer.I
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
//     Function: CullBinHierarchicalZBuffer::ObjectData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinHierarchicalZBuffer::ObjectData::
ObjectData(CullableObject *object, BoundingSphere *bounds) :
  _object(object),
  _bounds(bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::ObjectData::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinHierarchicalZBuffer::ObjectData::
ObjectData(const CullBinHierarchicalZBuffer::ObjectData &copy) :
  _object(copy._object),
  _bounds(copy._bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::ObjectData::Copy Assignment
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CullBinHierarchicalZBuffer::ObjectData::
operator = (const CullBinHierarchicalZBuffer::ObjectData &copy) {
  _object = copy._object;
  _bounds = copy._bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBinHierarchicalZBuffer::
CullBinHierarchicalZBuffer(const string &name, GraphicsStateGuardianBase *gsg) :
  CullBin(name, gsg),
  _draw_occlusion_pcollector(_draw_this_pcollector, "Occlusion")
{
  nassertv(_gsg->get_supports_occlusion_query());
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::initial_assign
//       Access: Public
//  Description: Assigns the object to the node, in preparation for
//               calling group_objects().
////////////////////////////////////////////////////////////////////
INLINE void CullBinHierarchicalZBuffer::OctreeNode::
initial_assign(const CullBinHierarchicalZBuffer::ObjectData &object_data) {
  _objects.push_back(object_data);
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::get_total_num_objects
//       Access: Public
//  Description: Returns the total number of objects in this node and
//               all nested nodes.
////////////////////////////////////////////////////////////////////
INLINE int CullBinHierarchicalZBuffer::OctreeNode::
get_total_num_objects() const {
  return _total_num_objects;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::reassign
//       Access: Public
//  Description: After determining that the object bisects one of the
//               major planes of the node, reassigns it back to the
//               same node.
////////////////////////////////////////////////////////////////////
INLINE void CullBinHierarchicalZBuffer::OctreeNode::
reassign(const CullBinHierarchicalZBuffer::ObjectData &object_data) {
  ++_total_num_objects;
  if (object_data._bounds->get_radius() / _half_side >= octree_multiassign_ratio) {
    // The object is large enough to keep in this node.
    //cerr << "    reassigning.\n";
    _objects.push_back(object_data);
  } else {
    //cerr << "    multidistributing.\n";
    multi_assign(object_data);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullBinHierarchicalZBuffer::OctreeNode::assign_to_corner
//       Access: Private
//  Description: Assigns the object to the octree node in the
//               indicated corner.
////////////////////////////////////////////////////////////////////
INLINE void CullBinHierarchicalZBuffer::OctreeNode::
assign_to_corner(int index, const CullBinHierarchicalZBuffer::ObjectData &object_data) {
  //cerr << "    Assigning to corner " << hex << index << dec << ".\n";
  nassertv(index >= 0 && index < 8);
  if (_corners[index] == NULL) {
    make_corner(index);
  }
  _corners[index]->initial_assign(object_data);
}
