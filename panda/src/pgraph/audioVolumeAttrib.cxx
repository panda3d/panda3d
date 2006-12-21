// Filename: audioVolumeAttrib.cxx
// Created by:  darren (15Dec06)
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

#include "audioVolumeAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraph.h"

TypeHandle AudioVolumeAttrib::_type_handle;
CPT(RenderAttrib) AudioVolumeAttrib::_identity_attrib;

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::Constructor
//       Access: Protected
//  Description: Use AudioVolumeAttrib::make() to construct a new
//               AudioVolumeAttrib object.
////////////////////////////////////////////////////////////////////
AudioVolumeAttrib::
AudioVolumeAttrib(bool off, float volume) :
  _off(off),
  _volume(volume)
{
  nassertv(_volume >= 0.f);
  _has_volume = !IS_NEARLY_EQUAL(_volume, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::make_identity
//       Access: Published, Static
//  Description: Constructs an identity audio volume attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AudioVolumeAttrib::
make_identity() {
  // We make identity a special case and store a pointer forever once
  // we find it the first time.
  if (_identity_attrib == (AudioVolumeAttrib *)NULL) {
    AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, 1.0f);;
    _identity_attrib = return_new(attrib);
  }

  return _identity_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::make
//       Access: Published, Static
//  Description: Constructs a new AudioVolumeAttrib object that indicates
//               audio volume should be scaled by the indicated factor.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AudioVolumeAttrib::
make(float volume) {
  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, volume);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new AudioVolumeAttrib object that ignores
//               any AudioVolumeAttrib inherited from above.  You may
//               also specify an additional volume scale to apply to
//               geometry below (using set_volume()).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AudioVolumeAttrib::
make_off() {
  AudioVolumeAttrib *attrib = 
    new AudioVolumeAttrib(true, 1.0f);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::set_volume
//       Access: Published
//  Description: Returns a new AudioVolumeAttrib, just like this one, but
//               with the volume changed to the indicated value.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AudioVolumeAttrib::
set_volume(float volume) const {
  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(*this);
  assert(volume >= 0.f);
  attrib->_volume = volume;
  attrib->_has_volume = !IS_NEARLY_EQUAL(volume, 1.0f);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AudioVolumeAttrib::
output(ostream &out) const {
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

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived AudioVolumeAttrib
//               types to return a unique number indicating whether
//               this AudioVolumeAttrib is equivalent to the other one.
//
//               This should return 0 if the two AudioVolumeAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two AudioVolumeAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int AudioVolumeAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AudioVolumeAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (is_off() != ta->is_off()) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Comparing " << (int)is_off() << " to " << (int)ta->is_off() << " result = "
        << (int)is_off() - (int)ta->is_off() << "\n";
    }
    
    return (int)is_off() - (int)ta->is_off();
  }

  int result = int(_volume * 1000.0f) - int(ta->_volume * 1000.0f);
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Comparing " << _volume << " to " << ta->_volume << " result = "
      << result << "\n";
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::compose_impl
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
CPT(RenderAttrib) AudioVolumeAttrib::
compose_impl(const RenderAttrib *other) const {
  const AudioVolumeAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (ta->is_off()) {
    return ta;
  }

  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(is_off(), ta->_volume * _volume);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AudioVolumeAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  if (is_off()) {
    return other;
  }
  const AudioVolumeAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  float new_volume = _volume == 0.0f ? 1.0f : ta->_volume / _volume;

  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, new_volume);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived AudioVolumeAttrib
//               types to specify what the default property for a
//               AudioVolumeAttrib of this type should be.
//
//               This should return a newly-allocated AudioVolumeAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of AudioVolumeAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *AudioVolumeAttrib::
make_default_impl() const {
  return new AudioVolumeAttrib(false, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void AudioVolumeAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_audio_volume = this;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               AudioVolumeAttrib.
////////////////////////////////////////////////////////////////////
void AudioVolumeAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void AudioVolumeAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  // We cheat, and modify the bam stream without upping the bam
  // version.  We can do this since we know that no existing bam files
  // have an AudioVolumeAttrib in them.
  dg.add_bool(_off);
  dg.add_float32(_volume);
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type AudioVolumeAttrib is encountered
//               in the Bam file.  It should create the AudioVolumeAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *AudioVolumeAttrib::
make_from_bam(const FactoryParams &params) {
  AudioVolumeAttrib *attrib = new AudioVolumeAttrib(false, 1.0f);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AudioVolumeAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new AudioVolumeAttrib.
////////////////////////////////////////////////////////////////////
void AudioVolumeAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _off = scan.get_bool();
  _volume = scan.get_float32();
  nassertv(_volume >= 0.f);
  _has_volume = !IS_NEARLY_EQUAL(_volume, 1.0f);
}
