// Filename: boundedObject.h
// Created by:  drose (02Oct99)
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

#ifndef BOUNDEDOBJECT_H
#define BOUNDEDOBJECT_H

#include "pandabase.h"

#include "boundingVolume.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "typedObject.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : BoundedObject
// Description : This is any object (particularly in a scene graph)
//               that may have a bounding volume established for it.
//               The user may set a fixed bounding volume, or s/he may
//               specify that the volume should be recomputed
//               dynamically.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BoundedObject {
public:
  INLINE BoundedObject();
  INLINE BoundedObject(const BoundedObject &copy);
  INLINE void operator = (const BoundedObject &copy);
  virtual ~BoundedObject();

PUBLISHED:
  enum BoundingVolumeType {
    BVT_static,
    BVT_dynamic_sphere,
  };

  INLINE void set_bound(BoundingVolumeType type);
  INLINE void set_bound(const BoundingVolume &volume);
  const BoundingVolume &get_bound() const;

  INLINE bool mark_bound_stale();
  INLINE void force_bound_stale();
  INLINE bool is_bound_stale() const;

  INLINE void set_final(bool flag);
  INLINE bool is_final() const;

protected:
  virtual void propagate_stale_bound();
  virtual BoundingVolume *recompute_bound();

  INLINE const BoundingVolume *get_bound_ptr() const;
  INLINE BoundingVolume *set_bound_ptr(BoundingVolume *bound);

private:
  enum Flags {
    F_bound_stale  = 0x0001,
    F_final        = 0x0002,
  };

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    INLINE void operator = (const CData &copy);

    virtual CycleData *make_copy() const;

    int _flags;
    BoundingVolumeType _bound_type;
    PT(BoundingVolume) _bound;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "BoundedObject");
  }

private:
  static TypeHandle _type_handle;

  friend class PandaNode;
  friend class CData;  // MSVC6 seems to require this.
};

#include "boundedObject.I"

#endif

