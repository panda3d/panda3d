// Filename: guiListBox.cxx
// Created by:  cary (18Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "guiListBox.h"

TypeHandle GuiListBox::_type_handle;

void GuiListBox::recompute_frame(void) {
  GuiBehavior::recompute_frame();
  LVector3f p = _pos;
  float lft = 100000.;
  float rgt = -100000.;
  float tp = -100000.;
  float btm = 100000.;
  LVector4f frm;
  for (ItemVector::iterator i=_visible.begin(); i!=_visible.end();
       p+=((*i)->get_height() * LVector3f::up()), ++i) {
    (*i)->set_pos(p);
    frm = (*i)->get_frame();
    if (frm[0] < lft)
      lft = frm[0];
    if (frm[1] > rgt)
      rgt = frm[1];
    if (frm[2] < btm)
      btm = frm[2];
    if (frm[3] > tp)
      tp = frm[3];
  }
  _left = lft;
  _right = rgt;
  _top = tp;
  _bottom = btm;
}

void GuiListBox::visible_patching(void) {
  // check for 2 cases on both top and bottom of the visible list.  First,
  // if there is an item off that edge, but no arrow active; then put an arrow
  // up (needing to move a second item off the edge).  Second, if there is an
  // arrow up, but there is only a single item off the edge; replace the arrow
  // with the item.

  // top first
  if (_arrow_top) {
    if (_top_stack.size() == 1) {
      // replace the up arrow with the single item in the top stack
      _arrow_top = false;
      _visible[0]->unmanage();
      _visible[0] = *(_top_stack.begin());
      _visible[0]->manage(_mgr, *_eh);
      _top_stack.pop_back();
    }
  } else {
    if (_top_stack.size() > 0) {
      // move the top of the visible list to the top stack and put the up
      // arrow in it's place
      _arrow_top = true;
      _visible[0]->unmanage();
      _top_stack.push_back(_visible[0]);
      _visible[0] = _up_arrow;
      _up_arrow->manage(_mgr, *_eh);
    }
  }

  // now bottom
  if (_arrow_bottom) {
    if (_bottom_stack.size() == 1) {
      // replace the down arrow with the single item in the bottom stack
      _arrow_bottom = false;
      int last = _n_visible-1;
      _visible[last]->unmanage();
      _visible[last] = *(_bottom_stack.begin());
      if (_mgr != (GuiManager*)0L)
	_visible[last]->manage(_mgr, *_eh);
      _bottom_stack.pop_back();
    }
  } else {
    if (_bottom_stack.size() > 0) {
      // move the bottom of the visible list to the bottom stack and put the
      // down arrow in it's place
      _arrow_bottom = true;
      int last = _n_visible-1;
      _visible[last]->unmanage();
      _bottom_stack.push_back(_visible[last]);
      _visible[last] = _down_arrow;
      if (_mgr != (GuiManager*)0L)
	_down_arrow->manage(_mgr, *_eh);
    }
  }
}

GuiListBox::GuiListBox(const string& name, int N, GuiItem* up, GuiItem* down)
  : GuiBehavior(name), _arrow_top(false), _arrow_bottom(false), _up_arrow(up),
    _down_arrow(down), _n_visible(N) {
  if (N < 4) {
    gui_cat->warning() << "ListBoxes should have at least 4 visible slots"
		       << endl;
    _n_visible = 4;
  }
}

GuiListBox::~GuiListBox(void) {
  this->unmanage();
}

void GuiListBox::scroll_up(void) {
  if (_bottom_stack.size() == 0)
    return;   // nothing to scroll
  // compute what the first and list item in the visible list are
  int first = 0;
  int last = _n_visible-1;
  if (_arrow_top)
    ++first;
  if (_arrow_bottom)
    --last;
  // first push one off the top onto the top stack
  _top_stack.push_back(_visible[first]);
  _visible[first]->unmanage();
  // then move everything up one
  for (int i=first; i!=last; ++i)
    _visible[i] = _visible[i+1];
  // then add one from the bottom stack to the bottom
  _visible[last] = *(_bottom_stack.rbegin());
  if (_mgr != (GuiManager*)0L)
    _visible[last]->manage(_mgr, *_eh);
  // and pop it off the bottom stack
  _bottom_stack.pop_back();
  // now patch-up any dangling items
  visible_patching();
  // finally recompute all the possitions
  this->recompute_frame();
}

