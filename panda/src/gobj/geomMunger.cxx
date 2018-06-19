/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomMunger.cxx
 * @author drose
 * @date 2005-03-10
 */

#include "geomMunger.h"
#include "geom.h"
#include "geomCacheManager.h"
#include "lightMutexHolder.h"
#include "lightReMutexHolder.h"
#include "pStatTimer.h"

GeomMunger::Registry *GeomMunger::_registry = nullptr;
TypeHandle GeomMunger::_type_handle;

PStatCollector GeomMunger::_munge_pcollector("*:Munge");

/**
 *
 */
GeomMunger::
GeomMunger(GraphicsStateGuardianBase *gsg) :
  _gsg(gsg),
  _is_registered(false)
{
#ifndef NDEBUG
  Registry *registry = get_registry();
  LightReMutexHolder holder(registry->_registry_lock);
  _registered_key = registry->_mungers.end();
#endif
}

/**
 *
 */
GeomMunger::
GeomMunger(const GeomMunger &copy) :
  _is_registered(false)
{
#ifndef NDEBUG
  Registry *registry = get_registry();
  LightReMutexHolder holder(registry->_registry_lock);
  _registered_key = registry->_mungers.end();
#endif
}

/**
 *
 */
void GeomMunger::
operator = (const GeomMunger &copy) {
  nassertv(!_is_registered);
}

/**
 *
 */
GeomMunger::
~GeomMunger() {
  unregister_myself();
  nassertv(_formats_by_animation.empty());
}

/**
 * Removes a prepared GeomVertexData from the cache.
 */
void GeomMunger::
remove_data(const GeomVertexData *data) {
  // If this assertion is triggered, maybe we accidentally deleted a
  // GeomVertexData while we were in the process of unregistering, causing a
  // recursive re-entry.
  nassertv(_is_registered);
}

/**
 * Applies the indicated munger to the geom and its data, and returns a
 * (possibly different) geom and data, according to the munger's whim.
 *
 * The assumption is that for a particular geom and a particular munger, the
 * result will always be the same; so this result may be cached.
 *
 * If force is false, this may do nothing and return false if the vertex data
 * is nonresident.  If force is true, this will always return true, but it may
 * have to block while the vertex data is paged in.
 */
bool GeomMunger::
munge_geom(CPT(Geom) &geom, CPT(GeomVertexData) &data,
           bool force, Thread *current_thread) {

  // Look up the munger in the geom's cache--maybe we've recently applied it.
  PT(Geom::CacheEntry) entry;

  Geom::CacheKey key(data, this);

  geom->_cache_lock.acquire();
  Geom::Cache::const_iterator ci = geom->_cache.find(&key);
  if (ci == geom->_cache.end()) {
    geom->_cache_lock.release();
  } else {
    entry = (*ci).second;
    geom->_cache_lock.release();
    nassertr(entry->_source == geom, false);

    // Here's an element in the cache for this computation.  Record a cache
    // hit, so this element will stay in the cache a while longer.
    entry->refresh(current_thread);

    // Now check that it's fresh.
    Geom::CDCacheReader cdata(entry->_cycler, current_thread);
    if (cdata->_source == geom &&
        cdata->_geom_result != nullptr &&
        geom->get_modified(current_thread) <= cdata->_geom_result->get_modified(current_thread) &&
        data->get_modified(current_thread) <= cdata->_data_result->get_modified(current_thread)) {
      // The cache entry is still good; use it.

      geom = cdata->_geom_result;
      data = cdata->_data_result;
      return true;
    }

    // The cache entry is stale, but we'll recompute it below.  Note that
    // there's a small race condition here; another thread might recompute the
    // cache at the same time.  No big deal, since it'll compute the same
    // result.
  }

  if (!force && (!geom->request_resident() || !data->request_resident())) {
    // Oh dear, the data isn't resident.  We can't munge it, so give up.
    return false;
  }

  // Ok, invoke the munger.
  PStatTimer timer(_munge_pcollector, current_thread);

  PT(Geom) orig_geom = (Geom *)geom.p();
  data = munge_data(data);
  munge_geom_impl(geom, data, current_thread);

  // Record the new result in the cache.
  if (entry == nullptr) {
    // Create a new entry for the result.
    // We don't need the key anymore, move the pointers into the CacheEntry.
    entry = new Geom::CacheEntry(orig_geom, std::move(key));

    {
      LightMutexHolder holder(orig_geom->_cache_lock);
      bool inserted = orig_geom->_cache.insert(Geom::Cache::value_type(&entry->_key, entry)).second;
      if (!inserted) {
        // Some other thread must have beat us to the punch.  Never mind.
        return true;
      }
    }

    // And tell the cache manager about the new entry.  (It might immediately
    // request a delete from the cache of the thing we just added.)
    entry->record(current_thread);
  }

  // Finally, store the cached result on the entry.
  Geom::CDCacheWriter cdata(entry->_cycler, true, current_thread);
  cdata->_source = (Geom *)orig_geom.p();
  cdata->set_result(geom, data);

  return true;
}

