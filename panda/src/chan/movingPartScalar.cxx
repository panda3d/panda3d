// Filename: movingPartScalar.cxx
// Created by:  drose (23Feb99)
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


#include "movingPartScalar.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle MovingPartScalar::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::get_blend_value
//       Access: Public
//  Description: Attempts to blend the various scalar values
//               indicated, and sets the _value member to the
//               resulting scalar.
////////////////////////////////////////////////////////////////////
void MovingPartScalar::
get_blend_value(const PartBundle *root) {
  const PartBundle::ChannelBlend &blend = root->get_blend_map();

  if (blend.empty()) {
    // No channel is bound; supply the default value.
    _value = _initial_value;

  } else if (blend.size() == 1) {
    // A single value, the normal case.
    AnimControl *control = (*blend.begin()).first;

    int channel_index = control->get_channel_index();
    nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
    ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
    nassertv(channel != NULL);

    channel->get_value(control->get_frame(), _value);

  } else {
    // A blend of two or more values.
    _value = 0.0f;
    float net = 0.0f;

    PartBundle::ChannelBlend::const_iterator cbi;
    for (cbi = blend.begin(); cbi != blend.end(); ++cbi) {
      AnimControl *control = (*cbi).first;
      float effect = (*cbi).second;
      nassertv(effect != 0.0f);

      int channel_index = control->get_channel_index();
      nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
      ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
      nassertv(channel != NULL);

      ValueType v;
      channel->get_value(control->get_frame(), v);

      _value += v * effect;
      net += effect;
    }

    nassertv(net != 0.0f);
    _value /= net;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::make_MovingPartScalar
//       Access: Protected
//  Description: Factory method to generate a MovingPartScalar object
////////////////////////////////////////////////////////////////////
TypedWritable* MovingPartScalar::
make_MovingPartScalar(const FactoryParams &params)
{
  MovingPartScalar *me = new MovingPartScalar;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a MovingPartScalar object
////////////////////////////////////////////////////////////////////
void MovingPartScalar::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_MovingPartScalar);
}

