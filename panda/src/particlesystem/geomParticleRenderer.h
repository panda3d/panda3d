// Filename: geomParticleRenderer.h
// Created by:  charles (05Jul00)
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

#ifndef GEOMPARTICLERENDERER_H
#define GEOMPARTICLERENDERER_H

#include "baseParticleRenderer.h"
#include "baseParticle.h"

#include "pandaNode.h"
#include "pointerTo.h"
#include "pointerToArray.h"

#include "pvector.h"

class EXPCL_PANDAPHYSICS GeomParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  GeomParticleRenderer(ParticleRendererAlphaMode am = PR_ALPHA_NONE,
                       PandaNode *geom_node = (PandaNode *) NULL);
  GeomParticleRenderer(const GeomParticleRenderer& copy);
  virtual ~GeomParticleRenderer();

  INLINE void set_geom_node(PandaNode *node);
  INLINE PandaNode *get_geom_node();

  virtual BaseParticleRenderer *make_copy();

private:

  PT(PandaNode) _geom_node;

  pvector< PT(PandaNode) > _node_vector;

  int _pool_size;

  // geomparticlerenderer takes advantage of the birth/death functions

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);

  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);

  virtual void resize_pool(int new_size);
  void kill_nodes();
};

#include "geomParticleRenderer.I"

#endif // GEOMPARTICLERENDERER_H
