/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderState.h
 * @author drose
 * @date 2002-02-21
 */

#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "nodeCachedReferenceCount.h"
#include "pointerTo.h"
#include "pvector.h"
#include "updateSeq.h"
#include "pStatCollector.h"
#include "renderModeAttrib.h"
#include "texMatrixAttrib.h"
#include "geomMunger.h"
#include "weakPointerTo.h"
#include "lightReMutex.h"
#include "lightMutex.h"
#include "deletedChain.h"
#include "simpleHashMap.h"
#include "cacheStats.h"
#include "renderAttribRegistry.h"

class FactoryParams;
class ShaderAttrib;

/**
 * This represents a unique collection of RenderAttrib objects that correspond
 * to a particular renderable state.
 *
 * You should not attempt to create or modify a RenderState object directly.
 * Instead, call one of the make() functions to create one for you.  And
 * instead of modifying a RenderState object, create a new one.
 */
class EXPCL_PANDA_PGRAPH RenderState : public NodeCachedReferenceCount {
protected:
  RenderState();

private:
  RenderState(const RenderState &copy);

public:
  virtual ~RenderState();
  ALLOC_DELETED_CHAIN(RenderState);

  RenderState &operator = (const RenderState &copy) = delete;

  typedef RenderAttribRegistry::SlotMask SlotMask;

PUBLISHED:
  int compare_to(const RenderState &other) const;
  int compare_sort(const RenderState &other) const;
  int compare_mask(const RenderState &other, SlotMask compare_mask) const;
  INLINE size_t get_hash() const;

  INLINE bool is_empty() const;

  INLINE bool has_cull_callback() const;
  bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

  INLINE static CPT(RenderState) make_empty();
  static CPT(RenderState) make(const RenderAttrib *attrib, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2,
                               const RenderAttrib *attrib3, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2,
                               const RenderAttrib *attrib3,
                               const RenderAttrib *attrib4, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2,
                               const RenderAttrib *attrib3,
                               const RenderAttrib *attrib4,
                               const RenderAttrib *attrib5, int override = 0);
  static CPT(RenderState) make(const RenderAttrib * const *attrib,
                               int num_attribs, int override = 0);

  CPT(RenderState) compose(const RenderState *other) const;
  CPT(RenderState) invert_compose(const RenderState *other) const;

  CPT(RenderState) add_attrib(const RenderAttrib *attrib, int override = 0) const;
  CPT(RenderState) set_attrib(const RenderAttrib *attrib) const;
  CPT(RenderState) set_attrib(const RenderAttrib *attrib, int override) const;
  INLINE CPT(RenderState) remove_attrib(TypeHandle type) const;
  CPT(RenderState) remove_attrib(int slot) const;

  CPT(RenderState) adjust_all_priorities(int adjustment) const;

  INLINE bool has_attrib(TypeHandle type) const;
  INLINE bool has_attrib(int slot) const;
  INLINE const RenderAttrib *get_attrib(TypeHandle type) const;
  ALWAYS_INLINE const RenderAttrib *get_attrib(int slot) const;
  INLINE const RenderAttrib *get_attrib_def(int slot) const;
  INLINE int get_override(TypeHandle type) const;
  INLINE int get_override(int slot) const;

  MAKE_MAP_PROPERTY(attribs, has_attrib, get_attrib);

  INLINE CPT(RenderState) get_unique() const;

  virtual bool unref() const;

  INLINE void cache_ref() const;
  INLINE bool cache_unref() const;
  INLINE void node_ref() const;
  INLINE bool node_unref() const;

  INLINE size_t get_composition_cache_num_entries() const;
  INLINE size_t get_invert_composition_cache_num_entries() const;

  INLINE size_t get_composition_cache_size() const;
  INLINE const RenderState *get_composition_cache_source(size_t n) const;
  INLINE const RenderState *get_composition_cache_result(size_t n) const;
  INLINE size_t get_invert_composition_cache_size() const;
  INLINE const RenderState *get_invert_composition_cache_source(size_t n) const;
  INLINE const RenderState *get_invert_composition_cache_result(size_t n) const;
  EXTENSION(PyObject *get_composition_cache() const);
  EXTENSION(PyObject *get_invert_composition_cache() const);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

