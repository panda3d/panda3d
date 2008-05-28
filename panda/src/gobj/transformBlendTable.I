// Filename: transformBlendTable.I
// Created by:  drose (24Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::get_num_blends
//       Access: Published
//  Description: Returns the total number of different blend
//               combinations in the table.
////////////////////////////////////////////////////////////////////
INLINE int TransformBlendTable::
get_num_blends() const {
  return _blends.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::get_blend
//       Access: Published
//  Description: Returns the nth blend in the table.
////////////////////////////////////////////////////////////////////
INLINE const TransformBlend &TransformBlendTable::
get_blend(int n) const {
  nassertr(n >= 0 && n < (int)_blends.size(), _blends[0]);
  return _blends[n];
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::get_modified
//       Access: Published
//  Description: Returns a counter which is guaranteed to increment at
//               least when any TransformBlends within the table
//               have changed.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq TransformBlendTable::
get_modified(Thread *current_thread) const {
  CDLockedReader cdata(_cycler);
  if (cdata->_global_modified != VertexTransform::get_global_modified(current_thread)) {
    CDWriter cdataw(((TransformBlendTable *)this)->_cycler, cdata, false);
    ((TransformBlendTable *)this)->recompute_modified(cdataw, current_thread);
    return cdataw->_modified;
  } else {
    return cdata->_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::get_num_transforms
//       Access: Published
//  Description: Returns the number of unique VertexTransform objects
//               represented in the table.  This will correspond to
//               the size of the TransformTable object that would
//               represent the same table.  This is also the same
//               limit reflected by
//               GraphicsStateGuardian::get_max_vertex_transform_indices().
////////////////////////////////////////////////////////////////////
INLINE int TransformBlendTable::
get_num_transforms() const {
  consider_rebuild_index();

  if (_num_transforms < 0) {
    // Even if our index is otherwise accurate, we might have recently
    // added a blend or two, which would necessitate recomputing this
    // value--which means we need to rebuild the index.
    ((TransformBlendTable *)this)->rebuild_index();
  }

  return _num_transforms;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::get_max_simultaneous_transforms
//       Access: Published
//  Description: Returns the maximum number of unique VertexTransform
//               objects that are applied to any one vertex
//               simultaneously.  This is the same limit reflected by
//               GraphicsStateGuardian::get_max_vertex_transforms().
////////////////////////////////////////////////////////////////////
INLINE int TransformBlendTable::
get_max_simultaneous_transforms() const {
  consider_rebuild_index();
  return _max_simultaneous_transforms;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::set_rows
//       Access: Published
//  Description: Specifies the subset of rows (vertices) in the
//               associated GeomVertexData that this
//               TransformBlendTable actually affects.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlendTable::
set_rows(const SparseArray &rows) {
  _rows = rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::get_rows
//       Access: Published
//  Description: Returns the subset of rows (vertices) in the
//               associated GeomVertexData that this
//               TransformBlendTable actually affects.
////////////////////////////////////////////////////////////////////
INLINE const SparseArray &TransformBlendTable::
get_rows() const {
  return _rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::modify_rows
//       Access: Published
//  Description: Returns a modifiable reference to the SparseArray
//               that specifies the subset of rows (vertices) in the
//               associated GeomVertexData that this
//               TransformBlendTable actually affects.
////////////////////////////////////////////////////////////////////
INLINE SparseArray &TransformBlendTable::
modify_rows() {
  return _rows;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::consider_rebuild_index
//       Access: Private
//  Description: Calls rebuild_index() if the index needs to be
//               rebuilt.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlendTable::
consider_rebuild_index() const {
  if (_blend_index.empty()) {
    ((TransformBlendTable *)this)->rebuild_index();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformBlendTable::CData::
CData() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlendTable::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformBlendTable::CData::
CData(const TransformBlendTable::CData &copy) :
  _modified(copy._modified),
  _global_modified(copy._global_modified)
{
}
