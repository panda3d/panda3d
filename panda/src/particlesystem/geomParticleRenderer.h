// Filename: geomParticleRenderer.h
// Created by:  charles (05Jul00)
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

#include <vector>

class EXPCL_PANDAPHYSICS GeomParticleRenderer : public BaseParticleRenderer {
private:

  PT(Node) _geom_node;
  PT(Node) _dead_particle_parent_node;

  vector< PT(RenderRelation) > _arc_vector;

  int _pool_size;

  INLINE void kill_arcs(void);

  // geomparticlerenderer takes advantage of the birth/death functions

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);

  virtual void init_geoms(void);
  virtual void render(vector< PT(PhysicsObject) >& po_vector,
		      int ttl_particles);

  virtual void resize_pool(int new_size);

public:

  GeomParticleRenderer(ParticleRendererAlphaDecay ad = PR_NO_ALPHA,
		       Node *geom_node = (Node *) NULL);
  GeomParticleRenderer(const GeomParticleRenderer& copy);
  virtual ~GeomParticleRenderer(void);

  INLINE void set_geom_node(Node *node);
  INLINE Node *get_geom_node(void);

  virtual BaseParticleRenderer *make_copy(void);
};

#include "geomParticleRenderer.I"

#endif // GEOMPARTICLERENDERER_H
