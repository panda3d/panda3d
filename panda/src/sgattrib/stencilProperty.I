// Filename: stencilProperty.I
// Created by:  drose (23Mar00)
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
//     Function: StencilProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::
StencilProperty(StencilProperty::Mode mode, StencilProperty::Action pass_action, 
                  StencilProperty::Action fail_action, StencilProperty::Action zfail_action,
                  unsigned long refval, unsigned long funcmask, unsigned long writemask) :
  _mode(mode),
  _pass_action(pass_action),
  _fail_action(fail_action),
  _zfail_action(zfail_action),
  _refval(refval),
  _funcmask(funcmask),
  _writemask(writemask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StencilProperty::compare_to
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int StencilProperty::
compare_to(const StencilProperty &other) const {

  #define CMPVAL(VAL)                     \
   if (VAL != other.VAL) {return (int)VAL - (int)other.VAL;}
    
  CMPVAL(_mode); 
  CMPVAL(_pass_action);
  CMPVAL(_fail_action);
  CMPVAL(_zfail_action);
  CMPVAL(_refval);
  CMPVAL(_funcmask);
  CMPVAL(_writemask);

  return 0;
}
