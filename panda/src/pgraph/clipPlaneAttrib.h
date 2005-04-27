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
#include "nodePath.h"

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
  INLINE ClipPlaneAttrib(const ClipPlaneAttrib &copy);

PUBLISHED:

  // This is the old, deprecated interface to ClipPlaneAttrib.  Do not
  // use any of these methods for new code; these methods will be
  // removed soon.
  enum Operation {
    O_set,
    O_add,
    O_remove
  };

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

  Operation get_operation() const;

  int get_num_planes() const;
  PlaneNode *get_plane(int n) const;
  bool has_plane(PlaneNode *plane) const;

  CPT(RenderAttrib) add_plane(PlaneNode *plane) const;
  CPT(RenderAttrib) remove_plane(PlaneNode *plane) const;


  // The following is the new, more general interface to the
  // ClipPlaneAttrib.
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make_all_off();

  INLINE int get_num_on_planes() const;
  INLINE NodePath get_on_plane(int n) const;
  INLINE bool has_on_plane(const NodePath &plane) const;

  INLINE int get_num_off_planes() const;
  INLINE NodePath get_off_plane(int n) const;
  INLINE bool has_off_plane(const NodePath &plane) const;
  INLINE bool has_all_off() const;

  INLINE bool is_identity() const;

  CPT(RenderAttrib) add_on_plane(const NodePath &plane) const;
  CPT(RenderAttrib) remove_on_plane(const NodePath &plane) const;
  CPT(RenderAttrib) add_off_plane(const NodePath &plane) const;
  CPT(RenderAttrib) remove_off_plane(const NodePath &plane) const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  typedef ov_set<NodePath> Planes;
  Planes _on_planes, _off_planes;
  bool _off_all_planes;

  static CPT(RenderAttrib) _empty_attrib;
  static CPT(RenderAttrib) _all_off_attrib;

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

