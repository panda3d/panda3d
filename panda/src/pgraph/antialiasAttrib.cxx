/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file antialiasAttrib.cxx
 * @author drose
 * @date 2005-01-26
 */

#include "antialiasAttrib.h"
#include "config_pgraph.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle AntialiasAttrib::_type_handle;
int AntialiasAttrib::_attrib_slot;

/**
 * Constructs a new AntialiasAttrib object.
 *
 * The mode should be either M_none, M_auto, or a union of any or all of
 * M_point, M_line, M_polygon, and M_multisample.  Also, in addition to the
 * above choices, it may include either of M_better of M_faster to specify a
 * performance/quality tradeoff hint.
 *
 * If M_none is specified, no antialiasing is performed.
 *
 * If M_multisample is specified, it means to use the special framebuffer
 * multisample bits for antialiasing, if it is available.  If so, the M_point,
 * M_line, and M_polygon modes are ignored.  This advanced antialiasing mode
 * is only available on certain graphics hardware.  If it is not available,
 * the M_multisample bit is ignored (and the other modes may be used instead,
 * if specified).
 *
 * M_point, M_line, and/or M_polygon specify per-primitive smoothing.  When
 * enabled, M_point and M_line may force transparency on.  M_polygon requires
 * a frame buffer that includes an alpha channel, and it works best if the
 * primitives are sorted front-to-back.
 *
 * If M_auto is specified, M_multisample is selected if it is available,
 * otherwise M_polygon is selected, unless drawing lines or points, in which
 * case M_line or M_point is selected (these two generally produce better
 * results than M_multisample)
 */
CPT(RenderAttrib) AntialiasAttrib::
make(unsigned short mode) {
  AntialiasAttrib *attrib = new AntialiasAttrib(mode);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) AntialiasAttrib::
make_default() {
  return RenderAttribRegistry::quick_get_global_ptr()->get_slot_default(_attrib_slot);
}

/**
 *
 */
void AntialiasAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";

  int type = get_mode_type();
  char sep = ' ';

  if (type == M_none) {
    out << " none";

  } else if (type == M_auto) {
    out << " auto";

  } else {
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
    if ((_mode & M_auto) != 0) {
      out << sep << "best";
      sep = '|';
    }
  }

  if ((_mode & M_faster) != 0) {
    out << sep << "faster";
    sep = '|';
  }
  if ((_mode & M_better) != 0) {
    out << sep << "better";
    sep = '|';
  }
}

/**
 * Intended to be overridden by derived AntialiasAttrib types to return a
 * unique number indicating whether this AntialiasAttrib is equivalent to the
 * other one.
 *
 * This should return 0 if the two AntialiasAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two AntialiasAttrib objects whose get_type()
 * functions return the same.
 */
int AntialiasAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AntialiasAttrib *ta = (const AntialiasAttrib *)other;

  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t AntialiasAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_mode);
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
CPT(RenderAttrib) AntialiasAttrib::
compose_impl(const RenderAttrib *other) const {
  const AntialiasAttrib *ta = (const AntialiasAttrib *)other;

  unsigned short mode_type;
  unsigned short mode_quality;

  if (ta->get_mode_type() == M_none || ta->get_mode_type() == M_auto ||
      get_mode_type() == M_auto) {
    // These two special types don't combine: if one of these modes is
    // involved, the lower attrib wins.
    mode_type = ta->get_mode_type();

  } else {
    // Otherwise, the both modes reflect an explicit setting.  In that case,
    // these modes combine in the sensible way, as a union of bits.
    mode_type = get_mode_type() | ta->get_mode_type();
  }

  if (ta->get_mode_quality() != 0) {
    // If any quality is specified on the lower attrib, it wins.
    mode_quality = ta->get_mode_quality();
  } else {
    // Otherwise, the upper quality wins.
    mode_quality = get_mode_quality();
  }

  return make(mode_type | mode_quality);
}

/**
 * Tells the BamReader how to create objects of type AntialiasAttrib.
 */
void AntialiasAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AntialiasAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_uint16(_mode);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type AntialiasAttrib is encountered in the Bam file.  It should create the
 * AntialiasAttrib and extract its information from the file.
 */
TypedWritable *AntialiasAttrib::
make_from_bam(const FactoryParams &params) {
  AntialiasAttrib *attrib = new AntialiasAttrib(M_none);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AntialiasAttrib.
 */
void AntialiasAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = scan.get_uint16();
}

/**
 *
 */
void AntialiasAttrib::
init_type() {
  RenderAttrib::init_type();
  register_type(_type_handle, "AntialiasAttrib",
                RenderAttrib::get_class_type());

  // This is defined here, since we have otherwise no guarantee that the
  // config var has already been constructed by the time we call init_type()
  // at static init time.
  static ConfigVariableBool default_antialias_enable
  ("default-antialias-enable", false,
   PRC_DESC("Set this true to enable the M_auto antialiasing mode for all "
            "nodes by default."));

  _attrib_slot = register_slot(_type_handle, 100,
    new AntialiasAttrib(default_antialias_enable ? M_auto : M_none));
}
