// Filename: guiManager.cxx
// Created by:  cary (25Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "guiManager.h"

#include <dataRelation.h>
#include <renderRelation.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <materialTransition.h>
#include <cullFaceTransition.h>
#include <lightTransition.h>
#include <frustum.h>
#include <orthoProjection.h>

GuiManager::GuiMap* GuiManager::_map = (GuiManager::GuiMap*)0L;

GuiManager* GuiManager::get_ptr(GraphicsWindow* w, MouseAndKeyboard* mak) {
  GuiManager* ret;
  if (_map == (GuiMap*)0L)
    _map = new GuiMap;
  GuiMap::const_iterator gi;
  gi = _map->find(w);
  if (gi != _map->end())
    ret = (*gi).second;
  else {
    // going to allocate a new GuiManager for this window
    // first see if there is a mouseWatcher already under the MouseAndKeyboard
    bool has_watcher = false;
    TypeHandle dgt = DataRelation::get_class_type();
    MouseWatcher* watcher;
    for (int i=0; i<mak->get_num_children(dgt); ++i)
      if (mak->get_child(dgt, i)->get_child()->get_type() ==
	  MouseWatcher::get_class_type()) {
	has_watcher = true;
	watcher = DCAST(MouseWatcher, mak->get_child(dgt, i)->get_child());
      }
    if (!has_watcher) {
      // there isn't already a mousewatcher in the data graph, so we'll make
      // one and re-parent everything to it.
      watcher = new MouseWatcher("GUI watcher");
      DataRelation* tmp = new DataRelation(mak, watcher);
      for (int j=0; j<mak->get_num_children(dgt); ++j) {
	NodeRelation* rel = mak->get_child(dgt, j);
	if (rel != tmp)
	  // it's not the node we just created, so reparent it to ours
	  rel->change_parent(watcher);
      }
    }
    // next, create a 2d layer for the GUI stuff to live in.
    Node* root2d_top = new NamedNode("GUI_top");
    Node* root2d = new NamedNode("GUI");
    NodeRelation* root2d_arc = new RenderRelation(root2d_top, root2d);
    root2d_arc->set_transition(new DepthTestTransition(DepthTestProperty::M_none), 1);
    root2d_arc->set_transition(new DepthWriteTransition(DepthWriteTransition::off()), 1);
    root2d_arc->set_transition(new LightTransition(LightTransition::all_off()), 1);
    root2d_arc->set_transition(new MaterialTransition(MaterialTransition::off()), 1);
    root2d_arc->set_transition(new CullFaceTransition(CullFaceProperty::M_cull_none), 1);
    PT(Camera) cam = new Camera("GUI_cam");
    new RenderRelation(root2d, cam);
    cam->set_scene(root2d_top);
    Frustumf frust2d;
    frust2d.make_ortho_2D();
    cam->set_projection(OrthoProjection(frust2d));
    GraphicsChannel *chan = w->get_channel(0);  // root/full-window channel
    nassertv(chan != (GraphicsChannel*)0L);
    GraphicsLayer *layer = chan->make_layer();
    nassertv(layer != (GraphicsLayer*)0L);
    DisplayRegion *dr = layer->make_display_region();
    nassertv(dr != (DisplayRegion*)0L);
    dr->set_camera(cam);
    // now make the manager for this window
    ret = new GuiManager(watcher, root2d);
    (*_map)[w] = ret;
  }
  return ret;
}
