/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colladaBindMaterial.h
 * @author rdb
 * @date 2011-05-25
 */

#ifndef COLLADABINDMATERIAL_H
#define COLLADABINDMATERIAL_H

#include "config_collada.h"
#include "renderState.h"
#include "pmap.h"

class ColladaPrimitive;

class domBind_material;
class domInstance_material;

/**
 * Class that deals with binding materials to COLLADA geometry.
 */
class ColladaBindMaterial {
public:
  CPT(RenderState) get_material(const ColladaPrimitive *prim) const;
  CPT(RenderState) get_material(const string &symbol) const;

  void load_bind_material(domBind_material &bind_mat);
  void load_instance_material(domInstance_material &inst);

private:
  pmap<string, CPT(RenderState)> _states;
};

#endif
