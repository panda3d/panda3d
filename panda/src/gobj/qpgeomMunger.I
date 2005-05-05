// Filename: qpgeomMunger.I
// Created by:  drose (10Mar05)
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
//     Function: qpGeomMunger::is_registered
//       Access: Public
//  Description: Returns true if this munger has been registered,
//               false if it has not.  It may not be used for a Geom
//               until it has been registered, but once registered, it
//               may no longer be modified.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomMunger::
is_registered() const {
  return _is_registered;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::register_munger
//       Access: Public, Static
//  Description: Adds the indicated munger to the registry, if there
//               is not an equivalent munger already there; in either
//               case, returns the pointer to the equivalent munger
//               now in the registry.
//
//               This must be called before a munger may be used in a
//               Geom.  After this call, you should discard the
//               original pointer you passed in (which may or may not
//               now be invalid) and let its reference count decrement
//               normally; you should use only the returned value from
//               this point on.
////////////////////////////////////////////////////////////////////
INLINE PT(qpGeomMunger) qpGeomMunger::
register_munger(qpGeomMunger *munger) {
  return get_registry()->register_munger(munger);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::compare_to
//       Access: Public
//  Description: Provides an arbitrary ordering among all unique
//               GeomMungers, so we can store the essentially
//               different ones in a big set and throw away the rest.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomMunger::
compare_to(const qpGeomMunger &other) const {
  // First, we compare the types; if they are of different types then
  // they sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return compare_to_impl(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::geom_compare_to
//       Access: Public
//  Description: Compares two GeomMungers, considering only whether
//               they would produce a different answer to
//               munge_format(), munge_data(), or munge_geom().  (They
//               still might be different in other ways, but if they
//               would produce the same answer, this function consider
//               them to be the same.)
////////////////////////////////////////////////////////////////////
INLINE int qpGeomMunger::
geom_compare_to(const qpGeomMunger &other) const {
  // First, we compare the types; if they are of different types then
  // they sort differently.
  TypeHandle type = get_type();
  TypeHandle other_type = other.get_type();
  if (type != other_type) {
    return type.get_index() - other_type.get_index();
  }

  // We only call compare_to_impl() if they have the same type.
  return geom_compare_to_impl(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_format
//       Access: Public
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
//
//               If the GeomVertexAnimationSpec so indicates, then the
//               format will be chosen to convert CPU-based animation
//               tables to HW-based animation tables, reserving space
//               for the specified number of transforms per vertex.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexFormat) qpGeomMunger::
munge_format(const qpGeomVertexFormat *format,
             const qpGeomVertexAnimationSpec &animation) const {
  // We cast away the const pointer, because do_munge_format() needs
  // to update caches and stuff, but we trust it not to change any
  // user-definable parameters.
  return ((qpGeomMunger *)this)->do_munge_format(format, animation);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_data
//       Access: Public
//  Description: Given a source GeomVertexData, converts it if
//               necessary to the appropriate data for rendering.
////////////////////////////////////////////////////////////////////
INLINE CPT(qpGeomVertexData) qpGeomMunger::
munge_data(const qpGeomVertexData *data) const {
  // We cast away the const pointer, because do_munge_data() needs to
  // update caches and stuff, but we trust it not to change any
  // user-definable parameters.
  return ((qpGeomMunger *)this)->munge_data_impl(data);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::operator new
//       Access: Public
//  Description: This implementation of operator new isn't really
//               supposed to be called, but it's provided in case
//               someone screws up and doesn't define a more
//               appropriate operator new (that passes up a valid
//               _deleted_chain) for a derived class.  Having this
//               operator new that passes NULL for _deleted_chain will
//               at least allow us to detect the error.
////////////////////////////////////////////////////////////////////
INLINE void *qpGeomMunger::
operator new(size_t size) {
  return do_operator_new(size, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::operator delete
//       Access: Public
//  Description: This balances do_operator_new(), below, and adds the
//               deleted object to the stored deleted_chain, rather
//               than returning its memory to the heap.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomMunger::
operator delete(void *ptr) {
  qpGeomMunger *obj = (qpGeomMunger *)ptr;

#ifndef NDEBUG
  nassertd(obj->_deleted_chain != (qpGeomMunger **)NULL) {
    delete obj;
    return;
  }
#endif

  nassertv(obj->_next == (qpGeomMunger *)NULL);
  obj->_next = (*obj->_deleted_chain);
  (*obj->_deleted_chain) = obj;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::do_operator_new
//       Access: Protected, Static
//  Description: Intended to be called from a derived class's operator
//               new method, this allocates a new object from the
//               indicated deleted_chain, if there is an object
//               available there, or from the heap if not.
//
//               In either case, the indicated deleted_chain pointer
//               is cleverly stored on the new object, even before its
//               constructor is called, so that it can be returned
//               there when it is deleted.
////////////////////////////////////////////////////////////////////
INLINE void *qpGeomMunger::
do_operator_new(size_t size, qpGeomMunger **deleted_chain) {
  qpGeomMunger *obj;

#ifndef NDEBUG
  nassertd(deleted_chain != (qpGeomMunger **)NULL) {
    obj = (qpGeomMunger *)::operator new(size);
    obj->_deleted_chain = NULL;
    return obj;
  }
#endif
    
  if ((*deleted_chain) != (qpGeomMunger *)NULL) {
    obj = (*deleted_chain);
    (*deleted_chain) = (*deleted_chain)->_next;
  } else {
    obj = (qpGeomMunger *)::operator new(size);
  }
  obj->_deleted_chain = deleted_chain;

  return obj;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::get_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
INLINE qpGeomMunger::Registry *qpGeomMunger::
get_registry() {
  if (_registry == (Registry *)NULL) {
    make_registry();
  }
  return _registry;
}
