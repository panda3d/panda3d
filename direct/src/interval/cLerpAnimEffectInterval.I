// Filename: cLerpAnimEffectInterval.I
// Created by:  drose (27Aug02)
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


////////////////////////////////////////////////////////////////////
//     Function: CLerpAnimEffectInterval::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLerpAnimEffectInterval::
CLerpAnimEffectInterval(const string &name, double duration, 
                        CLerpInterval::BlendType blend_type) :
  CLerpInterval(name, duration, blend_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpAnimEffectInterval::add_control
//       Access: Published
//  Description: Adds another AnimControl to the list of AnimControls
//               affected by the lerp.  This control will be lerped
//               from begin_effect to end_effect over the period of
//               the lerp.
//
//               The AnimControl name parameter is only used when
//               formatting the interval for output.
////////////////////////////////////////////////////////////////////
INLINE void CLerpAnimEffectInterval::
add_control(AnimControl *control, const string &name,
            float begin_effect, float end_effect) {
  _controls.push_back(ControlDef(control, name, begin_effect, end_effect));
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpAnimEffectInterval::ControlDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLerpAnimEffectInterval::ControlDef::
ControlDef(AnimControl *control, const string &name,
           float begin_effect, float end_effect) :
  _control(control),
  _name(name),
  _begin_effect(begin_effect),
  _end_effect(end_effect)
{
}
