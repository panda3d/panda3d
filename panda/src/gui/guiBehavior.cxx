// Filename: guiBehavior.cxx
// Created by:  cary (07Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "guiBehavior.h"

TypeHandle GuiBehavior::_type_handle;

GuiBehavior::BehaviorFunctor::BehaviorFunctor(void) {
}

GuiBehavior::BehaviorFunctor::~BehaviorFunctor(void) {
}

void GuiBehavior::BehaviorFunctor::doit(GuiBehavior*) {
}

GuiBehavior::GuiBehavior(const string& name) : GuiItem(name),
					       _eh((EventHandler*)0L) {
}

GuiBehavior::~GuiBehavior(void) {
}

void GuiBehavior::manage(GuiManager* mgr, EventHandler& eh) {
  _eh = &eh;
  GuiItem::manage(mgr, eh);
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
