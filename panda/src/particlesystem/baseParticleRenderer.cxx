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
BaseParticleRenderer(ParticleRendererAlphaMode alpha_mode) :
  _alpha_arc((RenderRelation *) NULL),
  _alpha_mode(PR_NOT_INITIALIZED_YET) {
  _render_node = new GeomNode("BaseParticleRenderer render node");

  _user_alpha = 1.0f;

  update_alpha_mode(alpha_mode);
}

////////////////////////////////////////////////////////////////////
//    Function : BaseParticleRenderer
//      Access : Public
// Description : Copy Constructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
BaseParticleRenderer(const BaseParticleRenderer& copy) :
  _alpha_arc((RenderRelation *) NULL),
  _alpha_mode(PR_ALPHA_NONE) {
  _render_node = new GeomNode("BaseParticleRenderer render node");

  _user_alpha = copy._user_alpha;

  update_alpha_mode(copy._alpha_mode);
}

////////////////////////////////////////////////////////////////////
//    Function : ~BaseParticleRenderer
//      Access : Public
// Description : Destructor
////////////////////////////////////////////////////////////////////
BaseParticleRenderer::
~BaseParticleRenderer(void) {
  if(_alpha_mode != PR_ALPHA_NONE)
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
update_alpha_mode(ParticleRendererAlphaMode am) {
  if (_alpha_mode == am)
    return;

  if ((am == PR_ALPHA_NONE) && (_alpha_mode != PR_ALPHA_NONE))
    disable_alpha();
  else if ((am != PR_ALPHA_NONE) && (_alpha_mode == PR_ALPHA_NONE))
    enable_alpha();

  _alpha_mode = am;
}
