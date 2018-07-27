/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomMunger.h
 * @author drose
 * @date 2005-03-10
 */

#ifndef GEOMMUNGER_H
#define GEOMMUNGER_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "geomVertexAnimationSpec.h"
#include "geomVertexFormat.h"
#include "geomVertexData.h"
#include "geomCacheEntry.h"
#include "indirectCompareTo.h"
#include "pStatCollector.h"
#include "lightMutex.h"
#include "lightReMutex.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pset.h"

class GraphicsStateGuardianBase;
class RenderState;
class Geom;

/**
 * Objects of this class are used to convert vertex data from a Geom into a
 * format suitable for passing to the rendering backend.  Typically, the
 * rendering backend will create a specialization of this class to handle its
 * particular needs (e.g.  DXGeomMunger).  This class is necessary because
 * DirectX and OpenGL have somewhat different requirements for vertex format.
 *
 * This also performs runtime application of state changes to the vertex data;
 * for instance, by scaling all of the color values in response to a
 * ColorScaleAttrib.
 *
 * A GeomMunger must be registered before it can be used, and once registered,
 * the object is constant and cannot be changed.  All registered GeomMungers
 * that perform the same operation will have the same pointer.
 */
class EXPCL_PANDA_GOBJ GeomMunger : public TypedReferenceCount, public GeomEnums {
public:
  GeomMunger(GraphicsStateGuardianBase *gsg);
  GeomMunger(const GeomMunger &copy);
  void operator = (const GeomMunger &copy);
  virtual ~GeomMunger();

  INLINE GraphicsStateGuardianBase *get_gsg() const;

  INLINE bool is_registered() const;
  INLINE static PT(GeomMunger) register_munger(GeomMunger *munger, Thread *current_thread);
  INLINE static void unregister_mungers_for_gsg(GraphicsStateGuardianBase *gsg);

  INLINE CPT(GeomVertexFormat) munge_format(const GeomVertexFormat *format,
                                            const GeomVertexAnimationSpec &animation) const;

  INLINE CPT(GeomVertexData) munge_data(const GeomVertexData *data) const;
  void remove_data(const GeomVertexData *data);

  bool munge_geom(CPT(Geom) &geom, CPT(GeomVertexData) &data,
                  bool force, Thread *current_thread);

  INLINE CPT(GeomVertexFormat) premunge_format(const GeomVertexFormat *format) const;
  INLINE CPT(GeomVertexData) premunge_data(const GeomVertexData *data) const;
  INLINE void premunge_geom(CPT(Geom) &geom, CPT(GeomVertexData) &data) const;

public:
  INLINE int compare_to(const GeomMunger &other) const;
  INLINE int geom_compare_to(const GeomMunger &other) const;

protected:
  INLINE void unregister_myself();

  CPT(GeomVertexFormat) do_munge_format(const GeomVertexFormat *format,
                                        const GeomVertexAnimationSpec &animation);

  virtual CPT(GeomVertexFormat) munge_format_impl(const GeomVertexFormat *orig,
                                                  const GeomVertexAnimationSpec &animation);
  virtual CPT(GeomVertexData) munge_data_impl(const GeomVertexData *data);
  virtual void munge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &data,
                               Thread *current_thread);


  CPT(GeomVertexFormat) do_premunge_format(const GeomVertexFormat *format);
  virtual CPT(GeomVertexFormat) premunge_format_impl(const GeomVertexFormat *orig);
  virtual CPT(GeomVertexData) premunge_data_impl(const GeomVertexData *data);
  virtual void premunge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &data);

  virtual int compare_to_impl(const GeomMunger *other) const;
  virtual int geom_compare_to_impl(const GeomMunger *other) const;

private:
  class Registry;
  INLINE static Registry *get_registry();
  static void make_registry();

  void do_register(Thread *current_thread);
  void do_unregister();

private:
  class CacheEntry : public GeomCacheEntry {
  public:
    virtual void output(std::ostream &out) const;

    PT(GeomMunger) _munger;
  };

  typedef pmap<CPT(GeomVertexFormat), CPT(GeomVertexFormat) > Formats;
  typedef pmap<GeomVertexAnimationSpec, Formats> FormatsByAnimation;
  FormatsByAnimation _formats_by_animation;
  Formats _premunge_formats;

  // This mutex protects the above.
  LightMutex _formats_lock;

  GraphicsStateGuardianBase *_gsg;

  bool _is_registered;
  typedef pset<GeomMunger *, IndirectCompareTo<GeomMunger> > Mungers;
  class EXPCL_PANDA_GOBJ Registry {
  public:
    Registry();
    PT(GeomMunger) register_munger(GeomMunger *munger, Thread *current_thread);
    void unregister_munger(GeomMunger *munger);
    void unregister_mungers_for_gsg(GraphicsStateGuardianBase *gsg);

    Mungers _mungers;
    LightReMutex _registry_lock;
  };

  // We store the iterator into the above registry, while we are registered.
  // This makes it easier to remove our own entry, especially when the
  // destructor is called.  Since it's a virtual destructor, we can't reliably
  // look up our pointer in the map once we have reached the base class
  // destructor (since the object has changed types by then, and the sorting
  // in the map depends partly on type).
  Mungers::iterator _registered_key;

  static Registry *_registry;

  static PStatCollector _munge_pcollector;

  friend class GeomCacheManager;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GeomMunger",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class Geom;
};

#include "geomMunger.I"

#endif
