// Filename: renderState.h
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "indirectLess.h"
#include "ordered_vector.h"

class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : RenderState
// Description : This represents a unique collection of RenderAttrib
//               objects that correspond to a particular renderable
//               state.
//
//               You should not attempt to create or modify a
//               RenderState object directly.  Instead, call one of
//               the make() functions to create one for you.  And
//               instead of modifying a RenderState object, create a
//               new one.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderState : public TypedWritableReferenceCount {
protected:
  RenderState();

private:
  RenderState(const RenderState &copy);
  void operator = (const RenderState &copy);

public:
  virtual ~RenderState();

  bool operator < (const RenderState &other) const;

PUBLISHED:
  INLINE bool is_empty() const;
  INLINE int get_num_attribs() const;
  INLINE const RenderAttrib *get_attrib(int n) const;
  INLINE int get_override(int n) const;

  int find_attrib(TypeHandle type) const;

  static CPT(RenderState) make_empty();
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

  CPT(RenderState) compose(const RenderState *other) const;
  CPT(RenderState) invert_compose(const RenderState *other) const;

  CPT(RenderState) add(const RenderAttrib *attrib, int override = 0) const;
  CPT(RenderState) remove(TypeHandle type) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

public:
  CPT(RenderState) issue_delta_modify(const RenderState *other, 
                                      GraphicsStateGuardianBase *gsg) const;
  CPT(RenderState) issue_delta_set(const RenderState *other, 
                                   GraphicsStateGuardianBase *gsg) const;

private:
  static CPT(RenderState) return_new(RenderState *state);
  CPT(RenderState) do_compose(const RenderState *other) const;
  CPT(RenderState) do_invert_compose(const RenderState *other) const;

private:
  typedef pset<const RenderState *, IndirectLess<RenderState> > States;
  static States _states;
  static CPT(RenderState) _empty_state;

  // This iterator records the entry corresponding to this RenderState
  // object in the above global set.  We keep the iterator around so
  // we can remove it when the RenderState destructs.
  States::iterator _saved_entry;

  // This data structure manages the job of caching the composition of
  // two RenderStates.  It's complicated because we have to be sure to
  // remove the entry if *either* of the input RenderStates destructs.
  // To implement this, we always record Composition entries in pairs,
  // one in each of the two involved RenderState objects.
  class Composition {
  public:
    CPT(RenderState) _result;
  };
    
  typedef pmap<const RenderState *, Composition> CompositionCache;
  CompositionCache _composition_cache;
  CompositionCache _invert_composition_cache;

  // Thise pointer is used to cache the result of compose(this).  This
  // has to be a special case, because we have to handle the reference
  // counts carefully so that we don't leak.  Most of the time, the
  // result of compose(this) is this, which should not be reference
  // counted, but other times the result is something else (which
  // should be).
  const RenderState *_self_compose;

  // This is the actual set of data within the RenderState: a set of
  // RenderAttribs.
  class Attribute {
  public:
    INLINE Attribute(const RenderAttrib *attrib, int override);
    INLINE Attribute(TypeHandle type);
    INLINE Attribute(const Attribute &copy);
    INLINE void operator = (const Attribute &copy);
    INLINE bool operator < (const Attribute &other) const;
    INLINE int compare_to(const Attribute &other) const;

    TypeHandle _type;
    CPT(RenderAttrib) _attrib;
    int _override;
  };
  typedef ov_set<Attribute> Attributes;
  Attributes _attributes;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void finalize();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  static TypedWritable *new_from_bam(RenderState *state, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "RenderState",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const RenderState &state) {
  state.output(out);
  return out;
}

#include "renderState.I"

#endif

