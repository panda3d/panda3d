// Filename: guiBehavior.cxx
// Created by:  cary (07Feb01)
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

#include "guiBehavior.h"

TypeHandle GuiBehavior::_type_handle;
TypeHandle GuiBehavior::BehaviorFunctor::_type_handle;

GuiBehavior::BehaviorFunctor::BehaviorFunctor(void) : TypedReferenceCount() {
}

GuiBehavior::BehaviorFunctor::~BehaviorFunctor(void) {
}

void GuiBehavior::BehaviorFunctor::doit(GuiBehavior*) {
}

GuiBehavior::GuiBehavior(const string& name) : GuiItem(name),
                                               _eh((EventHandler*)0L),
                                               _behavior_running(false) {
}

GuiBehavior::~GuiBehavior(void) {
}

void GuiBehavior::manage(GuiManager* mgr, EventHandler& eh) {
  _eh = &eh;
  GuiItem::manage(mgr, eh);
}

void GuiBehavior::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  _eh = &eh;
  GuiItem::manage(mgr, eh, n);
}

void GuiBehavior::unmanage(void) {
  if (_behavior_running)
    this->stop_behavior();
  _eh = (EventHandler*)0L;
  GuiItem::unmanage();
}

void GuiBehavior::start_behavior(void) {
  _behavior_running = true;
}

void GuiBehavior::stop_behavior(void) {
  _behavior_running = false;
}

void GuiBehavior::reset_behavior(void) {
}

void GuiBehavior::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Behavior Data:" << endl;
  os << "    behavior is " << (_behavior_running?"":"not ") << "running"
     << endl;
  os << "    eh - 0x" << (void*)_eh << endl;
}
