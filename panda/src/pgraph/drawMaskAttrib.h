// Filename: drawMaskAttrib.h
// Created by:  drose (28Sep05)
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

#ifndef DRAWMASKATTRIB_H
#define DRAWMASKATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "drawMask.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : DrawMaskAttrib
// Description : This attrib can be used to control the visibility of
//               certain Geoms from certain cameras.  It is similar in
//               principle to the PandaNode::set_draw_mask()
//               interface, except it does not cause an early prune in
//               the cull traversal; thus, it can be used to show a
//               node even though its parent has been hidden (if the
//               parent was hidden using the same interface).
//
//               It is mainly useful for unusual circumstances in
//               which the visibility of a node is not easy to
//               determine from examining the static hierarchy of the
//               graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DrawMaskAttrib : public RenderAttrib {
protected:
  INLINE DrawMaskAttrib(DrawMask new_mask, DrawMask bits_to_change);
  INLINE DrawMaskAttrib(const DrawMaskAttrib &copy);

PUBLISHED:
  INLINE static CPT(RenderAttrib) make_hide(DrawMask draw_mask = DrawMask::all_on());
  INLINE static CPT(RenderAttrib) make_show(DrawMask draw_mask = DrawMask::all_on());
  static CPT(RenderAttrib) make(DrawMask new_mask, DrawMask bits_to_change);

  INLINE DrawMask get_new_mask() const;
  INLINE DrawMask get_bits_to_change() const;

public:
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  DrawMask _new_mask;
  DrawMask _bits_to_change;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "DrawMaskAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "drawMaskAttrib.I"

#endif

