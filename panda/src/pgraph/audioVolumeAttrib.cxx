/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioVolumeAttrib.cxx
 * @author darren
 * @date 2006-12-15
 */

#include "audioVolumeAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraph.h"

CPT(RenderAttrib) AudioVolumeAttrib::_identity_attrib;
TypeHandle AudioVolumeAttrib::_type_handle;
int AudioVolumeAttrib::_attrib_slot;

/**
 * Use AudioVolumeAttrib::make() to construct a new AudioVolumeAttrib object.
 */
AudioVolumeAttrib::
AudioVolumeAttrib(bool off, PN_stdfloat volume) :
  _off(off),
  _volume(volume)
{
  nassertv(_volume >= 0.f);
  _has_volume = !IS_NEARLY_EQUAL(_volume, 1.0f);
}

/**
 * Constructs an identity audio volume attrib.
 */
CPT(RenderAttrib) AudioVolumeAttrib::
make_identity() {
  // We make identity a special case and store a pointer forever once we find
  // it the first time.
  if (_identity_attrib == nullptr) {
    AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, 1.0f);;
    _identity_attrib = return_new(attrib);
  }

  return _identity_attrib;
}

/**
 * Constructs a new AudioVolumeAttrib object that indicates audio volume
 * should be scaled by the indicated factor.
 */
CPT(RenderAttrib) AudioVolumeAttrib::
make(PN_stdfloat volume) {
  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, volume);
  return return_new(attrib);
}

/**
 * Constructs a new AudioVolumeAttrib object that ignores any
 * AudioVolumeAttrib inherited from above.  You may also specify an additional
 * volume scale to apply to geometry below (using set_volume()).
 */
CPT(RenderAttrib) AudioVolumeAttrib::
make_off() {
  AudioVolumeAttrib *attrib =
    new AudioVolumeAttrib(true, 1.0f);
  return return_new(attrib);
}

/**
 * Returns a RenderAttrib that corresponds to whatever the standard default
 * properties for render attributes of this type ought to be.
 */
CPT(RenderAttrib) AudioVolumeAttrib::
make_default() {
  return return_new(new AudioVolumeAttrib(false, 1.0f));
}

/**
 * Returns a new AudioVolumeAttrib, just like this one, but with the volume
 * changed to the indicated value.
 */
CPT(RenderAttrib) AudioVolumeAttrib::
set_volume(PN_stdfloat volume) const {
  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(*this);
  assert(volume >= 0.f);
  attrib->_volume = volume;
  attrib->_has_volume = !IS_NEARLY_EQUAL(volume, 1.0f);
  return return_new(attrib);
}

/**
 *
 */
void AudioVolumeAttrib::
output(std::ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "off";
  }
  if (has_volume()) {
    out << "(" << get_volume() << ")";

  } else if (!is_off()) {
    out << "identity";
  }
}

/**
 * Intended to be overridden by derived AudioVolumeAttrib types to return a
 * unique number indicating whether this AudioVolumeAttrib is equivalent to
 * the other one.
 *
 * This should return 0 if the two AudioVolumeAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two AudioVolumeAttrib objects whose
 * get_type() functions return the same.
 */
int AudioVolumeAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AudioVolumeAttrib *ta = (const AudioVolumeAttrib *)other;

  if (_off != ta->_off) {
    return (int)_off - (int)ta->_off;
  }

  if (_volume != ta->_volume) {
    return _volume < ta->_volume ? -1 : 1;
  }

  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t AudioVolumeAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, (int)_off);
  hash = float_hash().add_hash(hash, _volume);
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
CPT(RenderAttrib) AudioVolumeAttrib::
compose_impl(const RenderAttrib *other) const {
  const AudioVolumeAttrib *ta = (const AudioVolumeAttrib *)other;

  if (ta->is_off()) {
    return ta;
  }

  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(is_off(), ta->_volume * _volume);
  return return_new(attrib);
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) AudioVolumeAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  if (is_off()) {
    return other;
  }
  const AudioVolumeAttrib *ta = (const AudioVolumeAttrib *)other;

  PN_stdfloat new_volume = _volume == 0.0f ? 1.0f : ta->_volume / _volume;

  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, new_volume);
  return return_new(attrib);
}

/**
 * Tells the BamReader how to create objects of type AudioVolumeAttrib.
 */
void AudioVolumeAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void AudioVolumeAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  // We cheat, and modify the bam stream without upping the bam version.  We
  // can do this since we know that no existing bam files have an
  // AudioVolumeAttrib in them.
  dg.add_bool(_off);
  dg.add_stdfloat(_volume);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type AudioVolumeAttrib is encountered in the Bam file.  It should create
 * the AudioVolumeAttrib and extract its information from the file.
 */
TypedWritable *AudioVolumeAttrib::
make_from_bam(const FactoryParams &params) {
  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, 1.0f);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new AudioVolumeAttrib.
 */
void AudioVolumeAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _off = scan.get_bool();
  _volume = scan.get_stdfloat();
  nassertv(_volume >= 0.f);
  _has_volume = !IS_NEARLY_EQUAL(_volume, 1.0f);
}
