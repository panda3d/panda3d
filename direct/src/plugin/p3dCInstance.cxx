/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dCInstance.cxx
 * @author drose
 * @date 2009-06-08
 */

#include "p3dCInstance.h"


/**
 * Constructs a new Instance from an XML description.
 */
P3DCInstance::
P3DCInstance(TiXmlElement *xinstance) :
  _func(NULL)
{
  xinstance->Attribute("instance_id", &_instance_id);
}

/**

 */
P3DCInstance::
~P3DCInstance() {
}