void GuiListBox::scroll_down(void) {
  if (_top_stack.size() == 0)
    return;  // nothing to scroll
  // compute what the first and last item in the visible list are
  int first = 0;
  int last = _n_visible - 1;
  if (_arrow_top)
    ++first;
  if (_arrow_bottom)
    --last;
  // first push one off the bottom onto the bottom stack
  _bottom_stack.push_back(_visible[last]);
  _visible[last]->unmanage();
  // then move everything down one
  for (int i=last; i!=first; --i)
    _visible[i] = _visible[i-1];
  // then add one from the top stack to the top
  _visible[first] = *(_top_stack.rbegin());
  if (_mgr != (GuiManager*)0L)
    _visible[first]->manage(_mgr, *_eh);
  // and pop it off the top stack
  _top_stack.pop_back();
  // now patch-up any dangling item
  visible_patching();
  // finally recompute all the possitions
  this->recompute_frame();
}

void GuiListBox::add_item(GuiItem* item) {
  if (_bottom_stack.size() > 0)
    _bottom_stack.push_front(item);
  else {
    if (_visible.size() < _n_visible) {
      _visible.push_back(item);
      if (_mgr != (GuiManager*)0L)
	item->manage(_mgr, *_eh);
    } else
      _bottom_stack.push_back(item);
  }
  visible_patching();
}

int GuiListBox::freeze(void) {
  int result = 0;

  for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i) {
    int count = (*i)->freeze();
    result = max(result, count);
  }
  return result;
}

int GuiListBox::thaw(void) {
  int result = 0;

  for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i) {
    int count = (*i)->thaw();
    result = max(result, count);
  }
  return result;
}

void GuiListBox::manage(GuiManager* mgr, EventHandler& eh) {
  if (_mgr == (GuiManager*)0L) {
    for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i)
      (*i)->manage(mgr, eh);
    _eh = &eh;
    GuiBehavior::manage(mgr, eh);
  } else
    gui_cat->warning() << "tried to manage listbox (0x" << (void*)this
		       << ") that is already managed" << endl;
}

void GuiListBox::unmanage(void) {
  for (ItemVector::iterator i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->unmanage();
  GuiBehavior::unmanage();
}

void GuiListBox::set_scale(float f) {
  ItemVector::iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_scale(f);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_scale(f);
  for (ItemDeque::iterator j=_bottom_stack.begin(); j!=_bottom_stack.end();
       ++j)
    (*j)->set_scale(f);
  GuiBehavior::set_scale(f);
}

void GuiListBox::set_pos(const LVector3f& p) {
  ItemVector::iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    (*i)->set_pos(p);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    (*i)->set_pos(p);
  for (ItemDeque::iterator j=_bottom_stack.begin(); j!=_bottom_stack.end();
       ++j)
    (*j)->set_pos(p);
  GuiBehavior::set_pos(p);
}

void GuiListBox::start_behavior(void) {
  GuiBehavior::start_behavior();
}

void GuiListBox::stop_behavior(void) {
  GuiBehavior::stop_behavior();
}

void GuiListBox::reset_behavior(void) {
  GuiBehavior::reset_behavior();
}

void GuiListBox::output(ostream& os) const {
  GuiBehavior::output(os);
  os << "  Listbox data:" << endl;
  os << "    There is ";
  if (!_arrow_top)
    os << "no ";
  os << "top arrow (0x" << (void*)_up_arrow << ")" << endl;
  os << "    There is ";
  if (!_arrow_bottom)
    os << "no ";
  os << "bottom arrow (0x" << (void*)_down_arrow << ")" << endl;
  os << "    Top stack (" << _top_stack.size() << "):" << endl;
  ItemVector::const_iterator i;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    os << "      0x" << (void*)(*i) << endl;
  os << "    Visible (" << _visible.size() << "):" << endl;
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    os << "      0x" << (void*)(*i) << endl;
  os << "    Bottom stack (" << _bottom_stack.size() << "):" << endl;
  ItemDeque::const_iterator j;
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    os << "      0x" << (void*)(*j) << endl;
  for (i=_top_stack.begin(); i!=_top_stack.end(); ++i)
    os << *(*i);
  for (i=_visible.begin(); i!=_visible.end(); ++i)
    os << *(*i);
  for (j=_bottom_stack.begin(); j!=_bottom_stack.end(); ++j)
    os << *(*j);
}
