// Filename: qpgeomMunger.cxx
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

#include "qpgeomMunger.h"
#include "qpgeomCacheManager.h"
#include "mutexHolder.h"
#include "pStatTimer.h"

qpGeomMunger::Registry *qpGeomMunger::_registry = NULL;
TypeHandle qpGeomMunger::_type_handle;

PStatCollector qpGeomMunger::_munge_pcollector("*:Munge");

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomMunger::
qpGeomMunger(const GraphicsStateGuardianBase *, const RenderState *) :
  _is_registered(false)
{
#ifndef NDEBUG
  _registered_key = get_registry()->_mungers.end();
  _next = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomMunger::
qpGeomMunger(const qpGeomMunger &copy) :
  _is_registered(false)
{
#ifndef NDEBUG
  _registered_key = get_registry()->_mungers.end();
  _next = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomMunger::
operator = (const qpGeomMunger &copy) {
  nassertv(!_is_registered);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomMunger::
~qpGeomMunger() {
  if (is_registered()) {
    get_registry()->unregister_munger(this);
  }
  nassertv(_formats_by_animation.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::remove_data
//       Access: Public
//  Description: Removes a prepared GeomVertexData from the cache.
////////////////////////////////////////////////////////////////////
void qpGeomMunger::
remove_data(const qpGeomVertexData *data) {
  // If this assertion is triggered, maybe we accidentally deleted a
  // GeomVertexData while we were in the process of unregistering,
  // causing a recursive re-entry.
  nassertv(_is_registered);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::do_munge_format
//       Access: Protected
//  Description: The protected implementation of munge_format().  This
//               exists just to cast away the const pointer.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexFormat) qpGeomMunger::
do_munge_format(const qpGeomVertexFormat *format, 
                const qpGeomVertexAnimationSpec &animation) {
  nassertr(_is_registered, NULL);
  nassertr(format->is_registered(), NULL);

  Formats &formats = _formats_by_animation[animation];

  Formats::iterator fi;
  fi = formats.find(format);
  if (fi != formats.end()) {
    // This format was previously munged, so the answer will be the
    // same.
    return (*fi).second;
  }

  // We have to munge this format for the first time.
  CPT(qpGeomVertexFormat) derived_format = munge_format_impl(format, animation);
  nassertr(derived_format->is_registered(), NULL);

  // Store the answer in the map, so we can quickly get it next time.
  bool inserted = formats.insert(Formats::value_type(format, derived_format)).second;
  nassertr(inserted, NULL);

  return derived_format;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexFormat) qpGeomMunger::
munge_format_impl(const qpGeomVertexFormat *orig, const qpGeomVertexAnimationSpec &) {
  return orig;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_data_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexData, converts it as
//               necessary for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) qpGeomMunger::
munge_data_impl(const qpGeomVertexData *data) {
  nassertr(_is_registered, NULL);

  CPT(qpGeomVertexFormat) orig_format = data->get_format();
  CPT(qpGeomVertexFormat) new_format = 
    munge_format(orig_format, orig_format->get_animation());

  if (new_format == orig_format) {
    // Trivial case.
    return data;
  }

  return data->convert_to(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::munge_geom_impl
//       Access: Protected, Virtual
//  Description: Converts a Geom and/or its data as necessary.
////////////////////////////////////////////////////////////////////
bool qpGeomMunger::
munge_geom_impl(CPT(qpGeom) &, CPT(qpGeomVertexData) &) {
  // The default implementation does nothing (the work has already
  // been done in munge_format_impl() and munge_data_impl()).
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int qpGeomMunger::
compare_to_impl(const qpGeomMunger *other) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Compares two GeomMungers, considering only whether
//               they would produce a different answer to
//               munge_format(), munge_data(), or munge_geom().  (They
//               still might be different in other ways, but if they
//               would produce the same answer, this function consider
//               them to be the same.)
////////////////////////////////////////////////////////////////////
int qpGeomMunger::
geom_compare_to_impl(const qpGeomMunger *other) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::make_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
void qpGeomMunger::
make_registry() {
  if (_registry == (Registry *)NULL) {
    _registry = new Registry;
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::do_register
//       Access: Private
//  Description: Called internally when the munger is registered.
////////////////////////////////////////////////////////////////////
void qpGeomMunger::
do_register() {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "GeomMunger::do_register(): " << (void *)this << "\n";
  }
  nassertv(!_is_registered);
  nassertv(_formats_by_animation.empty());

  // Tell the cache manager to hang on to this new GeomMunger, so we
  // don't waste our time re-registering the same GeomMunger over and
  // over again.
  CacheEntry *entry = new CacheEntry;
  entry->_munger = this;
  entry->record();

  _is_registered = true;
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::do_unregister
//       Access: Private
//  Description: Called internally when the munger is unregistered.
////////////////////////////////////////////////////////////////////
void qpGeomMunger::
do_unregister() {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "GeomMunger::do_unregister(): " << (void *)this << "\n";
  }
  nassertv(_is_registered);
  _is_registered = false;

  // Unregistering means we should blow away the cache.
  _formats_by_animation.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::CacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomMunger::CacheEntry::
output(ostream &out) const {
  out << "munger " << _munger;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Registry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomMunger::Registry::
Registry() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Registry::register_munger
//       Access: Public
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
CPT(qpGeomMunger) qpGeomMunger::Registry::
register_munger(qpGeomMunger *munger) {
  if (munger->is_registered()) {
    return munger;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has
  // a zero reference count and is not added into the map below, it
  // will be automatically deleted when this function returns.
  PT(qpGeomMunger) pt_munger = munger;

  Mungers::iterator mi = _mungers.insert(munger).first;
  qpGeomMunger *new_munger = (*mi);
  if (!new_munger->is_registered()) {
    new_munger->_registered_key = mi;
    new_munger->do_register();
  }

  return new_munger;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomMunger::Registry::unregister_munger
//       Access: Public
//  Description: Removes the indicated munger from the registry.
//               Normally this should not be done until the munger is
//               destructing.
////////////////////////////////////////////////////////////////////
void qpGeomMunger::Registry::
unregister_munger(qpGeomMunger *munger) {
  nassertv(munger->is_registered());
  nassertv(munger->_registered_key != _mungers.end());
  _mungers.erase(munger->_registered_key);
  munger->_registered_key = _mungers.end();
  munger->do_unregister();
}
