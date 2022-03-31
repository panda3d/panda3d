/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomParticleRenderer.h
 * @author charles
 * @date 2000-07-05
 */

#ifndef GEOMPARTICLERENDERER_H
#define GEOMPARTICLERENDERER_H

#include "baseParticleRenderer.h"
#include "baseParticle.h"
#include "colorInterpolationManager.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "pointerToArray.h"
#include "pvector.h"
#include "pStatCollector.h"

class EXPCL_PANDA_PARTICLESYSTEM GeomParticleRenderer : public BaseParticleRenderer {
PUBLISHED:
  explicit GeomParticleRenderer(ParticleRendererAlphaMode am = PR_ALPHA_NONE,
                                PandaNode *geom_node = nullptr);
  GeomParticleRenderer(const GeomParticleRenderer& copy);
  virtual ~GeomParticleRenderer();

  INLINE void set_geom_node(PandaNode *node);
  INLINE PandaNode *get_geom_node();
  INLINE ColorInterpolationManager* get_color_interpolation_manager() const;

  INLINE void set_x_scale_flag(bool animate_x_ratio);
  INLINE void set_y_scale_flag(bool animate_y_ratio);
  INLINE void set_z_scale_flag(bool animate_z_ratio);
  INLINE void set_initial_x_scale(PN_stdfloat initial_x_scale);
  INLINE void set_final_x_scale(PN_stdfloat final_x_scale);
  INLINE void set_initial_y_scale(PN_stdfloat initial_y_scale);
  INLINE void set_final_y_scale(PN_stdfloat final_y_scale);
  INLINE void set_initial_z_scale(PN_stdfloat initial_z_scale);
  INLINE void set_final_z_scale(PN_stdfloat final_z_scale);

  INLINE bool get_x_scale_flag() const;
  INLINE bool get_y_scale_flag() const;
  INLINE bool get_z_scale_flag() const;
  INLINE PN_stdfloat get_initial_x_scale() const;
  INLINE PN_stdfloat get_final_x_scale() const;
  INLINE PN_stdfloat get_initial_y_scale() const;
  INLINE PN_stdfloat get_final_y_scale() const;
  INLINE PN_stdfloat get_initial_z_scale() const;
  INLINE PN_stdfloat get_final_z_scale() const;

public:
  virtual BaseParticleRenderer *make_copy();

  virtual void output(std::ostream &out) const;
  virtual void write_linear_forces(std::ostream &out, int indent=0) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  PT(PandaNode) _geom_node;
  PT(ColorInterpolationManager) _color_interpolation_manager;

  pvector< PT(PandaNode) > _node_vector;

  int _pool_size;
  PN_stdfloat _initial_x_scale;
  PN_stdfloat _final_x_scale;
  PN_stdfloat _initial_y_scale;
  PN_stdfloat _final_y_scale;
  PN_stdfloat _initial_z_scale;
  PN_stdfloat _final_z_scale;

  bool _animate_x_ratio;
  bool _animate_y_ratio;
  bool _animate_z_ratio;

  // geomparticlerenderer takes advantage of the birthdeath functions

  virtual void birth_particle(int index);
  virtual void kill_particle(int index);

  virtual void init_geoms();
  virtual void render(pvector< PT(PhysicsObject) >& po_vector,
                      int ttl_particles);

  virtual void resize_pool(int new_size);
  void kill_nodes();

  static PStatCollector _render_collector;
};

#include "geomParticleRenderer.I"

#endif // GEOMPARTICLERENDERER_H