  static int get_max_priority();

  static int get_num_states();
  static int get_num_unused_states();
  static int clear_cache();
  static void clear_munger_cache();
  static int garbage_collect();
  static void list_cycles(std::ostream &out);
  static void list_states(std::ostream &out);
  static bool validate_states();
  EXTENSION(static PyObject *get_states());

PUBLISHED:
  // These methods are intended for use by low-level code, but they're also
  // handy enough to expose to high-level users.
  INLINE int get_draw_order() const;
  INLINE int get_bin_index() const;
  int get_geom_rendering(int geom_rendering) const;

public:
  static void bin_removed(int bin_index);

  INLINE static void flush_level();

#ifndef CPPPARSER
  template<class AttribType>
  INLINE bool get_attrib(const AttribType *&attrib) const;
  template<class AttribType>
  INLINE bool get_attrib(CPT(AttribType) &attrib) const;
  template<class AttribType>
  INLINE void get_attrib_def(const AttribType *&attrib) const;
  template<class AttribType>
  INLINE void get_attrib_def(CPT(AttribType) &attrib) const;
#endif  // CPPPARSER

private:
  INLINE void check_hash() const;
  bool validate_filled_slots() const;
  INLINE bool do_cache_unref() const;
  INLINE bool do_node_unref() const;
  INLINE void calc_hash();
  void do_calc_hash();

  class CompositionCycleDescEntry {
  public:
    INLINE CompositionCycleDescEntry(const RenderState *obj,
                                     const RenderState *result,
                                     bool inverted);

    const RenderState *_obj;
    const RenderState *_result;
    bool _inverted;
  };
  typedef pvector<CompositionCycleDescEntry> CompositionCycleDesc;

  static CPT(RenderState) return_new(RenderState *state);
  static CPT(RenderState) return_unique(RenderState *state);
  CPT(RenderState) do_compose(const RenderState *other) const;
  CPT(RenderState) do_invert_compose(const RenderState *other) const;
  void detect_and_break_cycles();
  static bool r_detect_cycles(const RenderState *start_state,
                              const RenderState *current_state,
                              int length, UpdateSeq this_seq,
                              CompositionCycleDesc *cycle_desc);
  static bool r_detect_reverse_cycles(const RenderState *start_state,
                                      const RenderState *current_state,
                                      int length, UpdateSeq this_seq,
                                      CompositionCycleDesc *cycle_desc);

  void release_new();
  void remove_cache_pointers();

  void determine_bin_index();
  void determine_cull_callback();
  void fill_default();

  INLINE void set_destructing();
  INLINE bool is_destructing() const;

  INLINE void consider_update_pstats(int old_referenced_bits) const;
  static void update_pstats(int old_referenced_bits, int new_referenced_bits);

public:
  static void init_states();

  // If this state contains an "auto" ShaderAttrib, then an explicit
  // ShaderAttrib will be synthesized by the runtime and stored here.  I can't
  // declare this as a ShaderAttrib because that would create a circular
  // include-file dependency problem.  Aaargh.
  mutable CPT(RenderAttrib) _generated_shader;
  mutable UpdateSeq _generated_shader_seq;

private:
  // This mutex protects _states.  It also protects any modification to the
  // cache, which is encoded in _composition_cache and
  // _invert_composition_cache.
  static LightReMutex *_states_lock;
  typedef SimpleHashMap<const RenderState *, std::nullptr_t, indirect_compare_to_hash<const RenderState *> > States;
  static States _states;
  static const RenderState *_empty_state;

  // This iterator records the entry corresponding to this RenderState object
  // in the above global set.  We keep the index around so we can remove it
  // when the RenderState destructs.
  int _saved_entry;

