// Filename: partBundle.I
// Created by:  drose (22Feb99)
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
//     Function: PartBundle::get_blend_type
//       Access: Published
//  Description: Returns the way the character responds to multiple
//               animations being bound simultaneously.
////////////////////////////////////////////////////////////////////
INLINE PartBundle::BlendType PartBundle::
get_blend_type() const {
  CDReader cdata(_cycler);
  return cdata->_blend_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_node
//       Access: Published
//  Description: Returns the PartBundleNode associated with this
//               PartBundle.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode *PartBundle::
get_node() const {
  return _node;
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::set_control_effect
//       Access: Published
//  Description: Sets the amount by which the character is affected by
//               the indicated AnimControl (and its associated
//               animation).  Normally, this will only be zero or one.
//               Zero indicates the animation does not affect the
//               character, and one means it does.
//
//               If the blend_type is not BT_single (see
//               set_blend_type()), it is possible to have multiple
//               AnimControls in effect simultaneously.  In this case,
//               the effect is a weight that indicates the relative
//               importance of each AnimControl to the final
//               animation.
////////////////////////////////////////////////////////////////////
void PartBundle::
set_control_effect(AnimControl *control, float effect) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDWriter cdata(_cycler);
  do_set_control_effect(control, effect, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_control_effect
//       Access: Published
//  Description: Returns the amount by which the character is affected
//               by the indicated AnimControl and its associated
//               animation.  See set_control_effect().
////////////////////////////////////////////////////////////////////
INLINE float PartBundle::
get_control_effect(AnimControl *control) const {
  CDReader cdata(_cycler);
  return do_get_control_effect(control, cdata);
}