/**
 * The protected implementation of munge_format().  This exists just to cast
 * away the const pointer.
 */
CPT(GeomVertexFormat) GeomMunger::
do_munge_format(const GeomVertexFormat *format,
                const GeomVertexAnimationSpec &animation) {
  nassertr(_is_registered, nullptr);
  nassertr(format->is_registered(), nullptr);

  LightMutexHolder holder(_formats_lock);

  Formats &formats = _formats_by_animation[animation];

  Formats::iterator fi;
  fi = formats.find(format);
  if (fi != formats.end()) {
    // This format was previously munged, so the answer will be the same.
    return (*fi).second;
  }

  // We have to munge this format for the first time.
  CPT(GeomVertexFormat) derived_format = munge_format_impl(format, animation);
  nassertr(derived_format->is_registered(), nullptr);

  // Store the answer in the map, so we can quickly get it next time.
  bool inserted = formats.insert(Formats::value_type(format, derived_format)).second;
  nassertr(inserted, nullptr);

  return derived_format;
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) GeomMunger::
munge_format_impl(const GeomVertexFormat *orig, const GeomVertexAnimationSpec &) {
  return orig;
}

/**
 * Given a source GeomVertexData, converts it as necessary for rendering.
 */
CPT(GeomVertexData) GeomMunger::
munge_data_impl(const GeomVertexData *data) {
  nassertr(_is_registered, nullptr);

  CPT(GeomVertexFormat) orig_format = data->get_format();
  CPT(GeomVertexFormat) new_format =
    munge_format(orig_format, orig_format->get_animation());

  if (new_format == orig_format) {
    // Trivial case.
    return data;
  }

  return data->convert_to(new_format);
}

/**
 * Converts a Geom and/or its data as necessary.
 */
void GeomMunger::
munge_geom_impl(CPT(Geom) &, CPT(GeomVertexData) &, Thread *) {
  // The default implementation does nothing (the work has already been done
  // in munge_format_impl() and munge_data_impl()).
}

/**
 * The protected implementation of premunge_format().  This exists just to
 * cast away the const pointer.
 */
CPT(GeomVertexFormat) GeomMunger::
do_premunge_format(const GeomVertexFormat *format) {
  nassertr(_is_registered, nullptr);
  nassertr(format->is_registered(), nullptr);

  LightMutexHolder holder(_formats_lock);

  Formats::iterator fi;
  fi = _premunge_formats.find(format);
  if (fi != _premunge_formats.end()) {
    // This format was previously munged, so the answer will be the same.
    return (*fi).second;
  }

  // We have to munge this format for the first time.
  CPT(GeomVertexFormat) derived_format = premunge_format_impl(format);
  nassertr(derived_format->is_registered(), nullptr);

  // Store the answer in the map, so we can quickly get it next time.
  bool inserted = _premunge_formats.insert(Formats::value_type(format, derived_format)).second;
  nassertr(inserted, nullptr);

  return derived_format;
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) GeomMunger::
premunge_format_impl(const GeomVertexFormat *orig) {
  return orig;
}

/**
 * Given a source GeomVertexData, converts it as necessary for rendering.
 */
CPT(GeomVertexData) GeomMunger::
premunge_data_impl(const GeomVertexData *data) {
  nassertr(_is_registered, nullptr);

  CPT(GeomVertexFormat) orig_format = data->get_format();
  CPT(GeomVertexFormat) new_format = premunge_format(orig_format);

  if (new_format == orig_format) {
    // Trivial case.
    return data;
  }

  return data->convert_to(new_format);
}

