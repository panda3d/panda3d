// Filename: geomMunger.cxx
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

#include "geomMunger.h"
#include "geomCacheManager.h"
#include "mutexHolder.h"
#include "pStatTimer.h"

GeomMunger::Registry *GeomMunger::_registry = NULL;
TypeHandle GeomMunger::_type_handle;

PStatCollector GeomMunger::_munge_pcollector("*:Munge");

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomMunger::
GeomMunger() :
  _is_registered(false)
{
#ifndef NDEBUG
  _registered_key = get_registry()->_mungers.end();
  _next = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomMunger::
GeomMunger(const GeomMunger &copy) :
  _is_registered(false)
{
#ifndef NDEBUG
  _registered_key = get_registry()->_mungers.end();
  _next = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomMunger::
operator = (const GeomMunger &copy) {
  nassertv(!_is_registered);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomMunger::
~GeomMunger() {
  if (is_registered()) {
    get_registry()->unregister_munger(this);
  }
  nassertv(_formats_by_animation.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::remove_data
//       Access: Public
//  Description: Removes a prepared GeomVertexData from the cache.
////////////////////////////////////////////////////////////////////
void GeomMunger::
remove_data(const GeomVertexData *data) {
  // If this assertion is triggered, maybe we accidentally deleted a
  // GeomVertexData while we were in the process of unregistering,
  // causing a recursive re-entry.
  nassertv(_is_registered);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::munge_geom
//       Access: Published
//  Description: Applies the indicated munger to the geom and its
//               data, and returns a (possibly different) geom and
//               data, according to the munger's whim.  
//
//               The assumption is that for a particular geom and a
//               particular munger, the result will always be the
//               same; so this result may be cached.
////////////////////////////////////////////////////////////////////
void GeomMunger::
munge_geom(CPT(Geom) &geom, CPT(GeomVertexData) &data) {
  CPT(GeomVertexData) source_data = data;

  // Look up the munger in the geom's cache--maybe we've recently
  // applied it.
  {
    Geom::CDReader cdata(geom->_cycler);
    Geom::CacheEntry temp_entry(source_data, this);
    temp_entry.local_object();
    Geom::Cache::const_iterator ci = cdata->_cache.find(&temp_entry);
    if (ci != cdata->_cache.end()) {
      Geom::CacheEntry *entry = (*ci);

      if (geom->get_modified() <= entry->_geom_result->get_modified() &&
          data->get_modified() <= entry->_data_result->get_modified()) {
        // The cache entry is still good; use it.

        // Record a cache hit, so this element will stay in the cache a
        // while longer.
        entry->refresh();
        geom = entry->_geom_result;
        data = entry->_data_result;
        return;
      }

      // The cache entry is stale, remove it.
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Cache entry " << *entry << " is stale, removing.\n";
      }
      entry->erase();
      Geom::CDWriter cdataw(((Geom *)geom.p())->_cycler, cdata);
      cdataw->_cache.erase(entry);
    }
  }

  // Ok, invoke the munger.
  PStatTimer timer(_munge_pcollector);

  CPT(Geom) orig_geom = geom;
  data = munge_data(data);
  munge_geom_impl(geom, data);

  {
    // Record the new result in the cache.
    Geom::CacheEntry *entry;
    {
      Geom::CDWriter cdata(((Geom *)orig_geom.p())->_cycler);
      entry = new Geom::CacheEntry((Geom *)orig_geom.p(), source_data, this,
                                     geom, data);
      bool inserted = cdata->_cache.insert(entry).second;
      nassertv(inserted);
    }
    
    // And tell the cache manager about the new entry.  (It might
    // immediately request a delete from the cache of the thing we
    // just added.)
    entry->record();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::do_munge_format
//       Access: Protected
//  Description: The protected implementation of munge_format().  This
//               exists just to cast away the const pointer.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) GeomMunger::
do_munge_format(const GeomVertexFormat *format, 
                const GeomVertexAnimationSpec &animation) {
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
  CPT(GeomVertexFormat) derived_format = munge_format_impl(format, animation);
  nassertr(derived_format->is_registered(), NULL);

  // Store the answer in the map, so we can quickly get it next time.
  bool inserted = formats.insert(Formats::value_type(format, derived_format)).second;
  nassertr(inserted, NULL);

  return derived_format;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) GeomMunger::
munge_format_impl(const GeomVertexFormat *orig, const GeomVertexAnimationSpec &) {
  return orig;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::munge_data_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexData, converts it as
//               necessary for rendering.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexData) GeomMunger::
munge_data_impl(const GeomVertexData *data) {
  nassertr(_is_registered, NULL);

  CPT(GeomVertexFormat) orig_format = data->get_format();
  CPT(GeomVertexFormat) new_format = 
    munge_format(orig_format, orig_format->get_animation());

  if (new_format == orig_format) {
    // Trivial case.
    return data;
  }

  return data->convert_to(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::munge_geom_impl
//       Access: Protected, Virtual
//  Description: Converts a Geom and/or its data as necessary.
////////////////////////////////////////////////////////////////////
bool GeomMunger::
munge_geom_impl(CPT(Geom) &, CPT(GeomVertexData) &) {
  // The default implementation does nothing (the work has already
  // been done in munge_format_impl() and munge_data_impl()).
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int GeomMunger::
compare_to_impl(const GeomMunger *other) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Compares two GeomMungers, considering only whether
//               they would produce a different answer to
//               munge_format(), munge_data(), or munge_geom().  (They
//               still might be different in other ways, but if they
//               would produce the same answer, this function consider
//               them to be the same.)
////////////////////////////////////////////////////////////////////
int GeomMunger::
geom_compare_to_impl(const GeomMunger *other) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::make_registry
//       Access: Private
//  Description: Returns the global registry object.
////////////////////////////////////////////////////////////////////
void GeomMunger::
make_registry() {
  if (_registry == (Registry *)NULL) {
    _registry = new Registry;
  }
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::do_register
//       Access: Private
//  Description: Called internally when the munger is registered.
////////////////////////////////////////////////////////////////////
void GeomMunger::
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
//     Function: GeomMunger::do_unregister
//       Access: Private
//  Description: Called internally when the munger is unregistered.
////////////////////////////////////////////////////////////////////
void GeomMunger::
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
//     Function: GeomMunger::CacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GeomMunger::CacheEntry::
output(ostream &out) const {
  out << "munger " << _munger;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Registry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GeomMunger::Registry::
Registry() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Registry::register_munger
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
PT(GeomMunger) GeomMunger::Registry::
register_munger(GeomMunger *munger) {
  if (munger->is_registered()) {
    return munger;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has
  // a zero reference count and is not added into the map below, it
  // will be automatically deleted when this function returns.
  PT(GeomMunger) pt_munger = munger;

  Mungers::iterator mi = _mungers.insert(munger).first;
  GeomMunger *new_munger = (*mi);
  if (!new_munger->is_registered()) {
    new_munger->_registered_key = mi;
    new_munger->do_register();
  }

  return new_munger;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomMunger::Registry::unregister_munger
//       Access: Public
//  Description: Removes the indicated munger from the registry.
//               Normally this should not be done until the munger is
//               destructing.
////////////////////////////////////////////////////////////////////
void GeomMunger::Registry::
unregister_munger(GeomMunger *munger) {
  nassertv(munger->is_registered());
  nassertv(munger->_registered_key != _mungers.end());
  _mungers.erase(munger->_registered_key);
  munger->_registered_key = _mungers.end();
  munger->do_unregister();
}
