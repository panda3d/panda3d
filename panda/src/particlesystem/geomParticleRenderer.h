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

#include <node.h>
#include <pointerTo.h>
#include <pointerToArray.h>
#include <renderRelation.h>

#include "pvector.h"

class EXPCL_PANDAPHYSICS GeomParticleRenderer : public BaseParticleRenderer {
private:

  PT(Node) _geom_node;
  PT(Node) _dead_particle_parent_node;

  pvector< PT(RenderRelation) > _arc_vector;

  int _pool_size;

  INLINE void kill_arcs(void);

  // geomparticlerenderer takes advantage of the birth/death functions

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);

  virtual void init_geoms(void);
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);

  virtual void resize_pool(int new_size);

PUBLISHED:
  GeomParticleRenderer(ParticleRendererAlphaMode am = PR_ALPHA_NONE,
                       Node *geom_node = (Node *) NULL);
  GeomParticleRenderer(const GeomParticleRenderer& copy);
  virtual ~GeomParticleRenderer(void);

  INLINE void set_geom_node(Node *node);
  INLINE Node *get_geom_node(void);

  virtual BaseParticleRenderer *make_copy(void);
};

#include "geomParticleRenderer.I"

#endif // GEOMPARTICLERENDERER_H