  // This data structure manages the job of caching the composition of two
  // RenderStates.  It's complicated because we have to be sure to remove the
  // entry if *either* of the input RenderStates destructs.  To implement
  // this, we always record Composition entries in pairs, one in each of the
  // two involved RenderState objects.
  class Composition {
  public:
    INLINE Composition();
    INLINE Composition(const Composition &copy);

    // _result is reference counted if and only if it is not the same pointer
    // as this.
    const RenderState *_result;
  };

  // The first element of the map is the object we compose with.  This is not
  // reference counted within this map; instead we store a companion pointer
  // in the other object, and remove the references explicitly when either
  // object destructs.
  typedef SimpleHashMap<const RenderState *, Composition, pointer_hash> CompositionCache;
  CompositionCache _composition_cache;
  CompositionCache _invert_composition_cache;

  // This is here to provide a quick cache of GSG + RenderState -> GeomMunger
  // for the cull phase.  It is here because it is faster to look up the GSG
  // in the RenderState pointer than vice-versa, since there are likely to be
  // far fewer GSG's than RenderStates.  The code to manage this map lives in
  // GraphicsStateGuardian::get_geom_munger().
  typedef SimpleHashMap<size_t, PT(GeomMunger), size_t_hash> Mungers;
  mutable Mungers _mungers;
  mutable int _last_mi;

  // Similarly, this is a cache of munged states.  This map is managed by
  // StateMunger::munge_state().
  typedef SimpleHashMap<size_t, WCPT(RenderState), size_t_hash> MungedStates;
  mutable MungedStates _munged_states;

  // This is used to mark nodes as we visit them to detect cycles.
  UpdateSeq _cycle_detect;
  static UpdateSeq _last_cycle_detect;

  // This keeps track of our current position through the garbage collection
  // cycle.
  static size_t _garbage_index;

  static PStatCollector _cache_update_pcollector;
  static PStatCollector _garbage_collect_pcollector;
  static PStatCollector _state_compose_pcollector;
  static PStatCollector _state_invert_pcollector;
  static PStatCollector _state_break_cycles_pcollector;
  static PStatCollector _state_validate_pcollector;

  static PStatCollector _node_counter;
  static PStatCollector _cache_counter;

private:
  // This is the actual data within the RenderState: a set of max_slots
  // RenderAttribs.
  class Attribute {
  public:
    INLINE Attribute(const RenderAttrib *attrib, int override);
    INLINE Attribute(int override = 0);
    INLINE Attribute(const Attribute &copy);
    INLINE void operator = (const Attribute &copy);
    INLINE void set(const RenderAttrib *attrib, int override);
    INLINE int compare_to(const Attribute &other) const;

    CPT(RenderAttrib) _attrib;
    int _override;
  };
  Attribute _attributes[RenderAttribRegistry::_max_slots];

  // We also store a bitmask of the non-NULL attributes in the above array.
  // This is redundant, but it is a useful cache.
  SlotMask _filled_slots;

  // We cache the index to the associated CullBin, if there happens to be a
  // CullBinAttrib in the state.
  int _bin_index;
  int _draw_order;
  size_t _hash;

  enum Flags {
    F_checked_bin_index       = 0x000001,
    F_checked_cull_callback   = 0x000002,
    F_has_cull_callback       = 0x000004,
    F_is_destructing          = 0x000008,
    F_hash_known              = 0x000010,
  };
  unsigned int _flags;

  vector_int *_read_overrides;  // Only used during bam reading.

  // This mutex protects _flags, and all of the above computed values.
  LightMutex _lock;

  static CacheStats _cache_stats;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);
  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeCachedReferenceCount::init_type();
    register_type(_type_handle, "RenderState",
                  NodeCachedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsStateGuardian;
  friend class RenderAttribRegistry;
  friend class Extension<RenderState>;
  friend class ShaderGenerator;
  friend class StateMunger;
};

// We can safely redefine this as a no-op.
template<>
INLINE void PointerToBase<RenderState>::update_type(To *ptr) {}

INLINE std::ostream &operator << (std::ostream &out, const RenderState &state) {
  state.output(out);
  return out;
}

#include "renderState.I"

#endif
