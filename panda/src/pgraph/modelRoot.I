// Filename: modelRoot.I
// Created by:  drose (16Mar02)
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
//     Function: ModelRoot::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelRoot::
ModelRoot(const string &name) :
  ModelNode(name),
  _fullpath(name),
  _timestamp(0),
  _reference(new ModelRoot::ModelReference)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelRoot::
ModelRoot(const Filename &fullpath, time_t timestamp) :
  ModelNode(fullpath.get_basename()),
  _fullpath(fullpath),
  _timestamp(timestamp),
  _reference(new ModelRoot::ModelReference)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::get_model_ref_count
//       Access: Published
//  Description: Returns the number of copies that exist of this
//               particular ModelRoot node.  Each time
//               ModelRoot::copy_subgraph() or make_copy() is called
//               (or some other copying mechanism, such as
//               NodePath.copy_to(), is used), this count will
//               increment by one in all copies; when one of the
//               copies is destructed, this count will decrement.
////////////////////////////////////////////////////////////////////
INLINE int ModelRoot::
get_model_ref_count() const {
  return _reference->get_ref_count();
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::get_fullpath
//       Access: Published
//  Description: Returns the full pathname of the model represented by
//               this node, as found on disk.  This is mainly useful
//               for reference purposes, but is also used to index the
//               ModelRoot into the ModelPool.
////////////////////////////////////////////////////////////////////
INLINE const Filename &ModelRoot::
get_fullpath() const {
  return _fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::set_fullpath
//       Access: Published
//  Description: Sets the full pathname of the model represented by
//               this node, as found on disk.  This is mainly useful
//               for reference purposes, but is also used to index the
//               ModelRoot into the ModelPool.
//
//               This is normally set automatically when a model is
//               loaded, and should not be set directly by the user.
//               If you change this on a loaded model, then
//               ModelPool::release_model() may fail.
////////////////////////////////////////////////////////////////////
INLINE void ModelRoot::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::get_timestamp
//       Access: Published
//  Description: Returns the timestamp of the file on disk that was
//               read for this model, at the time it was read, if it
//               is known.  Returns 0 if the timestamp is not known or
//               could not be provided.  This can be used as a quick
//               (but fallible) check to verify whether the file might
//               have changed since the model was read.
////////////////////////////////////////////////////////////////////
INLINE time_t ModelRoot::
get_timestamp() const {
  return _timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::set_timestamp
//       Access: Published
//  Description: Sets the timestamp of the file on disk that was read
//               for this model.  This is normally set automatically
//               when a model is loaded, and should not be set
//               directly by the user.
////////////////////////////////////////////////////////////////////
INLINE void ModelRoot::
set_timestamp(time_t timestamp) {
  _timestamp = timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::get_reference
//       Access: Published
//  Description: Returns the pointer that represents the object shared
//               between all copies of this ModelRoot.  Since there's
//               not much associated with this object other than a
//               reference count, normally there's not much reason to
//               get the pointer (though it may be compared
//               pointerwise with other ModelRoot objects).
////////////////////////////////////////////////////////////////////
INLINE ModelRoot::ModelReference *ModelRoot::
get_reference() const {
  return _reference;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::set_reference
//       Access: Published
//  Description: Changes the pointer that represents the object shared
//               between all copies of this ModelRoot.  This will
//               disassociate this ModelRoot from all of its copies.
//               Normally, there's no reason to do this.
////////////////////////////////////////////////////////////////////
INLINE void ModelRoot::
set_reference(ModelRoot::ModelReference *ref) {
  _reference = ref;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelRoot::
ModelRoot(const ModelRoot &copy) :
  ModelNode(copy),
  _fullpath(copy._fullpath),
  _timestamp(copy._timestamp),
  _reference(copy._reference)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::ModelReference::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelRoot::ModelReference::
ModelReference() {
}
