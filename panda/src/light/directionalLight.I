// Filename: directionalLight.I
// Created by:  mike (04Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::get_specular
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Colorf DirectionalLight::get_specular( void ) const
{
  return _specular;
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::set_specular
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DirectionalLight::set_specular( const Colorf& color )
{
  _specular = color;
}
