// Filename: clipPlaneAttrib.h
// Created by:  drose (11Jul02)
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

#ifndef CLIPPINGPLANEATTRIB_H
#define CLIPPINGPLANEATTRIB_H

#include "pandabase.h"

#include "planeNode.h"
#include "renderAttrib.h"
#include "ordered_vector.h"

////////////////////////////////////////////////////////////////////
//       Class : ClipPlaneAttrib
// Description : This functions similarly to a LightAttrib.  It
//               indicates the set of clipping planes that modify the
//               geometry at this level and below.  A ClipPlaneAttrib
//               can either add planes or remove planes from the total
//               set of clipping planes in effect.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ClipPlaneAttrib : public RenderAttrib {
private:
  INLINE ClipPlaneAttrib();

PUBLISHED:
  enum Operation {
    O_set,
    O_add,
    O_remove
  };

  static CPT(RenderAttrib) make_all_off();
  static CPT(RenderAttrib) make(Operation op, 
                                PlaneNode *plane);
  static CPT(RenderAttrib) make(Operation op, 
                                PlaneNode *plane1, PlaneNode *plane2);
  static CPT(RenderAttrib) make(Operation op, 
                                PlaneNode *plane1, PlaneNode *plane2,
                                PlaneNode *plane3);
  static CPT(RenderAttrib) make(Operation op, 
                                PlaneNode *plane1, PlaneNode *plane2,
                                PlaneNode *plane3, PlaneNode *plane4);

  INLINE Operation get_operation() const;

  INLINE int get_num_planes() const;
  INLINE PlaneNode *get_plane(int n) const;
  bool has_plane(PlaneNode *plane) const;

  INLINE CPT(RenderAttrib) add_plane(PlaneNode *plane) const;
  INLINE CPT(RenderAttrib) remove_plane(PlaneNode *plane) const;

  INLINE bool is_identity() const;
  INLINE bool is_all_off() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  CPT(RenderAttrib) do_add(const ClipPlaneAttrib *other, Operation op) const;
  CPT(RenderAttrib) do_remove(const ClipPlaneAttrib *other, Operation op) const;

private:
  Operation _operation;
  typedef ov_set< PT(PlaneNode) > Planes;
  Planes _planes;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "ClipPlaneAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "clipPlaneAttrib.I"

#endif

