// Filename: antialiasAttrib.cxx
// Created by:  drose (26Jan05)
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

#include "antialiasAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle AntialiasAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::make
//       Access: Published, Static
//  Description: Constructs a new AntialiasAttrib object.
//
//               The mode should be either M_none, M_best, or a union
//               of any or all of M_point, M_line, M_polygon, and
//               M_multisample.
//
//               If M_none is specified, no antialiasing is performed.  
//
//               If M_best is specified, M_multisample is selected if
//               it is available, otherwise M_polygon is selected,
//               unless drawing lines or points, in which case M_line
//               or M_point is selected (these two generally produce
//               better results than M_multisample)
//
//               In the explicit form, it enables all of the specified
//               multisample modes.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AntialiasAttrib::
make(unsigned short mode) {
  AntialiasAttrib *attrib = new AntialiasAttrib(mode);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void AntialiasAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_antialias(this);
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AntialiasAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (_mode == M_none) {
    out << " none";

  } else if (_mode == M_best) {
    out << " best";

  } else {
    char sep = ' ';
    if ((_mode & M_point) != 0) {
      out << sep << "point";
      sep = '|';
    }
    if ((_mode & M_line) != 0) {
      out << sep << "line";
      sep = '|';
    }
    if ((_mode & M_polygon) != 0) {
      out << sep << "polygon";
      sep = '|';
    }
    if ((_mode & M_best) != 0) {
      out << sep << "best";
      sep = '|';
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived AntialiasAttrib
//               types to return a unique number indicating whether
//               this AntialiasAttrib is equivalent to the other one.
//
//               This should return 0 if the two AntialiasAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two AntialiasAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int AntialiasAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AntialiasAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::compose_impl
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
CPT(RenderAttrib) AntialiasAttrib::
compose_impl(const RenderAttrib *other) const {
  const AntialiasAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  if (ta->get_mode() == M_none || ta->get_mode() == M_best ||
      get_mode() == M_none || get_mode() == M_best) {
    // These two special modes don't combine: if one of these modes is
    // involved, the lower attrib wins.
    return ta;
  }

  // Otherwise, both attribs reflect an explicit setting.  In that
  // case, these modes combine in the sensible way, as a union of
  // bits.
  return make(get_mode() | ta->get_mode());
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived AntialiasAttrib
//               types to specify what the default property for a
//               AntialiasAttrib of this type should be.
//
//               This should return a newly-allocated AntialiasAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of AntialiasAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *AntialiasAttrib::
make_default_impl() const {
  return new AntialiasAttrib(M_none);
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               AntialiasAttrib.
////////////////////////////////////////////////////////////////////
void AntialiasAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void AntialiasAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint16(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type AntialiasAttrib is encountered
//               in the Bam file.  It should create the AntialiasAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *AntialiasAttrib::
make_from_bam(const FactoryParams &params) {
  AntialiasAttrib *attrib = new AntialiasAttrib(M_none);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AntialiasAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new AntialiasAttrib.
////////////////////////////////////////////////////////////////////
void AntialiasAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = scan.get_uint16();
}
