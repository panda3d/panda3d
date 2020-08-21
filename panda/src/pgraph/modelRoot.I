/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modelRoot.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE ModelRoot::
ModelRoot(const std::string &name) :
  ModelNode(name),
  _fullpath(name),
  _timestamp(0),
  _reference(new ModelRoot::ModelReference)
{
}

/**
 *
 */
INLINE ModelRoot::
ModelRoot(const Filename &fullpath, time_t timestamp) :
  ModelNode(fullpath.get_basename()),
  _fullpath(fullpath),
  _timestamp(timestamp),
  _reference(new ModelRoot::ModelReference)
{
}

/**
 * Returns the number of copies that exist of this particular ModelRoot node.
 * Each time ModelRoot::copy_subgraph() or make_copy() is called (or some
 * other copying mechanism, such as NodePath.copy_to(), is used), this count
 * will increment by one in all copies; when one of the copies is destructed,
 * this count will decrement.
 */
INLINE int ModelRoot::
get_model_ref_count() const {
  return _reference->get_ref_count();
}

/**
 * Returns the full pathname of the model represented by this node, as found
 * on disk.  This is mainly useful for reference purposes, but is also used to
 * index the ModelRoot into the ModelPool.
 */
INLINE const Filename &ModelRoot::
get_fullpath() const {
  return _fullpath;
}

/**
 * Sets the full pathname of the model represented by this node, as found on
 * disk.  This is mainly useful for reference purposes, but is also used to
 * index the ModelRoot into the ModelPool.
 *
 * This is normally set automatically when a model is loaded, and should not
 * be set directly by the user.  If you change this on a loaded model, then
 * ModelPool::release_model() may fail.
 */
INLINE void ModelRoot::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

/**
 * Returns the timestamp of the file on disk that was read for this model, at
 * the time it was read, if it is known.  Returns 0 if the timestamp is not
 * known or could not be provided.  This can be used as a quick (but fallible)
 * check to verify whether the file might have changed since the model was
 * read.
 */
INLINE time_t ModelRoot::
get_timestamp() const {
  return _timestamp;
}

/**
 * Sets the timestamp of the file on disk that was read for this model.  This
 * is normally set automatically when a model is loaded, and should not be set
 * directly by the user.
 */
INLINE void ModelRoot::
set_timestamp(time_t timestamp) {
  _timestamp = timestamp;
}

/**
 * Returns the pointer that represents the object shared between all copies of
 * this ModelRoot.  Since there's not much associated with this object other
 * than a reference count, normally there's not much reason to get the pointer
 * (though it may be compared pointerwise with other ModelRoot objects).
 */
INLINE ModelRoot::ModelReference *ModelRoot::
get_reference() const {
  return _reference;
}

/**
 * Changes the pointer that represents the object shared between all copies of
 * this ModelRoot.  This will disassociate this ModelRoot from all of its
 * copies.  Normally, there's no reason to do this.
 */
INLINE void ModelRoot::
set_reference(ModelRoot::ModelReference *ref) {
  _reference = ref;
}

/**
 *
 */
INLINE ModelRoot::
ModelRoot(const ModelRoot &copy) :
  ModelNode(copy),
  _fullpath(copy._fullpath),
  _timestamp(copy._timestamp),
  _reference(copy._reference)
{
}

/**
 *
 */
INLINE ModelRoot::ModelReference::
ModelReference() {
}
