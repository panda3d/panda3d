// Filename: p3dCInstance.cxx
// Created by:  drose (08Jun09)
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

#include "p3dCInstance.h"


////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::Constructor
//       Access: Public
//  Description: Constructs a new Instance from an XML description.
////////////////////////////////////////////////////////////////////
P3DCInstance::
P3DCInstance(TiXmlElement *xinstance) :
  _func(NULL)
{
  xinstance->Attribute("instance_id", &_instance_id);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DCInstance::
~P3DCInstance() {
}
