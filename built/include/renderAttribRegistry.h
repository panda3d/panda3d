/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderAttribRegistry.h
 * @author drose
 * @date 2008-11-13
 */

#ifndef RENDERATTRIBREGISTRY_H
#define RENDERATTRIBREGISTRY_H

#include "pandabase.h"
#include "typeHandle.h"
#include "vector_int.h"
#include "pointerTo.h"
#include "bitMask.h"

class RenderAttrib;
class DeletedBufferChain;

/**
 * This class is used to associate each RenderAttrib with a different slot
 * index at runtime, so we can store a list of RenderAttribs in the
 * RenderState object, and very quickly look them up by type.
 */
class EXPCL_PANDA_PGRAPH RenderAttribRegistry {
private:
  RenderAttribRegistry();
  ~RenderAttribRegistry();

public:
  typedef CPT(RenderAttrib) MakeDefaultFunc();

  // This typedef defines the native bitmask type for indicating which slots
  // are present in a RenderState.  It must be wide enough to allow room for
  // all of the possible RenderAttribs that might register themselves.
  // Presently, 32 bits is wide enough, but only barely; when we exceed this
  // limit, we will need to go to a 64-bit type instead.  It will be
  // interesting to see whether a BitMask64 or a DoubleBitMask<BitMask32> will
  // be faster on a 32-bit machine.
  typedef BitMask32 SlotMask;

  // Raise this number whenever we add a new attrib.  This used to be
  // determined at runtime, but it's better to have it as a constexpr.
  static const int _max_slots = 32;

  int register_slot(TypeHandle type_handle, int sort,
                    RenderAttrib *default_attrib);

PUBLISHED:
  INLINE int get_slot(TypeHandle type_handle) const;
  static constexpr int get_max_slots() { return _max_slots; }

  INLINE int get_num_slots() const;
  INLINE TypeHandle get_slot_type(int slot) const;
  INLINE int get_slot_sort(int slot) const;
  void set_slot_sort(int slot, int sort);
  INLINE const RenderAttrib *get_slot_default(int slot) const;

  INLINE int get_num_sorted_slots() const;
  INLINE int get_sorted_slot(int n) const;

  INLINE static RenderAttribRegistry *get_global_ptr();

public:
  INLINE static RenderAttribRegistry *quick_get_global_ptr();

private:
  static void init_global_ptr();

private:
  class SortSlots {
  public:
    INLINE SortSlots(RenderAttribRegistry *reg);
    INLINE bool operator () (int a, int b) const;
    RenderAttribRegistry *_reg;
  };

  class RegistryNode {
  public:
    INLINE RegistryNode(TypeHandle type, int sort,
                        const RenderAttrib *default_attrib);

    TypeHandle _type;
    int _sort;
    const RenderAttrib *_default_attrib;
  };
  typedef pvector<RegistryNode> Registry;
  Registry _registry;

  vector_int _slots_by_type;
  vector_int _sorted_slots;

  static RenderAttribRegistry *_global_ptr;
};

#include "renderAttribRegistry.I"

#endif
