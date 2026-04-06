/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLerpAnimEffectInterval.I
 * @author drose
 * @date 2002-08-27
 */

/**
 *
 */
INLINE CLerpAnimEffectInterval::
CLerpAnimEffectInterval(const std::string &name, double duration,
                        CLerpInterval::BlendType blend_type) :
  CLerpInterval(name, duration, blend_type)
{
}

/**
 * Adds another AnimControl to the list of AnimControls affected by the lerp.
 * This control will be lerped from begin_effect to end_effect over the period
 * of the lerp.
 *
 * The AnimControl name parameter is only used when formatting the interval
 * for output.
 */
INLINE void CLerpAnimEffectInterval::
add_control(AnimControl *control, const std::string &name,
            float begin_effect, float end_effect) {
  _controls.push_back(ControlDef(control, name, begin_effect, end_effect));
}

/**
 *
 */
INLINE CLerpAnimEffectInterval::ControlDef::
ControlDef(AnimControl *control, const std::string &name,
           float begin_effect, float end_effect) :
  _control(control),
  _name(name),
  _begin_effect(begin_effect),
  _end_effect(end_effect)
{
}
