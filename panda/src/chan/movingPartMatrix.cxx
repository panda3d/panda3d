/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartMatrix.cxx
 * @author drose
 * @date 1999-02-23
 */

#include "movingPartMatrix.h"
#include "animChannelMatrixDynamic.h"
#include "animChannelMatrixFixed.h"
#include "compose_matrix.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_chan.h"

template class MovingPart<ACMatrixSwitchType>;

TypeHandle MovingPartMatrix::_type_handle;

/**
 *
 */
MovingPartMatrix::
~MovingPartMatrix() {
}


/**
 * Creates and returns a new AnimChannel that is not part of any hierarchy,
 * but that returns the default value associated with this part.
 */
AnimChannelBase *MovingPartMatrix::
make_default_channel() const {
  LVecBase3 pos, hpr, scale, shear;
  decompose_matrix(_default_value, pos, hpr, scale, shear);
  return new AnimChannelMatrixFixed(get_name(), pos, hpr, scale);
}

/**
 * Attempts to blend the various matrix values indicated, and sets the _value
 * member to the resulting matrix.
 */
void MovingPartMatrix::
get_blend_value(const PartBundle *root) {
  // If a forced channel is set on this particular joint, we always return
  // that value instead of performing the blend.  Furthermore, the frame
  // number is always 0 for the forced channel.
  if (_forced_channel != nullptr) {
    ChannelType *channel = DCAST(ChannelType, _forced_channel);
    channel->get_value(0, _value);
    return;
  }

  PartBundle::CDReader cdata(root->_cycler);

  if (cdata->_blend.empty()) {
    // No channel is bound; supply the default value.
    if (restore_initial_pose) {
      _value = _default_value;
    }

  } else if (_effective_control != nullptr &&
             !cdata->_frame_blend_flag) {
    // A single value, the normal case.
    ChannelType *channel = DCAST(ChannelType, _effective_channel);
    channel->get_value(_effective_control->get_frame(), _value);

  } else {
    // A blend of two or more values, either between multiple different
    // animations, or between consecutive frames of the same animation (or
    // both).
    switch (cdata->_blend_type) {
    case PartBundle::BT_linear:
      {
        // An ordinary, linear blend.
        LMatrix4 net_value = LMatrix4::zeros_mat();
        PN_stdfloat net_effect = 0.0f;

        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          PN_stdfloat effect = (*cbi).second;
          nassertv(effect != 0.0f);

          int channel_index = control->get_channel_index();
          nassertv(channel_index >= 0 && channel_index < (int)_channels.size());
          ChannelType *channel = DCAST(ChannelType, _channels[channel_index]);
          if (channel != nullptr) {
            ValueType v;
            channel->get_value(control->get_frame(), v);

            if (!cdata->_frame_blend_flag) {
              // Hold the current frame until the next one is ready.
              net_value += v * effect;
            } else {
              // Blend between successive frames.
              PN_stdfloat frac = (PN_stdfloat)control->get_frac();
              net_value += v * (effect * (1.0f - frac));

              channel->get_value(control->get_next_frame(), v);
              net_value += v * (effect * frac);
            }
            net_effect += effect;
          }
        }

        if (net_effect == 0.0f) {
          if (restore_initial_pose) {
            _value = _default_value;
          }
        } else {
          _value = net_value / net_effect;
        }
      }
      break;

    case PartBundle::BT_normalized_linear:
      {
        // A normalized linear blend.  This means we do a linear blend without
        // scales or shears, normalize the scale and shear components of the
        // resulting matrix to eliminate artificially-introduced scales, and
        // then reapply the scales and shears.

        LMatrix4 net_value = LMatrix4::zeros_mat();
        LVecBase3 scale(0.0f, 0.0f, 0.0f);
        LVecBase3 shear(0.0f, 0.0f, 0.0f);
        PN_stdfloat net_effect = 0.0f;

        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          PN_stdfloat effect = (*cbi).second;
          nassertv(effect != 0.0f);

          ChannelType *channel = nullptr;
          int channel_index = control->get_channel_index();
          if (channel_index >= 0 && channel_index < (int)_channels.size()) {
            channel = DCAST(ChannelType, _channels[channel_index]);
          }
          if (channel != nullptr) {
            int frame = control->get_frame();
            ValueType v;
            LVecBase3 iscale, ishear;
            channel->get_value_no_scale_shear(frame, v);
            channel->get_scale(frame, iscale);
            channel->get_shear(frame, ishear);

            if (!cdata->_frame_blend_flag) {
              // Hold the current frame until the next one is ready.
              net_value += v * effect;
              scale += iscale * effect;
              shear += ishear * effect;
            } else {
              // Blend between successive frames.
              PN_stdfloat frac = (PN_stdfloat)control->get_frac();
              PN_stdfloat e0 = effect * (1.0f - frac);
              net_value += v * e0;
              scale += iscale * e0;
              shear += ishear * e0;

              int next_frame = control->get_next_frame();
              channel->get_value_no_scale_shear(next_frame, v);
              channel->get_scale(next_frame, iscale);
              channel->get_shear(next_frame, ishear);
              PN_stdfloat e1 = effect * frac;
              net_value += v * e1;
              scale += iscale * e1;
              shear += ishear * e1;
            }
            net_effect += effect;
          }
        }

        if (net_effect == 0.0f) {
          if (restore_initial_pose) {
            _value = _default_value;
          }

        } else {
          net_value /= net_effect;
          scale /= net_effect;
          shear /= net_effect;

          // Now rebuild the matrix with the correct scale values.

          LVector3 false_scale, false_shear, hpr, translate;
          decompose_matrix(net_value, false_scale, false_shear, hpr, translate);
          compose_matrix(_value, scale, shear, hpr, translate);
        }
      }
      break;

    case PartBundle::BT_componentwise:
      {
        // Componentwise linear, including componentwise H, P, and R.
        LVecBase3 scale(0.0f, 0.0f, 0.0f);
        LVecBase3 hpr(0.0f, 0.0f, 0.0f);
        LVecBase3 pos(0.0f, 0.0f, 0.0f);
        LVecBase3 shear(0.0f, 0.0f, 0.0f);
        PN_stdfloat net_effect = 0.0f;

        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          PN_stdfloat effect = (*cbi).second;
          nassertv(effect != 0.0f);

          ChannelType *channel = nullptr;
          int channel_index = control->get_channel_index();
          if (channel_index >= 0 && channel_index < (int)_channels.size()) {
            channel = DCAST(ChannelType, _channels[channel_index]);
          }
          if (channel != nullptr) {
            int frame = control->get_frame();
            LVecBase3 iscale, ihpr, ipos, ishear;
            channel->get_scale(frame, iscale);
            channel->get_hpr(frame, ihpr);
            channel->get_pos(frame, ipos);
            channel->get_shear(frame, ishear);

            if (!cdata->_frame_blend_flag) {
              // Hold the current frame until the next one is ready.
              scale += iscale * effect;
              hpr += ihpr * effect;
              pos += ipos * effect;
              shear += ishear * effect;
            } else {
              // Blend between successive frames.
              PN_stdfloat frac = (PN_stdfloat)control->get_frac();
              PN_stdfloat e0 = effect * (1.0f - frac);

              scale += iscale * e0;
              hpr += ihpr * e0;
              pos += ipos * e0;
              shear += ishear * e0;

              int next_frame = control->get_next_frame();
              channel->get_scale(next_frame, iscale);
              channel->get_hpr(next_frame, ihpr);
              channel->get_pos(next_frame, ipos);
              channel->get_shear(next_frame, ishear);
              PN_stdfloat e1 = effect * frac;

              scale += iscale * e1;
              hpr += ihpr * e1;
              pos += ipos * e1;
              shear += ishear * e1;
            }
            net_effect += effect;
          }
        }

        if (net_effect == 0.0f) {
          if (restore_initial_pose) {
            _value = _default_value;
          }

        } else {
          scale /= net_effect;
          hpr /= net_effect;
          pos /= net_effect;
          shear /= net_effect;

          compose_matrix(_value, scale, shear, hpr, pos);
        }
      }
      break;

    case PartBundle::BT_componentwise_quat:
      {
        // Componentwise linear, except for rotation, which is a quaternion.
        LVecBase3 scale(0.0f, 0.0f, 0.0f);
        LQuaternion quat(0.0f, 0.0f, 0.0f, 0.0f);
        LVecBase3 pos(0.0f, 0.0f, 0.0f);
        LVecBase3 shear(0.0f, 0.0f, 0.0f);
        PN_stdfloat net_effect = 0.0f;

        PartBundle::ChannelBlend::const_iterator cbi;
        for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
          AnimControl *control = (*cbi).first;
          PN_stdfloat effect = (*cbi).second;
          nassertv(effect != 0.0f);

          ChannelType *channel = nullptr;
          int channel_index = control->get_channel_index();
          if (channel_index >= 0 && channel_index < (int)_channels.size()) {
            channel = DCAST(ChannelType, _channels[channel_index]);
          }
          if (channel != nullptr) {
            int frame = control->get_frame();
            LVecBase3 iscale, ipos, ishear;
            LQuaternion iquat;
            channel->get_scale(frame, iscale);
            channel->get_quat(frame, iquat);
            channel->get_pos(frame, ipos);
            channel->get_shear(frame, ishear);

            if (!cdata->_frame_blend_flag) {
              // Hold the current frame until the next one is ready.
              scale += iscale * effect;
              quat += iquat * effect;
              pos += ipos * effect;
              shear += ishear * effect;

            } else {
              // Blend between successive frames.
              PN_stdfloat frac = (PN_stdfloat)control->get_frac();
              PN_stdfloat e0 = effect * (1.0f - frac);

              scale += iscale * e0;
              quat += iquat * e0;
              pos += ipos * e0;
              shear += ishear * e0;

              int next_frame = control->get_next_frame();
              channel->get_scale(next_frame, iscale);
              channel->get_quat(next_frame, iquat);
              channel->get_pos(next_frame, ipos);
              channel->get_shear(next_frame, ishear);
              PN_stdfloat e1 = effect * frac;

              scale += iscale * e1;
              quat += iquat * e1;
              pos += ipos * e1;
              shear += ishear * e1;
            }
            net_effect += effect;
          }
        }

        if (net_effect == 0.0f) {
          if (restore_initial_pose) {
            _value = _default_value;
          }

        } else {
          scale /= net_effect;
          quat /= net_effect;
          pos /= net_effect;
          shear /= net_effect;

          // There should be no need to normalize the quaternion, assuming all
          // of the input quaternions were already normalized.

          _value = LMatrix4::scale_shear_mat(scale, shear) * quat;
          _value.set_row(3, pos);
        }
      }
      break;
    }
  }
}

/**
 * Freezes this particular joint so that it will always hold the specified
 * transform.  Returns true if this is a joint that can be so frozen, false
 * otherwise.  This is called internally by PartBundle::freeze_joint().
 */
bool MovingPartMatrix::
apply_freeze_matrix(const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale) {
  _forced_channel = new AnimChannelMatrixFixed(get_name(), pos, hpr, scale);
  return true;
}

/**
 * Specifies a node to influence this particular joint so that it will always
 * hold the node's transform.  Returns true if this is a joint that can be so
 * controlled, false otherwise.  This is called internally by
 * PartBundle::control_joint().
 */
bool MovingPartMatrix::
apply_control(PandaNode *node) {
  AnimChannelMatrixDynamic *chan = new AnimChannelMatrixDynamic(get_name());
  chan->set_value_node(node);
  _forced_channel = chan;
  return true;
}

/**
 * Factory method to generate a MovingPartMatrix object
 */
TypedWritable* MovingPartMatrix::
make_MovingPartMatrix(const FactoryParams &params) {
  MovingPartMatrix *me = new MovingPartMatrix;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a MovingPartMatrix object
 */
void MovingPartMatrix::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_MovingPartMatrix);
}
