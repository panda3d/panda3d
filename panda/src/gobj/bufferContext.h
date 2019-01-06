/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bufferContext.h
 * @author drose
 * @date 2006-03-16
 */

#ifndef BUFFERCONTEXT_H
#define BUFFERCONTEXT_H

#include "pandabase.h"

#include "savedContext.h"
#include "updateSeq.h"
#include "linkedListNode.h"
#include "bufferContextChain.h"
#include "bufferResidencyTracker.h"

class PreparedGraphicsObjects;
class TypedWritableReferenceCount;

/**
 * This is a base class for those kinds of SavedContexts that occupy an
 * easily-measured (and substantial) number of bytes in the video card's frame
 * buffer memory or AGP memory.  At the present, this includes most of the
 * SavedContext types: VertexBufferContext and IndexBufferContext, as well as
 * TextureContext.
 *
 * This class provides methods for tracking the video memory utilization, as
 * well as residency of each object, via PStats.
 */
class EXPCL_PANDA_GOBJ BufferContext : public SavedContext, private LinkedListNode {
public:
  BufferContext(BufferResidencyTracker *residency, TypedWritableReferenceCount *object);
  virtual ~BufferContext();

  INLINE TypedWritableReferenceCount *get_object() const;

PUBLISHED:
  INLINE size_t get_data_size_bytes() const;
  INLINE UpdateSeq get_modified() const;
  INLINE bool get_active() const;
  INLINE bool get_resident() const;

  MAKE_PROPERTY(object, get_object);

  MAKE_PROPERTY(data_size_bytes, get_data_size_bytes);
  MAKE_PROPERTY(modified, get_modified);
  MAKE_PROPERTY(active, get_active);
  MAKE_PROPERTY(resident, get_resident);

public:
  INLINE void set_active(bool flag);
  INLINE void set_resident(bool flag);

  INLINE BufferContext *get_next() const;

  INLINE void update_data_size_bytes(size_t new_data_size_bytes);
  INLINE void update_modified(UpdateSeq new_modified);

private:
  void set_owning_chain(BufferContextChain *chain);

protected:
  // This cannot be a PT(), because the object and the GSG both own their
  // BufferContexts!  That would create a circular reference count.
  TypedWritableReferenceCount *_object;

private:
  BufferResidencyTracker *_residency;
  int _residency_state;

  size_t _data_size_bytes;
  UpdateSeq _modified;
  BufferContextChain *_owning_chain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SavedContext::init_type();
    register_type(_type_handle, "BufferContext",
                  SavedContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PreparedGraphicsObjects;
  friend class BufferResidencyTracker;
  friend class BufferContextChain;
};

#include "bufferContext.I"

#endif
