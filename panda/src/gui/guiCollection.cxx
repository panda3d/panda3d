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
    (*i)->recompute_frame();

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
  for (Items.iterator i=_items.begin(); i!=_items.end(); ++i)
    if ((*i) == item)
      found = true;
  if (!found)
    _items.push_back(item);
  this->recompute();
}

void GuiCollection::remove_item(GuiItem* item) {
  Items::iterator i = _items.find(item);
  if (i == _items.end())
    return;
  item->unmanage();
  _items.erase(i);
  this->recompute();
}
