// Filename: colladaBindMaterial.cxx
// Created by:  rdb (26May11)
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

#include "colladaBindMaterial.h"
#include "colladaPrimitive.h"

// Collada DOM includes.  No other includes beyond this point.
#include "pre_collada_include.h"
#include <dom/domBind_material.h>
#include <dom/domEffect.h>
#include <dom/domInstance_effect.h>
#include <dom/domInstance_material.h>
#include <dom/domMaterial.h>

#if PANDA_COLLADA_VERSION >= 15
#include <dom/domFx_profile.h>
#else
#include <dom/domFx_profile_abstract.h>
#define domFx_profile domFx_profile_abstract
#define domFx_profile_Array domFx_profile_abstract_Array
#define getFx_profile_array getFx_profile_abstract_array
#endif

////////////////////////////////////////////////////////////////////
//     Function: ColladaBindMaterial::get_material
//  Description: Returns the material to be applied to the given
//               primitive, or NULL if there was none bound.
////////////////////////////////////////////////////////////////////
CPT(RenderState) ColladaBindMaterial::
get_material(const ColladaPrimitive *prim) const {
  if (prim == NULL || _states.count(prim->get_material()) == 0) {
    return NULL;
  }
  return _states.find(prim->get_material())->second;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaBindMaterial::get_material
//  Description: Returns the bound material with the indicated
//               symbol, or NULL if it was not found.
////////////////////////////////////////////////////////////////////
CPT(RenderState) ColladaBindMaterial::
get_material(const string &symbol) const {
  if (_states.count(symbol) == 0) {
    return NULL;
  }
  return _states.find(symbol)->second;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaBindMaterial::load_bind_material
//  Description: Loads a bind_material object.
////////////////////////////////////////////////////////////////////
void ColladaBindMaterial::
load_bind_material(domBind_material &bind_mat) {
  domInstance_material_Array &mat_instances
    = bind_mat.getTechnique_common()->getInstance_material_array();

  for (size_t i = 0; i < mat_instances.getCount(); ++i) {
    load_instance_material(*mat_instances[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaBindMaterial::load_instance_material
//  Description: Loads an instance_material object.
////////////////////////////////////////////////////////////////////
void ColladaBindMaterial::
load_instance_material(domInstance_material &inst) {
  domMaterialRef mat = daeSafeCast<domMaterial> (inst.getTarget().getElement());
  nassertv(mat != NULL);

  domInstance_effectRef einst = mat->getInstance_effect();
  nassertv(einst != NULL);

  domInstance_effect::domSetparam_Array &setparams = einst->getSetparam_array();

  domEffectRef effect = daeSafeCast<domEffect>
    (mat->getInstance_effect()->getUrl().getElement());

  //TODO: read params
  
  const domFx_profile_Array &profiles = effect->getFx_profile_array();
  for (size_t i = 0; i < profiles.getCount(); ++i) {
    //profiles[i]->
  }
}
