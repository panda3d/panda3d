// Filename: guiManager.h
// Created by:  cary (25Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __GUIMANAGER_H__
#define __GUIMANAGER_H__

#include <pandabase.h>
#include <graphicsWindow.h>
#include <mouse.h>
#include <mouseWatcher.h>
#include <node.h>
#include <set>

#include "guiRegion.h"
#include "guiLabel.h"
#include "config_gui.h"

class EXPCL_PANDA GuiManager {
private:
  typedef map<GraphicsWindow*, GuiManager*> GuiMap;
  static GuiMap* _map;
  typedef set<GuiRegion*> RegionSet;
  RegionSet _regions;
  typedef set<GuiLabel*> LabelSet;
  LabelSet _labels;

  Node* _root;
  MouseWatcher* _watcher;

  INLINE GuiManager(MouseWatcher*, Node*);

PUBLISHED:
  static GuiManager* get_ptr(GraphicsWindow*, MouseAndKeyboard*, Node *root2d);

  void add_region(GuiRegion*);
  void add_label(GuiLabel*);

  void remove_region(GuiRegion*);
  void remove_label(GuiLabel*);
};

#include "guiManager.I"

#endif /* __GUIMANAGER_H__ */
