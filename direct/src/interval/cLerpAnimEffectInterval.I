// Filename: cLerpAnimEffectInterval.I
// Created by:  drose (27Aug02)
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
