// Filename: baseParticleRenderer.cxx
// Created by:  charles (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include <nodeRelation.h>

#include "baseParticleRenderer.h"

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleRenderer
//      Access : Public
// Description : Default Constructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
BaseParticleRenderer(ParticleRendererAlphaDecay alpha_decay) :
  _alpha_arc((RenderRelation *) NULL), 
  _alpha_decay(PR_ALPHA_INVALID) {
  _render_node = new GeomNode("BaseParticleRenderer render node");

  update_alpha_state(alpha_decay);
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleRenderer
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
BaseParticleRenderer(const BaseParticleRenderer& copy) :
  _alpha_arc((RenderRelation *) NULL),
  _alpha_decay(PR_ALPHA_INVALID) {
  _render_node = new GeomNode("BaseParticleRenderer render node");

  update_alpha_state(copy._alpha_decay);
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticleRenderer
//      Access : Public
// Description : Destructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
~BaseParticleRenderer(void) {
  if (_alpha_decay == PR_ALPHA_OUT || _alpha_decay == PR_ALPHA_IN)
    remove_arc(_alpha_arc);
}

////////////////////////////////////////////////////////////////////
//    Function : enable_alpha
//      Access : Private
// Description : Builds an intermediate node and transition that
//               enables alpha channeling.
////////////////////////////////////////////////////////////////////
void BaseParticleRenderer::
enable_alpha(void) {
  _render_node->clear();

  _alpha_node = new GeomNode("BaseParticleRenderer alpha node");

  _alpha_arc = new RenderRelation(_render_node, _alpha_node);
  _alpha_arc->set_transition(new TransparencyTransition(TransparencyProperty::M_alpha));

  _interface_node = _alpha_node;
}

////////////////////////////////////////////////////////////////////
//    Function : update_alpha_state
//      Access : Private
// Description : handles the base class part of alpha updating.
////////////////////////////////////////////////////////////////////
void BaseParticleRenderer::
update_alpha_state(ParticleRendererAlphaDecay ad) {
  if (_alpha_decay == ad)
    return;

  if (ad == PR_NO_ALPHA && _alpha_decay != PR_NO_ALPHA)
    disable_alpha();
  else if ((_alpha_decay == PR_ALPHA_INVALID || _alpha_decay == PR_NO_ALPHA) && 
	   ad != PR_NO_ALPHA)
    enable_alpha();

  _alpha_decay = ad;
}