/**
 * Converts a Geom and/or its data as necessary.
 */
void GeomMunger::
premunge_geom_impl(CPT(Geom) &, CPT(GeomVertexData) &) {
  // The default implementation does nothing (the work has already been done
  // in premunge_format_impl() and premunge_data_impl()).
}

/**
 * Called to compare two GeomMungers who are known to be of the same type, for
 * an apples-to-apples comparison.  This will never be called on two pointers
 * of a different type.
 */
int GeomMunger::
compare_to_impl(const GeomMunger *other) const {
  return 0;
}

/**
 * Compares two GeomMungers, considering only whether they would produce a
 * different answer to munge_format(), munge_data(), or munge_geom().  (They
 * still might be different in other ways, but if they would produce the same
 * answer, this function will consider them to be the same.)
 */
int GeomMunger::
geom_compare_to_impl(const GeomMunger *other) const {
  return 0;
}

/**
 * Returns the global registry object.
 */
void GeomMunger::
make_registry() {
  if (_registry == nullptr) {
    _registry = new Registry;
  }
}

/**
 * Called internally when the munger is registered.
 */
void GeomMunger::
do_register(Thread *current_thread) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "GeomMunger::do_register(): " << (void *)this << "\n";
  }
  nassertv(!_is_registered);
  nassertv(_formats_by_animation.empty());

  // Tell the cache manager to hang on to this new GeomMunger, so we don't
  // waste our time re-registering the same GeomMunger over and over again.
  CacheEntry *entry = new CacheEntry;
  entry->_munger = this;
  entry->record(current_thread);

  _is_registered = true;
}

/**
 * Called internally when the munger is unregistered.
 */
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

/**
 *
 */
void GeomMunger::CacheEntry::
output(std::ostream &out) const {
  out << "munger " << _munger;
}

/**
 *
 */
GeomMunger::Registry::
Registry() {
}

/**
 * Adds the indicated munger to the registry, if there is not an equivalent
 * munger already there; in either case, returns the pointer to the equivalent
 * munger now in the registry.
 *
 * This must be called before a munger may be used in a Geom.  After this
 * call, you should discard the original pointer you passed in (which may or
 * may not now be invalid) and let its reference count decrement normally; you
 * should use only the returned value from this point on.
 */
PT(GeomMunger) GeomMunger::Registry::
register_munger(GeomMunger *munger, Thread *current_thread) {
  if (munger->is_registered()) {
    return munger;
  }

  // Save the incoming pointer in a local PointerTo, so that if it has a zero
  // reference count and is not added into the map below, it will be
  // automatically deleted when this function returns.
  PT(GeomMunger) pt_munger = munger;

  LightReMutexHolder holder(_registry_lock);

  Mungers::iterator mi = _mungers.insert(munger).first;
  GeomMunger *new_munger = (*mi);
  if (!new_munger->is_registered()) {
    new_munger->_registered_key = mi;
    new_munger->do_register(current_thread);
  }

  return new_munger;
}

/**
 * Removes the indicated munger from the registry.  Normally this should not
 * be done until the munger is destructing.
 */
void GeomMunger::Registry::
unregister_munger(GeomMunger *munger) {
  LightReMutexHolder holder(_registry_lock);

  nassertv(munger->is_registered());
  nassertv(munger->_registered_key != _mungers.end());
  _mungers.erase(munger->_registered_key);
  munger->_registered_key = _mungers.end();
  munger->do_unregister();
}

/**
 * Removes all the mungers from the registry that are associated with the
 * indicated GSG.
 */
void GeomMunger::Registry::
unregister_mungers_for_gsg(GraphicsStateGuardianBase *gsg) {
  LightReMutexHolder holder(_registry_lock);

  Mungers::iterator mi = _mungers.begin();
  while (mi != _mungers.end()) {
    GeomMunger *munger = (*mi);
    Mungers::iterator mnext = mi;
    ++mnext;

    if (munger->get_gsg() == gsg) {
      nassertv(mi == munger->_registered_key);
      _mungers.erase(mi);
      munger->_registered_key = _mungers.end();
      munger->do_unregister();
    }

    mi = mnext;
  }
}
