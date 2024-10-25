/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file instanceList.I
 * @author rdb
 * @date 2019-03-10
 */

/**
 * Initializes an instance with the identity transform.
 */
INLINE InstanceList::Instance::
Instance() : _transform(Transform::make_identity()) {
}

/**
 * Initializes an instance with the given transformation.
 */
INLINE InstanceList::Instance::
Instance(const Transform &transform) : _transform(transform) {
}

/**
 *
 */
INLINE LPoint3 InstanceList::Instance::
get_pos() const {
  return _transform.get_pos();
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_pos(const LPoint3 &pos) {
  _transform.set_pos(pos);
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _transform.set_pos(LPoint3(x, y, z));
}

/**
 *
 */
INLINE LVecBase3 InstanceList::Instance::
get_hpr() const {
  return _transform.get_hpr();
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_hpr(const LVecBase3 &hpr) {
  _transform.set_hpr(hpr);
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  _transform.set_hpr(LVecBase3(h, p, r));
}

/**
 *
 */
INLINE LQuaternion InstanceList::Instance::
get_quat() const {
  return _transform.get_quat();
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_quat(const LQuaternion &quat) {
  _transform.set_quat(quat);
}

/**
 *
 */
INLINE LVecBase3 InstanceList::Instance::
get_scale() const {
  return _transform.get_scale();
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_scale(const LVecBase3 &scale) {
  _transform.set_scale(scale);
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_scale(PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz) {
  _transform.set_scale(LVecBase3(sx, sy, sz));
}

/**
 *
 */
INLINE LMatrix4 InstanceList::Instance::
get_mat() const {
  return _transform.get_mat();
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_mat(const LMatrix4 &mat) {
  _transform = Transform::make_mat(mat);
}

/**
 *
 */
INLINE Transform InstanceList::Instance::
get_transform() const {
  return _transform;
}

/**
 *
 */
INLINE void InstanceList::Instance::
set_transform(const Transform &transform) {
  _transform = transform;
}

/**
 * Adds a new instance with the indicated transformation to the list.
 */
INLINE void InstanceList::
append(InstanceList::Instance instance) {
  _instances.push_back(std::move(instance));
  _cached_array.clear();
}

/**
 * Adds a new instance with the indicated transformation to the list.
 */
INLINE void InstanceList::
append(const Transform &transform) {
  _instances.push_back(Instance(transform));
  _cached_array.clear();
}

/**
 * Adds a new instance with the indicated transformation to the list.
 */
INLINE void InstanceList::
append(const TransformState *transform) {
  _instances.push_back(Instance(Transform::make_mat(transform->get_mat())));
  _cached_array.clear();
}

/**
 * Adds a new instance with the indicated transformation to the list.
 */
INLINE void InstanceList::
append(const LPoint3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale) {

  append(Transform::make_pos_hpr_scale(pos, hpr, scale));
}

/**
 * Adds a new instance with the indicated transformation to the list.
 */
INLINE void InstanceList::
append(const LPoint3 &pos, const LQuaternion &quat, const LVecBase3 &scale) {

  append(Transform::make_pos_quat_scale(pos, quat, scale));
}

/**
 * Returns the total number of instances in the list.
 */
INLINE size_t InstanceList::
size() const {
  return _instances.size();
}

/**
 * Returns the nth instance in the list.
 */
INLINE const InstanceList::Instance &InstanceList::
operator [] (size_t n) const {
  return _instances[n];
}

/**
 * Returns the nth instance in the list.
 */
INLINE InstanceList::Instance &InstanceList::
operator [] (size_t n) {
  _cached_array.clear();
  return _instances[n];
}

/**
 * Empties the instance list.
 */
INLINE void InstanceList::
clear() {
  _instances.clear();
  _cached_array.clear();
}

/**
 * Reserves space for the given number of instances.
 */
INLINE void InstanceList::
reserve(size_t n) {
  _instances.reserve(n);
}

/**
 * Returns true if the InstanceList is empty.
 */
INLINE bool InstanceList::
empty() const {
  return _instances.empty();
}

/**
 * Returns an iterator to the beginning of the list.
 */
INLINE InstanceList::iterator InstanceList::
begin() {
  return _instances.begin();
}

/**
 * Returns a const_iterator to the beginning of the list.
 */
INLINE InstanceList::const_iterator InstanceList::
begin() const {
  return _instances.begin();
}

/**
 * Returns a const_iterator to the beginning of the list.
 */
INLINE InstanceList::const_iterator InstanceList::
cbegin() const {
  return _instances.cbegin();
}

/**
 * Returns an iterator to the end of the list.
 */
INLINE InstanceList::iterator InstanceList::
end() {
  return _instances.end();
}

/**
 * Returns a const_iterator to the end of the list.
 */
INLINE InstanceList::const_iterator InstanceList::
end() const {
  return _instances.end();
}

/**
 * Returns a const_iterator to the end of the list.
 */
INLINE InstanceList::const_iterator InstanceList::
cend() const {
  return _instances.cend();
}
