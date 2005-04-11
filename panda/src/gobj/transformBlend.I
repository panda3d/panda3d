// Filename: transformBlend.I
// Created by:  drose (24Mar05)
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
//     Function: TransformBlend::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
TransformBlend() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, float) {
  add_transform(transform0, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, float weight0,
               const VertexTransform *transform1, float weight1) {
  add_transform(transform0, weight0);
  add_transform(transform1, weight1);
  normalize_weights();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, float weight0,
               const VertexTransform *transform1, float weight1,
               const VertexTransform *transform2, float weight2) {
  add_transform(transform0, weight0);
  add_transform(transform1, weight1);
  add_transform(transform2, weight2);
  normalize_weights();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
TransformBlend(const VertexTransform *transform0, float weight0,
               const VertexTransform *transform1, float weight1,
               const VertexTransform *transform2, float weight2,
               const VertexTransform *transform3, float weight3) {
  add_transform(transform0, weight0);
  add_transform(transform1, weight1);
  add_transform(transform2, weight2);
  add_transform(transform3, weight3);
  normalize_weights();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
TransformBlend(const TransformBlend &copy) :
  _entries(copy._entries)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
operator = (const TransformBlend &copy) {
  _entries = copy._entries;
  clear_result();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::
~TransformBlend() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::operator <
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TransformBlend::
operator < (const TransformBlend &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::operator ==
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TransformBlend::
operator == (const TransformBlend &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::operator !=
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool TransformBlend::
operator != (const TransformBlend &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::get_num_transforms
//       Access: Published
//  Description: Returns the number of transforms stored in the blend
//               object.
////////////////////////////////////////////////////////////////////
INLINE int TransformBlend::
get_num_transforms() const {
  return _entries.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::get_transform
//       Access: Published
//  Description: Returns the nth transform stored in the blend
//               object.
////////////////////////////////////////////////////////////////////
INLINE const VertexTransform *TransformBlend::
get_transform(int n) const {
  nassertr(n >= 0 && n < (int)_entries.size(), NULL);
  return _entries[n]._transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::get_weight
//       Access: Published
//  Description: Returns the weight associated with the nth transform
//               stored in the blend object.
////////////////////////////////////////////////////////////////////
INLINE float TransformBlend::
get_weight(int n) const {
  nassertr(n >= 0 && n < (int)_entries.size(), 0.0f);
  return _entries[n]._weight;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::set_transform
//       Access: Published
//  Description: Replaces the nth transform stored in the blend
//               object.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
set_transform(int n, const VertexTransform *transform) {
  nassertv(n >= 0 && n < (int)_entries.size());
  _entries[n]._transform = transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::set_weight
//       Access: Published
//  Description: Replaces the weight associated with the nth transform
//               stored in the blend object.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
set_weight(int n, float weight) {
  nassertv(n >= 0 && n < (int)_entries.size());
  _entries[n]._weight = weight;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::update_blend
//       Access: Published
//  Description: Recomputes the internal representation of the blend
//               value, if necessary.  You should call this before
//               calling get_blend() or transform_point().
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
update_blend() const {
  CDReader cdata(_cycler);
  if (cdata->_global_modified != VertexTransform::get_global_modified()) {
    CDWriter cdataw(((TransformBlend *)this)->_cycler, cdata);
    ((TransformBlend *)this)->recompute_result(cdataw);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::get_blend
//       Access: Published
//  Description: Returns the current value of the blend, based on the
//               current value of all of the nested transform objects
//               and their associated weights.
//
//               You should call update_blend() to ensure that the
//               cache is up-to-date before calling this.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
get_blend(LMatrix4f &result) const {
  CDReader cdata(_cycler);
  nassertv(cdata->_global_modified == VertexTransform::get_global_modified());
  result = cdata->_result;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::transform_point
//       Access: Published
//  Description: Transforms the indicated point by the blend matrix.
//
//               You should call update_blend() to ensure that the
//               cache is up-to-date before calling this.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
transform_point(LPoint4f &point) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler);
    nassertv(cdata->_global_modified == VertexTransform::get_global_modified());
    point = point * cdata->_result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::transform_point
//       Access: Published
//  Description: Transforms the indicated point by the blend matrix.
//
//               You should call update_blend() to ensure that the
//               cache is up-to-date before calling this.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
transform_point(LPoint3f &point) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler);
    nassertv(cdata->_global_modified == VertexTransform::get_global_modified());
    point = point * cdata->_result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::transform_vector
//       Access: Published
//  Description: Transforms the indicated vector by the blend matrix.
//
//               You should call update_blend() to ensure that the
//               cache is up-to-date before calling this.
////////////////////////////////////////////////////////////////////
INLINE void TransformBlend::
transform_vector(LVector3f &vector) const {
  if (!_entries.empty()) {
    CDReader cdata(_cycler);
    nassertv(cdata->_global_modified == VertexTransform::get_global_modified());
    vector = vector * cdata->_result;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::get_modified
//       Access: Published
//  Description: Returns a counter which is guaranteed to increment at
//               least as often as the result of get_blend() changes.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq TransformBlend::
get_modified() const {
  CDReader cdata(_cycler);
  if (cdata->_global_modified != VertexTransform::get_global_modified()) {
    CDWriter cdataw(((TransformBlend *)this)->_cycler, cdata);
    ((TransformBlend *)this)->recompute_result(cdataw);
    return cdataw->_modified;
  } else {
    return cdata->_modified;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::TransformEntry::operator <
//       Access: Public
//  Description: Provides an ordering of TransformEntries by the
//               VertexTransform pointer only, so we can easily look
//               up in the set to see if a particular transform
//               exists.
////////////////////////////////////////////////////////////////////
INLINE bool TransformBlend::TransformEntry::
operator < (const TransformBlend::TransformEntry &other) const {
  return _transform < other._transform;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::CData::
CData() :
  _result(LMatrix4f::ident_mat())
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformBlend::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TransformBlend::CData::
CData(const TransformBlend::CData &copy) :
  _result(copy._result),
  _modified(copy._modified),
  _global_modified(copy._global_modified)
{
}

INLINE ostream &
operator << (ostream &out, const TransformBlend &obj) {
  obj.output(out);
  return out;
}
