// Filename: guiCollection.cxx
// Created by:  cary (07Mar01)
// 
////////////////////////////////////////////////////////////////////

#include "guiCollection.h"

TypeHandle GuiCollection::_type_handle;

void GuiCollection::recompute_frame(void) {
  GuiItem::recompute_frame();

  freeze();

  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->recompute();

  this->adjust_region();
  thaw();
}

void GuiCollection::set_priority(GuiLabel* l, const GuiItem::Priority p) {
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_priority(l, p);
}

GuiCollection::GuiCollection(const string& name) : GuiItem(name) {
}

GuiCollection::~GuiCollection(void) {
  this->unmanage();
}

int GuiCollection::freeze(void) {
  int result = 0;

  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i) {
    int count = (*i)->freeze();
    result = max(result, count);
  }
  return result;
}

int GuiCollection::thaw(void) {
  int result = 0;

  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i) {
    int count = (*i)->thaw();
    result = max(result, count);
  }
  return result;
}

void GuiCollection::add_item(GuiItem* item) {
  bool found = false;
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    if ((*i) == item)
      found = true;
  if (!found)
    _items.push_back(item);
  // this->recompute();
}

void GuiCollection::remove_item(GuiItem* item) {
  Items::iterator i;
  for (i=_items.begin(); i!=_items.end(); ++i)
    if ((*i) == item)
      break;
  if (i == _items.end())
    return;
  item->unmanage();
  _items.erase(i);
  // this->recompute();
}

void GuiCollection::manage(GuiManager* mgr, EventHandler& eh) {
  if (_mgr == (GuiManager*)0L) {
    for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
      (*i)->manage(mgr, eh);
    GuiItem::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage collection (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiCollection::manage(GuiManager* mgr, EventHandler& eh, Node* n) {
  if (_mgr == (GuiManager*)0L) {
    for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
      (*i)->manage(mgr, eh, n);
    GuiItem::manage(mgr, eh, n);
  } else
    gui_cat->warning() << "tried to manage collection (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiCollection::unmanage(void) {
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->unmanage();
  GuiItem::unmanage();
}

void GuiCollection::set_scale(float f) {
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_scale(f * (*i)->get_scale());
  GuiItem::set_scale(f);
  this->recompute_frame();
}

void GuiCollection::set_scale(float x, float y, float z) {
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_scale(x * (*i)->get_scale_x(),
		    y * (*i)->get_scale_y(),
		    z * (*i)->get_scale_z());
  GuiItem::set_scale(x, y, z);
  this->recompute_frame();
}

void GuiCollection::set_pos(const LVector3f& p) {
  LVector3f delta = p - this->get_pos();
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_pos((*i)->get_pos() + delta);
  GuiItem::set_pos(p);
  this->recompute_frame();
}

void GuiCollection::set_priority(GuiItem* it, const GuiItem::Priority p) {
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    (*i)->set_priority(it, p);
}

int GuiCollection::set_draw_order(int v) {
  bool first = true;
  int o;
  for (Items::iterator i=_items.begin(); i!=_items.end(); ++i)
    if (first) {
      first = false;
      o = (*i)->set_draw_order(v);
    } else
      o = (*i)->set_draw_order(o);
  return GuiItem::set_draw_order(o);
}

void GuiCollection::output(ostream& os) const {
  GuiItem::output(os);
  os << "  Collection data:" << endl;
  Items::const_iterator i;
  for (i=_items.begin(); i!=_items.end(); ++i)
    os << "    item - 0x" << (void*)(*i) << endl;
  for (i=_items.begin(); i!=_items.end(); ++i)
    os << *(*i);
}
