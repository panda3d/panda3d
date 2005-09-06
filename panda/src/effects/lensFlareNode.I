// Filename: lensFlareNode.I
// Created by:  jason (01Aug00)
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
//     Function: LensFlareNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LensFlareNode::
LensFlareNode() :
  _global_scale(1), _texel_scale(0.1), _blind_fall_off(45), _flare_fall_off(5)
{
  _blind = (Texture *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_texel_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensFlareNode::
set_texel_scale(float texel_to_world)
{
  _texel_scale = texel_to_world;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_global_scale
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensFlareNode::
set_global_scale(float scale)
{
  _global_scale = scale;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_blind_falloff
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensFlareNode::
set_blind_falloff(float fall_off)
{
  _blind_fall_off = cos(deg_2_rad(fall_off)) - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_flare_falloff
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensFlareNode::
set_flare_falloff(float fall_off)
{
  _flare_fall_off = cos(deg_2_rad(fall_off)) - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: LensFlareNode::set_light
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LensFlareNode::
set_light_source(PT_Node light)
{
 _light_node = light;
}
