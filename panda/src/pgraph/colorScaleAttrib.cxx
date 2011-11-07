// Filename: colorScaleAttrib.cxx
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "colorScaleAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraph.h"

TypeHandle ColorScaleAttrib::_type_handle;
int ColorScaleAttrib::_attrib_slot;
CPT(RenderAttrib) ColorScaleAttrib::_identity_attrib;

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::Constructor
//       Access: Protected
//  Description: Use ColorScaleAttrib::make() to construct a new
//               ColorScaleAttrib object.
////////////////////////////////////////////////////////////////////
ColorScaleAttrib::
ColorScaleAttrib(bool off, const LVecBase4 &scale) :
  _off(off),
  _scale(scale)
{
  quantize_scale();
  _has_scale = !_scale.almost_equal(LVecBase4(1.0f, 1.0f, 1.0f, 1.0f));
  _has_rgb_scale = !LVecBase3(_scale[0], _scale[1], _scale[2]).almost_equal(LVecBase3(1.0f, 1.0f, 1.0f));
  _has_alpha_scale = !IS_NEARLY_EQUAL(_scale[3], 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make_identity
//       Access: Published, Static
//  Description: Constructs an identity scale attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
make_identity() {
  // We make identity a special case and store a pointer forever once
  // we find it the first time.
  if (_identity_attrib == (ColorScaleAttrib *)NULL) {
    ColorScaleAttrib *attrib = new ColorScaleAttrib(false, LVecBase4(1.0f, 1.0f, 1.0f, 1.0f));;
    _identity_attrib = return_new(attrib);
  }

  return _identity_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ColorScaleAttrib object that indicates
//               geometry should be scaled by the indicated factor.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
make(const LVecBase4 &scale) {
  ColorScaleAttrib *attrib = new ColorScaleAttrib(false, scale);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new ColorScaleAttrib object that ignores
//               any ColorScaleAttrib inherited from above.  You may
//               also specify an additional color scale to apply to
//               geometry below (using set_scale()).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
make_off() {
  ColorScaleAttrib *attrib = 
    new ColorScaleAttrib(true, LVecBase4(1.0f, 1.0f, 1.0f, 1.0f));
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make_default
//       Access: Published, Static
//  Description: Returns a RenderAttrib that corresponds to whatever
//               the standard default properties for render attributes
//               of this type ought to be.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
make_default() {
  return return_new(new ColorScaleAttrib(false, LVecBase4(1.0f, 1.0f, 1.0f, 1.0f)));
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::set_scale
//       Access: Published
//  Description: Returns a new ColorScaleAttrib, just like this one, but
//               with the scale changed to the indicated value.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
set_scale(const LVecBase4 &scale) const {
  ColorScaleAttrib *attrib = new ColorScaleAttrib(*this);
  attrib->_scale = scale;
  attrib->quantize_scale();
  attrib->_has_scale = !scale.almost_equal(LVecBase4(1.0f, 1.0f, 1.0f, 1.0f));
  attrib->_has_rgb_scale = !LVecBase3(scale[0], scale[1], scale[2]).almost_equal(LVecBase3(1.0f, 1.0f, 1.0f));
  attrib->_has_alpha_scale = !IS_NEARLY_EQUAL(scale[3], 1.0f);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::lower_attrib_can_override
//       Access: Public, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return false if a RenderAttrib on a
//               higher node will compose into a RenderAttrib on a
//               lower node that has a higher override value, or false
//               if the lower RenderAttrib will completely replace the
//               state.
//
//               The default behavior is false: normally, a
//               RenderAttrib in the graph cannot completely override
//               a RenderAttrib above it, regardless of its override
//               value--instead, the two attribs are composed.  But
//               for some kinds of RenderAttribs, it is useful to
//               allow this kind of override.
//
//               This method only handles the one special case of a
//               lower RenderAttrib with a higher override value.  If
//               the higher RenderAttrib has a higher override value,
//               it always completely overrides.  And if both
//               RenderAttribs have the same override value, they are
//               always composed.
////////////////////////////////////////////////////////////////////
bool ColorScaleAttrib::
lower_attrib_can_override() const {
  // A ColorScaleAttrib doesn't compose through an override.  This
  // allows us to meaningfully set an override on a lower node, which
  // prevents any color scales from coming in from above.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "off";
  }
  if (has_scale()) {
    out << "(" << get_scale() << ")";

  } else if (!is_off()) {
    out << "identity";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorScaleAttrib
//               types to return a unique number indicating whether
//               this ColorScaleAttrib is equivalent to the other one.
//
//               This should return 0 if the two ColorScaleAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ColorScaleAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ColorScaleAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorScaleAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (is_off() != ta->is_off()) {
    return (int)is_off() - (int)ta->is_off();
  }

  return _scale.compare_to(ta->_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::get_hash_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique hash for these particular
//               properties.  RenderAttribs that compare the same with
//               compare_to_impl(), above, should return the same
//               hash; RenderAttribs that compare differently should
//               return a different hash.
////////////////////////////////////////////////////////////////////
size_t ColorScaleAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)is_off());
  hash = _scale.add_hash(hash);
  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::compose_impl
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
CPT(RenderAttrib) ColorScaleAttrib::
compose_impl(const RenderAttrib *other) const {
  const ColorScaleAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (ta->is_off()) {
    return ta;
  }

  LVecBase4 new_scale(ta->_scale[0] * _scale[0],
                       ta->_scale[1] * _scale[1],
                       ta->_scale[2] * _scale[2],
                       ta->_scale[3] * _scale[3]);
  
  ColorScaleAttrib *attrib = new ColorScaleAttrib(is_off(), new_scale);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  if (is_off()) {
    return other;
  }
  const ColorScaleAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  LVecBase4 new_scale(_scale[0] == 0.0f ? 1.0f : ta->_scale[0] / _scale[0],
                       _scale[1] == 0.0f ? 1.0f : ta->_scale[1] / _scale[1],
                       _scale[2] == 0.0f ? 1.0f : ta->_scale[2] / _scale[2],
                       _scale[3] == 0.0f ? 1.0f : ta->_scale[3] / _scale[3]);

  ColorScaleAttrib *attrib = new ColorScaleAttrib(false, new_scale);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::get_auto_shader_attrib_impl
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
get_auto_shader_attrib_impl(const RenderState *state) const {
  // A ColorScaleAttrib doesn't directly contribute to the auto-shader
  // contents--instead, the shader is always written to query
  // attr_colorscale at runtime.  So the attrib itself means nothing
  // to the shader.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::quantize_scale
//       Access: Private
//  Description: Quantizes the color scale to the nearest multiple of
//               1000, just to prevent runaway accumulation of
//               only slightly-different ColorScaleAttribs.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
quantize_scale() {
  _scale[0] = cfloor(_scale[0] * 1000.0f + 0.5f) * 0.001f;
  _scale[1] = cfloor(_scale[1] * 1000.0f + 0.5f) * 0.001f;
  _scale[2] = cfloor(_scale[2] * 1000.0f + 0.5f) * 0.001f;
  _scale[3] = cfloor(_scale[3] * 1000.0f + 0.5f) * 0.001f;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ColorScaleAttrib.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  // We cheat, and modify the bam stream without upping the bam
  // version.  We can do this since we know that no existing bam files
  // have a ColorScaleAttrib in them.
  dg.add_bool(_off);
  _scale.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ColorScaleAttrib is encountered
//               in the Bam file.  It should create the ColorScaleAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ColorScaleAttrib::
make_from_bam(const FactoryParams &params) {
  ColorScaleAttrib *attrib = new ColorScaleAttrib(false, LVecBase4(1.0f, 1.0f, 1.0f, 1.0f));
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ColorScaleAttrib.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _off = scan.get_bool();
  _scale.read_datagram(scan);
  quantize_scale();
  _has_scale = !_scale.almost_equal(LVecBase4(1.0f, 1.0f, 1.0f, 1.0f));
  _has_rgb_scale = !LVecBase3(_scale[0], _scale[1], _scale[2]).almost_equal(LVecBase3(1.0f, 1.0f, 1.0f));
  _has_alpha_scale = !IS_NEARLY_EQUAL(_scale[3], 1.0f);
}
