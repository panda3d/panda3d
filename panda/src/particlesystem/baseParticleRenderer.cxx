/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseParticleRenderer.cxx
 * @author charles
 * @date 2000-06-20
 */

#include "pandabase.h"

#include "baseParticleRenderer.h"
#include "transparencyAttrib.h"
#include "colorAttrib.h"
#include "compassEffect.h"

/**
 * Default Constructor
 */
BaseParticleRenderer::
BaseParticleRenderer(ParticleRendererAlphaMode alpha_mode) :
  _alpha_mode(PR_NOT_INITIALIZED_YET) {
  _render_node = new GeomNode("BaseParticleRenderer render node");
  _render_node_path = NodePath(_render_node);

  _user_alpha = 1.0f;
  _ignore_scale = false;

  update_alpha_mode(alpha_mode);
}

/**
 * Copy Constructor
 */
BaseParticleRenderer::
BaseParticleRenderer(const BaseParticleRenderer& copy) :
  _alpha_mode(PR_ALPHA_NONE) {
  _render_node = new GeomNode("BaseParticleRenderer render node");
  _render_node_path = NodePath(_render_node);

  _user_alpha = copy._user_alpha;
  set_ignore_scale(copy._ignore_scale);

  update_alpha_mode(copy._alpha_mode);
}

/**
 * Destructor
 */
BaseParticleRenderer::
~BaseParticleRenderer() {
}

/**
 * Sets the "ignore scale" flag.  When this is true, particles will be drawn
 * as if they had no scale, regardless of whatever scale might be inherited
 * from above the render node in the scene graph.
 *
 * This flag is mainly useful to support legacy code that was written for a
 * very early version of Panda, whose sprite particle renderer had a bug that
 * incorrectly ignored the inherited scale.
 */
void BaseParticleRenderer::
set_ignore_scale(bool ignore_scale) {
  _ignore_scale = ignore_scale;

  if (_ignore_scale) {
    _render_node->set_effect(CompassEffect::make(NodePath(), CompassEffect::P_scale));
  } else {
    _render_node->clear_effect(CompassEffect::get_class_type());
  }
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticleRenderer::
output(std::ostream &out) const {
  #ifndef NDEBUG //[
  out<<"BaseParticleRenderer";
  #endif //] NDEBUG
}

/**
 * Write a string representation of this instance to <out>.
 */
void BaseParticleRenderer::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"BaseParticleRenderer:\n";
  out.width(indent+2); out<<""; out<<"_render_node "<<_render_node_path<<"\n";
  out.width(indent+2); out<<""; out<<"_user_alpha "<<_user_alpha<<"\n";
  // ReferenceCount::write(out, indent+2);
  #endif //] NDEBUG
}

/**
 * handles the base class part of alpha updating.
 */
void BaseParticleRenderer::
update_alpha_mode(ParticleRendererAlphaMode am) {
  if (_alpha_mode == am)
    return;

  if ((am == PR_ALPHA_NONE) && (_alpha_mode != PR_ALPHA_NONE))
    disable_alpha();
  else if ((am != PR_ALPHA_NONE) && (_alpha_mode == PR_ALPHA_NONE))
    enable_alpha();

  _alpha_mode = am;
}

/**
 * Builds an intermediate node and transition that enables alpha channeling.
 */
void BaseParticleRenderer::
enable_alpha() {
  _render_state = RenderState::make(TransparencyAttrib::make(TransparencyAttrib::M_alpha),
                                    ColorAttrib::make_vertex());
}

/**
 * kills the intermediate alpha node/arc
 */
void BaseParticleRenderer::
disable_alpha() {
  _render_state = RenderState::make(TransparencyAttrib::make(TransparencyAttrib::M_none),
                                    ColorAttrib::make_vertex());
}
