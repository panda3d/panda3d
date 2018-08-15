/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderModeAttrib.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "renderModeAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle RenderModeAttrib::_type_handle;
int RenderModeAttrib::_attrib_slot;

/**
 * Constructs a new RenderModeAttrib object that specifies whether to draw
 * polygons in the normal, filled mode, or wireframe mode, or in some other
 * yet-to-be-defined mode.
 *
 * The thickness parameter specifies the thickness to be used for wireframe
 * lines, as well as for ordinary linestrip lines; it also specifies the
 * diameter of points.  (Thick lines are presently only supported in OpenGL;
 * but thick points are supported on either platform.)
 *
 * If perspective is true, the point thickness represented is actually a width
 * in 3-d units, and the points should scale according to perspective.  When
 * it is false, the point thickness is actually a width in pixels, and points
 * are a uniform screen size regardless of distance from the camera.
 *
 * In M_filled_wireframe mode, you should also specify the wireframe_color,
 * indicating the flat color to assign to the overlayed wireframe.
 */
CPT(RenderAttrib) RenderModeAttrib::
make(RenderModeAttrib::Mode mode, PN_stdfloat thickness,
     bool perspective, const LColor &wireframe_color) {
  RenderModeAttrib *attrib = new RenderModeAttrib(mode, thickness, perspective, wireframe_color);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) RenderModeAttrib::
make_default() {
  return return_new(new RenderModeAttrib(M_filled, 1.0f, false));
}

/**
 *
 */
void RenderModeAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_unchanged:
    out << "unchanged";
    break;

  case M_filled:
    out << "filled";
    break;

  case M_wireframe:
    out << "wireframe(" << get_thickness() << ")";
    break;

  case M_point:
    out << "point(" << get_thickness() << ")";
    break;

  case M_filled_flat:
    out << "filled_flat";
    break;

  case M_filled_wireframe:
    out << "filled_wireframe(" << get_wireframe_color() << ")";
    break;
  }

  if (get_thickness() != 1.0f) {
    out << ", thick " << get_thickness();
  }

  if (get_perspective()) {
    out << ", perspective";
  }
}

/**
 * Intended to be overridden by derived RenderModeAttrib types to return a
 * unique number indicating whether this RenderModeAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two RenderModeAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two RenderModeAttrib objects whose get_type()
 * functions return the same.
 */
int RenderModeAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const RenderModeAttrib *ta = (const RenderModeAttrib *)other;

  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }
  if (_thickness != ta->_thickness) {
    return _thickness < ta->_thickness ? -1 : 1;
  }
  if (_perspective != ta->_perspective) {
    return (int)_perspective - (int)ta->_perspective;
  }
  if (_mode == M_filled_wireframe && _wireframe_color != ta->_wireframe_color) {
    return _wireframe_color.compare_to(ta->_wireframe_color);
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t RenderModeAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
  hash = float_hash().add_hash(hash, _thickness);
  hash = int_hash::add_hash(hash, (int)_perspective);
  if (_mode == M_filled_wireframe) {
    hash = _wireframe_color.add_hash(hash);
  }
  return hash;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return the result of applying the other RenderAttrib to a node
 * in the scene graph below this RenderAttrib, which was already applied.  In
 * most cases, the result is the same as the other RenderAttrib (that is, a
 * subsequent RenderAttrib completely replaces the preceding one).  On the
 * other hand, some kinds of RenderAttrib (for instance, ColorTransformAttrib)
 * might combine in meaningful ways.
 */
CPT(RenderAttrib) RenderModeAttrib::
compose_impl(const RenderAttrib *other) const {
  const RenderModeAttrib *ta = (const RenderModeAttrib *)other;

  // The special mode M_unchanged means to keep the current mode.
  Mode mode = ta->get_mode();
  if (mode == M_unchanged) {
    mode = get_mode();
  }

  return make(mode, ta->get_thickness(), ta->get_perspective(),
              ta->get_wireframe_color());
}

/**
 * Tells the BamReader how to create objects of type RenderModeAttrib.
 */
void RenderModeAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RenderModeAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  dg.add_stdfloat(_thickness);
  dg.add_bool(_perspective);

  if (_mode == M_filled_wireframe) {
    _wireframe_color.write_datagram(dg);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type RenderModeAttrib is encountered in the Bam file.  It should create the
 * RenderModeAttrib and extract its information from the file.
 */
TypedWritable *RenderModeAttrib::
make_from_bam(const FactoryParams &params) {
  RenderModeAttrib *attrib = new RenderModeAttrib(M_filled, 1.0f, false);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RenderModeAttrib.
 */
void RenderModeAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
  _thickness = scan.get_stdfloat();
  _perspective = scan.get_bool();

  if (_mode == M_filled_wireframe) {
    _wireframe_color.read_datagram(scan);
  }
}
