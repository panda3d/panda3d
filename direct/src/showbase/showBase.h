// Filename: showBase.h
// Created by:  shochet (02Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SHOWBASE_H
#define SHOWBASE_H

#include <directbase.h>

#include <eventHandler.h>
#include <graphicsPipe.h>
#include <graphicsWindow.h>
#include <animControl.h>
#include <nodeRelation.h>
#include <pointerTo.h>
#include <nodePath.h>
#include <dconfig.h>

ConfigureDecl(config_showbase, EXPCL_DIRECT, EXPTP_DIRECT);
typedef Config::Config<ConfigureGetConfig_config_showbase> ConfigShowbase;

class CollisionTraverser;
class Camera;

BEGIN_PUBLISH

EXPCL_DIRECT PT(GraphicsPipe) make_graphics_pipe();
EXPCL_DIRECT PT(GraphicsWindow) 
  make_graphics_window(GraphicsPipe *pipe, 
		       NamedNode *render,
		       NamedNode *camera,
		       NamedNode *data_root,
		       NodeAttributes &initial_state
		       );	

EXPCL_DIRECT NodePath setup_panda_2d(GraphicsWindow *win);
EXPCL_DIRECT void add_render_layer(GraphicsWindow *win, Node *render_top,
				   Camera *camera);

EXPCL_DIRECT void set_collision_traverser(CollisionTraverser *traverser);
EXPCL_DIRECT CollisionTraverser *get_collision_traverser();
EXPCL_DIRECT void clear_collision_traverser();

EXPCL_DIRECT void toggle_wireframe(NodeAttributes &initial_state);
EXPCL_DIRECT void toggle_texture(NodeAttributes &initial_state);
EXPCL_DIRECT void toggle_backface(NodeAttributes &initial_state);

EXPCL_DIRECT ConfigShowbase &get_config_showbase();

END_PUBLISH

#endif
