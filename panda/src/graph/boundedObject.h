// Filename: boundedObject.h
// Created by:  drose (02Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef BOUNDEDOBJECT_H
#define BOUNDEDOBJECT_H

#include <pandabase.h>

#include "boundingVolume.h"

#include <typeHandle.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// 	 Class : BoundedObject
// Description : This is any object (particularly in a scene graph)
//               that may have a bounding volume established for it.
//               The user may set a fixed bounding volume, or s/he may
//               specify that the volume should be recomputed
//               dynamically.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundedObject {
public:
  INLINE BoundedObject();
  virtual ~BoundedObject();

PUBLISHED:
  enum BoundingVolumeType {
    BVT_static,
    BVT_dynamic_sphere,
  };

  INLINE void set_bound(BoundingVolumeType type);
  INLINE void set_bound(const BoundingVolume &volume);
  INLINE const BoundingVolume &get_bound() const;

  INLINE bool mark_bound_stale();
  INLINE void force_bound_stale();
  INLINE bool is_bound_stale() const;

protected:
  virtual void propagate_stale_bound();
  virtual void recompute_bound();

private:
  bool _bound_stale;
  BoundingVolumeType _bound_type;

protected:
  PT(BoundingVolume) _bound;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "BoundedObject");
  }

private:
  static TypeHandle _type_handle;
};

#include "boundedObject.I"

#endif

