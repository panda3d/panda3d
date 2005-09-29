// Filename: drawMaskAttrib.cxx
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

#include "drawMaskAttrib.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "cullTraverser.h"
#include "config_pgraph.h"

TypeHandle DrawMaskAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::make
//       Access: Published, Static
//  Description: Constructs a new DrawMaskAttrib that changes the
//               bits_to_change bits in the current DrawMask to the
//               values of the corresponding bits in new_mask.  Only
//               those bits in common with bits_to_change are
//               affected.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DrawMaskAttrib::
make(DrawMask new_mask, DrawMask bits_to_change) {
  DrawMaskAttrib *attrib = new DrawMaskAttrib(new_mask, bits_to_change);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DrawMaskAttrib::
output(ostream &out) const {
  out << get_type() << ":" << get_new_mask() << "/" << get_bits_to_change();
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DrawMaskAttrib
//               types to return a unique number indicating whether
//               this DrawMaskAttrib is equivalent to the other one.
//
//               This should return 0 if the two DrawMaskAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two DrawMaskAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int DrawMaskAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const DrawMaskAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  int compare = get_new_mask().compare_to(ta->get_new_mask());
  if (compare != 0) {
    return compare;
  }
  compare = get_bits_to_change().compare_to(ta->get_bits_to_change());
  return compare;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DrawMaskAttrib::
compose_impl(const RenderAttrib *other) const {
  const DrawMaskAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  
  DrawMask mask = get_new_mask();
  mask = (mask & ~ta->get_bits_to_change()) | ta->get_new_mask();

  DrawMaskAttrib *attrib = new DrawMaskAttrib(mask, DrawMask::all_on());
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DrawMaskAttrib
//               types to specify what the default property for a
//               DrawMaskAttrib of this type should be.
//
//               This should return a newly-allocated DrawMaskAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of DrawMaskAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *DrawMaskAttrib::
make_default_impl() const {
  return new DrawMaskAttrib(DrawMask::all_on(), DrawMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void DrawMaskAttrib::
store_into_slot(AttribSlots *) const {
  // There's no need to store a DrawMaskAttrib at the moment, since it
  // doesn't actually change any rendering state.
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool DrawMaskAttrib::
has_cull_callback() const {
  return (_new_mask != DrawMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.
//
//               This is called each time the RenderAttrib is
//               discovered applied to a Geom in the traversal.  It
//               should return true if the Geom is visible, false if
//               it should be omitted.
////////////////////////////////////////////////////////////////////
bool DrawMaskAttrib::
cull_callback(CullTraverser *trav, const CullTraverserData &) const {
  return (trav->get_camera_mask() & _new_mask) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               DrawMaskAttrib.
////////////////////////////////////////////////////////////////////
void DrawMaskAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void DrawMaskAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint32(_new_mask.get_word());
  dg.add_uint32(_bits_to_change.get_word());
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type DrawMaskAttrib is encountered
//               in the Bam file.  It should create the DrawMaskAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *DrawMaskAttrib::
make_from_bam(const FactoryParams &params) {
  DrawMaskAttrib *attrib = new DrawMaskAttrib(0, 0);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawMaskAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new DrawMaskAttrib.
////////////////////////////////////////////////////////////////////
void DrawMaskAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _new_mask.set_word(scan.get_uint32());
  _bits_to_change.set_word(scan.get_uint32());
}
