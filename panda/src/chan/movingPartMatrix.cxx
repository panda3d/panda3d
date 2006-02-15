// Filename: movingPartMatrix.cxx
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


#include "movingPartMatrix.h"

#include "compose_matrix.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

TypeHandle MovingPartMatrix::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MovingPartMatrix::
~MovingPartMatrix() {
}


////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::make_initial_channel
//       Access: Public, Virtual
//  Description: Creates and returns a new AnimChannel that is not
//               part of any hierarchy, but that returns the default
//               value associated with this part.
////////////////////////////////////////////////////////////////////
AnimChannelBase *MovingPartMatrix::
make_initial_channel() const {
  return new AnimChannelMatrixFixed(get_name(), _initial_value);
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::get_blend_value
//       Access: Public
//  Description: Attempts to blend the various matrix values
//               indicated, and sets the _value member to the
//               resulting matrix.
////////////////////////////////////////////////////////////////////
void MovingPartMatrix::
get_blend_value(const PartBundle *root) {
  PartBundle::CDReader cdata(root->_cycler);

  if (cdata->_blend.empty()) {
    // No channel is bound; supply the default value.
    _value = _initial_value;

  } else if (cdata->_blend.size() == 1) {
    // A single value, the normal case.
    AnimControl *control = (*cdata->_blend.begin()).first;

    int channel_index = control->get_channel_index();
    nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
    ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
    if (channel == (ChannelType *)NULL) {
      // Nothing is actually bound here.
      _value = _initial_value;

    } else {
      channel->get_value(control->get_frame(), _value);
    }

  } else {
    // A blend of two or more values.

    switch (cdata->_blend_type) {
    case PartBundle::BT_single:
    case PartBundle::BT_linear:
      {
        // An ordinary, linear blend.
        _value = 0.0f;
        float net = 0.0f;
        
        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          float effect = (*cbi).second;
          nassertv(effect != 0.0f);
          
          int channel_index = control->get_channel_index();
          nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
          ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
          if (channel != (ChannelType *)NULL) {
            ValueType v;
            channel->get_value(control->get_frame(), v);
            
            _value += v * effect;
            net += effect;
          }
        }
        
        if (net == 0.0f) {
          _value = _initial_value;
        } else {
          _value /= net;
        }
      }
      break;

    case PartBundle::BT_normalized_linear:
      {
        // A normalized linear blend.  This means we do a linear blend
        // without scales or shears, normalize the scale and shear
        // components of the resulting matrix to eliminate
        // artificially-introduced scales, and then reapply the
        // scales and shears.
        
        _value = 0.0f;
        LVecBase3f scale(0.0f, 0.0f, 0.0f);
        LVecBase3f shear(0.0f, 0.0f, 0.0f);
        float net = 0.0f;
        
        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          float effect = (*cbi).second;
          nassertv(effect != 0.0f);
          
          int channel_index = control->get_channel_index();
          nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
          ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
          if (channel != (ChannelType *)NULL) {
            ValueType v;
            channel->get_value_no_scale_shear(control->get_frame(), v);
            LVecBase3f iscale, ishear;
            channel->get_scale(control->get_frame(), iscale);
            channel->get_shear(control->get_frame(), ishear);
            
            _value += v * effect;
            scale += iscale * effect;
            shear += ishear * effect;
            net += effect;
          }
        }
        
        if (net == 0.0f) {
          _value = _initial_value;

        } else {
          _value /= net;
          scale /= net;
          shear /= net;
          
          // Now rebuild the matrix with the correct scale values.
          
          LVector3f false_scale, false_shear, hpr, translate;
          decompose_matrix(_value, false_scale, false_shear, hpr, translate);
          compose_matrix(_value, scale, shear, hpr, translate);
        }
      }
      break;

    case PartBundle::BT_componentwise:
      {
        // Componentwise linear, including componentwise H, P, and R.
        LVecBase3f scale(0.0f, 0.0f, 0.0f);
        LVecBase3f hpr(0.0f, 0.0f, 0.0f);
        LVecBase3f pos(0.0f, 0.0f, 0.0f);
        LVecBase3f shear(0.0f, 0.0f, 0.0f);
        float net = 0.0f;
        
        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          float effect = (*cbi).second;
          nassertv(effect != 0.0f);
          
          int channel_index = control->get_channel_index();
          nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
          ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
          if (channel != (ChannelType *)NULL) {
            LVecBase3f iscale, ihpr, ipos, ishear;
            channel->get_scale(control->get_frame(), iscale);
            channel->get_hpr(control->get_frame(), ihpr);
            channel->get_pos(control->get_frame(), ipos);
            channel->get_shear(control->get_frame(), ishear);
            
            scale += iscale * effect;
            hpr += ihpr * effect;
            pos += ipos * effect;
            shear += ishear * effect;
            net += effect;
          }
        }
        
        if (net == 0.0f) {
          _value = _initial_value;

        } else {
          scale /= net;
          hpr /= net;
          pos /= net;
          shear /= net;
          
          compose_matrix(_value, scale, shear, hpr, pos);
        }
      }
      break;

    case PartBundle::BT_componentwise_quat:
      {
        // Componentwise linear, except for rotation, which is a
        // quaternion.
        LVecBase3f scale(0.0f, 0.0f, 0.0f);
        LQuaternionf quat(0.0f, 0.0f, 0.0f, 0.0f);
        LVecBase3f pos(0.0f, 0.0f, 0.0f);
        LVecBase3f shear(0.0f, 0.0f, 0.0f);
        float net = 0.0f;
        
        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          float effect = (*cbi).second;
          nassertv(effect != 0.0f);
          
          int channel_index = control->get_channel_index();
          nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
          ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
          if (channel != (ChannelType *)NULL) {
            LVecBase3f iscale, ipos, ishear;
            LQuaternionf iquat;
            channel->get_scale(control->get_frame(), iscale);
            channel->get_quat(control->get_frame(), iquat);
            channel->get_pos(control->get_frame(), ipos);
            channel->get_shear(control->get_frame(), ishear);
            
            scale += iscale * effect;
            quat += iquat * effect;
            pos += ipos * effect;
            shear += ishear * effect;
            net += effect;
          }
        }
        
        if (net == 0.0f) {
          _value = _initial_value;

        } else {
          scale /= net;
          quat /= net;
          pos /= net;
          shear /= net;
          
          // There should be no need to normalize the quaternion,
          // assuming all of the input quaternions were already
          // normalized.
          
          _value = LMatrix4f::scale_shear_mat(scale, shear) * quat;
          _value.set_row(3, pos);
        }
      }
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::make_MovingPartMatrix
//       Access: Protected
//  Description: Factory method to generate a MovingPartMatrix object
////////////////////////////////////////////////////////////////////
TypedWritable* MovingPartMatrix::
make_MovingPartMatrix(const FactoryParams &params)
{
  MovingPartMatrix *me = new MovingPartMatrix;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a MovingPartMatrix object
////////////////////////////////////////////////////////////////////
void MovingPartMatrix::
register_with_read_factory()
{
  BamReader::get_factory()->register_factory(get_class_type(), make_MovingPartMatrix);
}
