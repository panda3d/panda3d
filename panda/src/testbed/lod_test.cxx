// Filename: lod_test.cxx
// Created by:  
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

#include "pandabase.h"
#include <namedNode.h>

#include <eventHandler.h>
#include <chancfg.h>
#include <textNode.h>
#include <eggLoader.h>
#include <LOD.h>
#include <LODNode.h>
#include <pt_NamedNode.h>

extern PT_NamedNode render;
extern PT_NamedNode egg_root;
extern EventHandler event_handler;

extern int framework_main(int argc, char *argv[]);
extern void (*define_keys)(EventHandler&);

void lod_keys(EventHandler& eh) {

  PT(LODNode) lodnode = new LODNode("lodnode");
  new RenderRelation(egg_root, lodnode);

  PT_NamedNode lodnode0 = loader.load_sync("smiley.egg");
  new RenderRelation(lodnode, lodnode0);
  lodnode->add_switch(10, 0);

  PT_NamedNode lodnode1 = loader.load_sync("frowney.egg");
  new RenderRelation(lodnode, lodnode1);
  lodnode->add_switch(1000, 10);
}

int main(int argc, char *argv[]) {
  define_keys = &lod_keys;
  return framework_main(argc, argv);
}
