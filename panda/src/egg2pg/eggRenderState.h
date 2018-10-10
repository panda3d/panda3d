/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggRenderState.h
 * @author drose
 * @date 2005-03-12
 */

#ifndef EGGRENDERSTATE_H
#define EGGRENDERSTATE_H

#include "pandabase.h"

#include "eggUserData.h"
#include "eggLoader.h"
#include "renderState.h"
#include "renderAttrib.h"
#include "internalName.h"
#include "geomPrimitive.h"
#include "luse.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"

class EggPrimitive;
class EggTexture;
class EggMaterial;

/**
 * This class is used within this package only to record the render state that
 * should be assigned to each primitive.  It is assigned to EggPrimitive
 * objects via the EggBinner.
 */
class EXPCL_PANDA_EGG2PG EggRenderState : public EggUserData {
public:
  INLINE EggRenderState(EggLoader &loader);
  INLINE void add_attrib(const RenderAttrib *attrib);

  void fill_state(EggPrimitive *egg_prim);

  int compare_to(const EggRenderState &other) const;

private:
  CPT(RenderAttrib) get_material_attrib(const EggMaterial *egg_mat,
                                        bool bface);
  static TexGenAttrib::Mode get_tex_gen(const EggTexture *egg_tex);

  CPT(RenderAttrib)
  apply_tex_mat(CPT(RenderAttrib) tex_mat_attrib,
                TextureStage *stage, const EggTexture *egg_tex);

public:
  CPT(RenderState) _state;
  bool _hidden;
  bool _flat_shaded;
  Geom::PrimitiveType _primitive_type;

  typedef EggLoader::BakeInUVs BakeInUVs;
  typedef EggLoader::TextureDef TextureDef;
  typedef EggLoader::Materials Materials;

  BakeInUVs _bake_in_uvs;

private:
  EggLoader &_loader;

  typedef pvector<const TextureDef *> TexMatTextures;
  typedef pmap<LMatrix4d, TexMatTextures> TexMatTransforms;
  typedef pmap<CPT(InternalName), TexMatTransforms> TexMats;
};

#include "eggRenderState.I"

#endif
